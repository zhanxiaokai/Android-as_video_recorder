#ifndef LIVE_BASE_VIDEO_CONSUMER_THREAD_H
#define LIVE_BASE_VIDEO_CONSUMER_THREAD_H
#include "../platform_dependent/platform_4_live_common.h"
#include "./../common/live_thread.h"
#include "./../common/publisher_statistics.h"
#include "./../common/live_packet_pool.h"
#include "./../common/live_audio_packet_pool.h"
#include "../publisher/recording_h264_publisher.h"
#include "../publisher/recording_publisher.h"

#define LIVE_AUDIO_PCM_OUTPUT_CHANNEL 2

#define CLIENT_CANCEL_CONNECT_ERR_CODE               -100199

class VideoConsumerThread: public LiveThread {
public:
    VideoConsumerThread();
    virtual ~VideoConsumerThread();
    int init(char* videoOutputURI,
             int videoWidth, int videoheight, int videoFrameRate, int videoBitRate,
             int audioSampleRate, int audioChannels, int audioBitRate, char* audio_codec_name,
             int qualityStrategy,
			 const std::map<std::string, int>& configMap);
    
    virtual void stop();
    
    void registerPublishTimeoutCallback(int (*on_publish_timeout_callback)(void *context), void* context);
    
    void registerAdaptiveBitrateCallback(int (*on_adaptive_bitrate_callback)(PushVideoQuality videoQuality, void *context), void* context);
    void registerHotAdaptiveBitrateCallback(int (*hot_adaptive_bitrate_callback)(int maxBitrate, int avgBitrate, int fps, void *context), void *context);
    void registerStatisticsBitrateCallback(int (*statistics_bitrate_callback)(int sendBitrate, int compressedBitrate, void *context), void *context);

    int getH264Packet(LiveVideoPacket ** packet);
    int getAudioPacket(LiveAudioPacket ** audioPacket);
    
    /** 获得实时的丢帧率 **/
    float getDiscardFrameRatio();
    /** 获得实时的视频流的发布比特率 **/
    float getRealTimePublishBitRate();
    /** 获得实时的视频流的压缩比特率 **/
    float getRealTimeCompressedBitRate();
    
    void registerPublishStatisticsCallback(int (*on_statistics_callback)(long startTimeMills, int connectTimeMills, int publishDurationInSec,
                                                                         float discardFrameRatio, float publishAVGBitRate, float expectedBitRate,
                                                                         char* adaptiveBitrateChart, 
                                                                         void *context), void* context);
    /** 当由于网络问题导致timeout的问题回调 **/
    typedef int (*on_statistics_callback)(long startTimeMills, int connectTimeMills, int publishDurationInSec,
    float discardFrameRatio, float publishAVGBitRate, float expectedBitRate, char* adaptiveBitrateChart, void *context);
    
protected:
    LivePacketPool* 							packetPool;
    LiveAudioPacketPool* 					aacPacketPool;
    /** 当我们停止发布的时候，会把统计数据回调给客户端进行上报服务器 **/
    PublisherStatistics* 					statistics;
    on_statistics_callback 					onStatisticsCallback;
    void* 									statisticsContext;
    
    bool                						isStopping;
    
    bool										isConnecting;
	pthread_mutex_t 							connectingLock;
	pthread_mutex_t 							interruptLock;
	pthread_cond_t 							interruptCondition;

    RecordingPublisher* 						videoPublisher;
    
    virtual void init();
    virtual void buildPublisherInstance();
    void releasePublisher();
    
    void handleRun(void* ptr);
};

#endif //LIVE_BASE_VIDEO_CONSUMER_THREAD_H
