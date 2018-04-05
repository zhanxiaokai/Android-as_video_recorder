#include "./video_packet_consumer.h"

#define LOG_TAG "VideoPacketConsumerThread"

VideoPacketConsumerThread::VideoPacketConsumerThread() {
    obj = NULL;
}

VideoPacketConsumerThread::~VideoPacketConsumerThread() {
}

static int on_publish_timeout_callback(void *context) {
    VideoPacketConsumerThread *videoPacketConsumer = (VideoPacketConsumerThread *) context;
    return videoPacketConsumer->publishTimeOut();
}

static int on_publish_statistics_callback(long startTimeMills, int connectTimeMills,
                                          int publishDurationInSec,
                                          float discardFrameRatio, float publishAVGBitRate,
                                          float expectedBitRate, char *adaptiveBitrateChart,
                                          void *context) {
    VideoPacketConsumerThread *videoPacketConsumer = (VideoPacketConsumerThread *) context;
    return videoPacketConsumer->statisticsCallback(startTimeMills, connectTimeMills,
                                                   publishDurationInSec,
                                                   discardFrameRatio, publishAVGBitRate,
                                                   expectedBitRate, adaptiveBitrateChart);
}

static int on_adaptive_bitrate_callback(PushVideoQuality videoQuality, void *context) {
    VideoPacketConsumerThread *videoPacketConsumer = (VideoPacketConsumerThread *) context;
    videoPacketConsumer->adaptiveVideoQuality(videoQuality);
    return 1;
}

static int hot_adaptive_bitrate_callback(int maxBitrate, int avgBitrate, int fps, void *context) {
    LOGI("hot_adaptive_bitrate_callback adapt new video quality:(current time:%d, maxBitrate:%d, avgBitrate:%d, fps:%d)",
         platform_4_live::getCurrentTimeSeconds()%100,
         maxBitrate, avgBitrate, fps);
    VideoPacketConsumerThread *videoPacketConsumer = (VideoPacketConsumerThread *) context;
    videoPacketConsumer->hotAdaptiveVideoQuality(maxBitrate, avgBitrate, fps);
    return 1;
}

static int on_publish_bitrate_statistics_callback(int sendBitrate, int compressedBitrate,
                                          void *context) {
    VideoPacketConsumerThread *videoPacketConsumer = (VideoPacketConsumerThread *) context;
    videoPacketConsumer->statisticsBitrateCallback(sendBitrate, compressedBitrate);
    return 0;
}

int VideoPacketConsumerThread::init(char *videoPath,
                                    int videoWidth, int videoheight, int videoFrameRate,
                                    int videoBitRate,
                                    int audioSampleRate, int audioChannels, int audioBitRate,
                                    char *audio_codec_name,
                                    int qualityStrategy,
									const std::map<std::string, int>& configMap,
                                    JavaVM *g_jvm, jobject obj) {
    this->g_jvm = g_jvm;
    this->obj = obj;
    int ret = VideoConsumerThread::init(videoPath,
                                        videoWidth, videoheight, videoFrameRate, videoBitRate,
                                        audioSampleRate, audioChannels, audioBitRate,
                                        audio_codec_name, qualityStrategy,configMap);
    packetPool = LiveCommonPacketPool::GetInstance();
    packetPool->setStatistics(statistics);
    if (ret < 0) {
        LOGI("VideoConsumerThread::init failed...");
        return ret;
    }

    VideoConsumerThread::registerPublishTimeoutCallback(on_publish_timeout_callback, this);
    VideoConsumerThread::registerPublishStatisticsCallback(on_publish_statistics_callback, this);
    VideoConsumerThread::registerAdaptiveBitrateCallback(on_adaptive_bitrate_callback, this);
    VideoConsumerThread::registerHotAdaptiveBitrateCallback(hot_adaptive_bitrate_callback, this);
    VideoConsumerThread::registerStatisticsBitrateCallback(on_publish_bitrate_statistics_callback, this);

    LOGI("leave VideoPacketConsumerThread::init...");
    return 1;
}

void VideoPacketConsumerThread::stop() {
    VideoConsumerThread::stop();
}

void VideoPacketConsumerThread::adaptiveVideoQuality(int videoQuality) {
    if (obj) {
        JNIEnv *env;
        int status = 0;
        bool needAttach = false;
        status = g_jvm->GetEnv((void **) (&env), JNI_VERSION_1_4);
        // don't know why, if detach directly, will crash
        if (status < 0) {
            if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
                LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
                return;
            }
            needAttach = true;
        }
        jclass jcls = env->GetObjectClass(obj);
        jmethodID adaptiveVideoQualityCallbackFunc = env->GetMethodID(jcls,
                                                                      "adaptiveVideoQualityFromNative",
                                                                      "(I)V");
        env->CallVoidMethod(obj, adaptiveVideoQualityCallbackFunc, videoQuality);
        if (needAttach) {
            if (g_jvm->DetachCurrentThread() != JNI_OK) {
                LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
            }
        }
    }
}

void VideoPacketConsumerThread::hotAdaptiveVideoQuality(int maxBitrate, int avgBitrate, int fps) {
    if (obj) {
        JNIEnv *env;
        int status = 0;
        bool needAttach = false;
        status = g_jvm->GetEnv((void **) (&env), JNI_VERSION_1_4);
        // don't know why, if detach directly, will crash
        if (status < 0) {
            if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
                LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
                return;
            }
            needAttach = true;
        }
        jclass jcls = env->GetObjectClass(obj);
        jmethodID adaptiveVideoQualityCallbackFunc = env->GetMethodID(jcls,
                                                                      "hotAdaptiveVideoQualityFromNative",
                                                                      "(III)V");
        env->CallVoidMethod(obj, adaptiveVideoQualityCallbackFunc, maxBitrate, avgBitrate, fps);
        if (needAttach) {
            if (g_jvm->DetachCurrentThread() != JNI_OK) {
                LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
            }
        }
    }
}


int VideoPacketConsumerThread::statisticsCallback(long startTimeMills, int connectTimeMills,
                                                  int publishDurationInSec,
                                                  float discardFrameRatio, float publishAVGBitRate,
                                                  float expectedBitRate,
                                                  char *adaptiveBitrateChart) {
    LOGI("startTimeMills %ld", startTimeMills);
    LOGI("connectTimeMills %d", connectTimeMills);
    LOGI("publishDurationInSec %d", publishDurationInSec);
    LOGI("discardFrameRatio %f", discardFrameRatio);
    LOGI("publishAVGBitRate %f", publishAVGBitRate);
    LOGI("expectedBitRate %f", expectedBitRate);
    LOGI("adaptiveBitrateChart %s", adaptiveBitrateChart);
    if (obj) {
        JNIEnv *env;
        int status = 0;
        bool needAttach = false;
        status = g_jvm->GetEnv((void **) (&env), JNI_VERSION_1_4);
        // don't know why, if detach directly, will crash
        if (status < 0) {
            if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
                LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
                return -1;
            }
            needAttach = true;
        }
        jclass jcls = env->GetObjectClass(obj);
        jmethodID statisticsCallbackFunc = env->GetMethodID(jcls, "statisticsCallbackFromNative",
                                                            "(IIFFFLjava/lang/String;)V");
        jclass strClass = env->FindClass("java/lang/String");
        jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
        int len = strlen(adaptiveBitrateChart);
        jstring bitrateChart = 0;
        if (len > 0) {
            jbyteArray bytes = env->NewByteArray(len);
            env->SetByteArrayRegion(bytes, 0, strlen(adaptiveBitrateChart),
                                    (jbyte *) adaptiveBitrateChart);
            jstring encoding = env->NewStringUTF("utf-8");
            bitrateChart = (jstring) env->NewObject(strClass, ctorID, bytes, encoding);
            env->DeleteLocalRef(bytes);
            env->DeleteLocalRef(encoding);
        }
        env->CallVoidMethod(obj, statisticsCallbackFunc, connectTimeMills, publishDurationInSec,
                            discardFrameRatio, publishAVGBitRate, expectedBitRate, bitrateChart);
        if (needAttach) {
            if (g_jvm->DetachCurrentThread() != JNI_OK) {
                LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
            }
        }
    }
    return 1;
}

int VideoPacketConsumerThread::publishTimeOut() {
    if (obj) {
        JNIEnv *env;
        int status = 0;
        bool needAttach = false;
        status = g_jvm->GetEnv((void **) (&env), JNI_VERSION_1_4);
        // don't know why, if detach directly, will crash
        if (status < 0) {
            if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
                LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
                return -1;
            }
            needAttach = true;
        }
        jclass jcls = env->GetObjectClass(obj);
        jmethodID pushTimeOutFunc = env->GetMethodID(jcls, "publishTimeOutFromNative", "()V");
        env->CallVoidMethod(obj, pushTimeOutFunc);
        if (needAttach) {
            if (g_jvm->DetachCurrentThread() != JNI_OK) {
                LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
            }
        }
    }
}

void VideoPacketConsumerThread::statisticsBitrateCallback(int sendBitrate, int compressedBitrate) {
    if (obj) {
        JNIEnv *env;
        int status = 0;
        bool needAttach = false;
        status = g_jvm->GetEnv((void **) (&env), JNI_VERSION_1_4);
        if (status < 0) {
            if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
                LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
                return;
            }
            needAttach = true;
        }
        jclass jcls = env->GetObjectClass(obj);
        jmethodID statisticsBitrateCallbackFunc = env->GetMethodID(jcls, "statisticsBitrateFromNative", "(II)V");
        env->CallVoidMethod(obj, statisticsBitrateCallbackFunc, sendBitrate, compressedBitrate);
        if (needAttach) {
            if (g_jvm->DetachCurrentThread() != JNI_OK) {
                LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
            }
        }
    }
}
