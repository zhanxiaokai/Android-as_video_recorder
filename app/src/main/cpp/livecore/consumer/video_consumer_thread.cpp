#include "./video_consumer_thread.h"
#define LOG_TAG "VideoConsumerThread"

VideoConsumerThread::VideoConsumerThread() {
	isStopping = false;
	videoPublisher = NULL;
	isConnecting = false;
	onStatisticsCallback = NULL;
	statistics = NULL;

	pthread_mutex_init(&connectingLock, NULL);
	pthread_cond_init(&interruptCondition, NULL);
	pthread_mutex_init(&interruptLock, NULL);
}
VideoConsumerThread::~VideoConsumerThread() {
	pthread_mutex_destroy(&connectingLock);
	pthread_cond_destroy(&interruptCondition);
	pthread_mutex_destroy(&interruptLock);
}

void VideoConsumerThread::registerPublishStatisticsCallback(
		int (*on_statistics_callback)(long startTimeMills, int connectTimeMills, int publishDurationInSec, float discardFrameRatio, float publishAVGBitRate, float expectedBitRate,
				char* adaptiveBitrateChart, void *context), void* context) {
	this->onStatisticsCallback = on_statistics_callback;
	this->statisticsContext = context;
}

void VideoConsumerThread::registerPublishTimeoutCallback(int (*on_publish_timeout_callback)(void *context), void* context) {
	if (NULL != videoPublisher) {
		videoPublisher->registerPublishTimeoutCallback(on_publish_timeout_callback, context);
	}
}

void VideoConsumerThread::registerAdaptiveBitrateCallback(int (*on_adaptive_bitrate_callback)(PushVideoQuality videoQuality, void *context), void* context) {
	if (NULL != videoPublisher) {
		videoPublisher->registerAdaptiveBitrateCallback(on_adaptive_bitrate_callback, context);
	}
}

void VideoConsumerThread::registerHotAdaptiveBitrateCallback(int (*hot_adaptive_bitrate_callback)(int maxBitrate, int avgBitrate, int fps, void *context), void *context) {
    if (NULL != videoPublisher) {
        videoPublisher->registerHotAdaptiveBitrateCallback(hot_adaptive_bitrate_callback, context);
    }
}

void VideoConsumerThread::registerStatisticsBitrateCallback(int (*statistics_bitrate_callback)(int sendBitrate, int compressedBitrate, void *context), void *context) {
    if (NULL != videoPublisher) {
        videoPublisher->registerStatisticsBitrateCallback(statistics_bitrate_callback, context);
    }
}

static int fill_aac_packet_callback(LiveAudioPacket** packet, void *context) {
	VideoConsumerThread* videoPacketConsumer = (VideoConsumerThread*) context;
	return videoPacketConsumer->getAudioPacket(packet);
}
int VideoConsumerThread::getAudioPacket(LiveAudioPacket ** audioPacket) {
	if (aacPacketPool->getAudioPacket(audioPacket, true) < 0) {
		LOGI("aacPacketPool->getAudioPacket return negetive value...");
		return -1;
	}
	return 1;
}

static int fill_h264_packet_callback(LiveVideoPacket ** packet, void *context) {
	VideoConsumerThread* videoPacketConsumer = (VideoConsumerThread*) context;
	return videoPacketConsumer->getH264Packet(packet);
}
int VideoConsumerThread::getH264Packet(LiveVideoPacket ** packet) {
	if (packetPool->getRecordingVideoPacket(packet, true) < 0) {
		LOGI("packetPool->getRecordingVideoPacket return negetive value...");
		return -1;
	}
	return 1;
}

float VideoConsumerThread::getDiscardFrameRatio() {
    if(NULL != statistics){
        return statistics->getDiscardFrameRatio();
    }
    return 0.0f;
}

float VideoConsumerThread::getRealTimePublishBitRate(){
    if(NULL != statistics){
        return statistics->getRealTimePublishBitRate();
    }
    return 0.0f;
}

float VideoConsumerThread::getRealTimeCompressedBitRate(){
    if(NULL != statistics){
        return statistics->getRealTimeCompressedBitRate();
    }
    return 0.0f;
}

void VideoConsumerThread::init() {
	isStopping = false;
	packetPool = LivePacketPool::GetInstance();
	aacPacketPool = LiveAudioPacketPool::GetInstance();
	videoPublisher = NULL;
	onStatisticsCallback = NULL;
	statistics = NULL;
}

int VideoConsumerThread::init(char* videoOutputURI, int videoWidth, int videoheight, int videoFrameRate, int videoBitRate, int audioSampleRate, int audioChannels, int audioBitRate,
		char* audio_codec_name, int qualityStrategy, const std::map<std::string, int>& configMap) {
	init();
	if (NULL == videoPublisher) {
		pthread_mutex_lock(&connectingLock);
		this->isConnecting = true;
		pthread_mutex_unlock(&connectingLock);
		buildPublisherInstance();
		statistics = new PublisherStatistics(videoBitRate + audioBitRate);
		int ret = videoPublisher->init(videoOutputURI, videoWidth, videoheight, videoFrameRate, videoBitRate, audioSampleRate, audioChannels, audioBitRate, audio_codec_name,
				statistics, qualityStrategy, configMap);

		pthread_mutex_lock(&connectingLock);
		this->isConnecting = false;
		pthread_mutex_unlock(&connectingLock);
		LOGI("videoPublisher->init return code %d...", ret);

		if (ret < 0 || videoPublisher->isInterrupted()) {
			LOGI("videoPublisher->init failed...");
			pthread_mutex_lock(&interruptLock);

			this->releasePublisher();
			delete statistics;
			statistics = NULL;

			pthread_cond_signal(&interruptCondition);
			pthread_mutex_unlock(&interruptLock);
			return ret;
		}
		if (!isStopping) {
			statistics->connectSuccess();
			packetPool->setStatistics(statistics);
			videoPublisher->registerFillAACPacketCallback(fill_aac_packet_callback, this);
			videoPublisher->registerFillVideoPacketCallback(fill_h264_packet_callback, this);
		} else {
			LOGI("Client Cancel ...");
			return CLIENT_CANCEL_CONNECT_ERR_CODE;
		}
	}
	return 0;
}

void VideoConsumerThread::releasePublisher() {
	if (NULL != videoPublisher) {
        videoPublisher->stop();
        delete videoPublisher;
        videoPublisher = NULL;
	}
}

void VideoConsumerThread::buildPublisherInstance() {
	videoPublisher = new RecordingH264Publisher();
}

void VideoConsumerThread::stop() {
	LOGI("enter VideoConsumerThread::stop...");
	pthread_mutex_lock(&connectingLock);
	if (isConnecting) {
		LOGI("before interruptPublisherPipe()");
		videoPublisher->interruptPublisherPipe();
		LOGI("after interruptPublisherPipe()");
		pthread_mutex_unlock(&connectingLock);

		//等待init处理完
		pthread_mutex_lock(&interruptLock);
		pthread_cond_wait(&interruptCondition, &interruptLock);
		pthread_mutex_unlock(&interruptLock);

		LOGI("VideoConsumerThread::stop isConnecting return...");
		return;
	}
	pthread_mutex_unlock(&connectingLock);

	int ret = -1;

	isStopping = true;
	packetPool->abortRecordingVideoPacketQueue();
	aacPacketPool->abortAudioPacketQueue();
    long startEndingThreadTimeMills = platform_4_live::getCurrentTimeMills();
	LOGI("before wait publisher encoder ...");

	if (videoPublisher != NULL){
		videoPublisher->interruptPublisherPipe();
	}
	if ((ret = wait()) != 0) {
		LOGI("Couldn't cancel VideoConsumerThread: %d", ret);
		//		return;
	}
	LOGI("after wait publisher encoder ... %d", (int)(platform_4_live::getCurrentTimeMills() - startEndingThreadTimeMills));

	this->releasePublisher();

	packetPool->destoryRecordingVideoPacketQueue();
	aacPacketPool->destoryAudioPacketQueue();
	if (NULL != statistics && NULL != onStatisticsCallback) {
		statistics->stopPublish();
		onStatisticsCallback(statistics->getStartTimeMills(), statistics->getConnectTimeMills(), statistics->getPublishDurationInSec(), statistics->getDiscardFrameRatio(),
				statistics->getPublishAVGBitRate(), statistics->getExpectedBitRate(), statistics->getAdaptiveBitrateChart(), statisticsContext);
		delete statistics;
		statistics = NULL;
	}
	LOGI("leave VideoConsumerThread::stop...");
}

void VideoConsumerThread::handleRun(void* ptr) {
	while (mRunning) {
		int ret = videoPublisher->encode();
		if (ret < 0) {
			LOGI("videoPublisher->encode result is invalid, so we will stop encode...");
			break;
		}
	}
}

