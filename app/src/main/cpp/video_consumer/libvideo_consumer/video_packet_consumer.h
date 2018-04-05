#ifndef SONGSTUDIO_BASE_VIDEO_PACKET_CONSUMER_H
#define SONGSTUDIO_BASE_VIDEO_PACKET_CONSUMER_H

#include "livecore/consumer/video_consumer_thread.h"
#include "live_common_packet_pool.h"
#include <map>

class VideoPacketConsumerThread : public VideoConsumerThread {
public:
    VideoPacketConsumerThread();

    virtual ~VideoPacketConsumerThread();

    virtual int init(char *videoPath,
                     int videoWidth, int videoheight, int videoFrameRate, int videoBitRate,
                     int audioSampleRate, int audioChannels, int audioBitRate,
                     char *audio_codec_name,
                     int qualityStrategy,
                     const std::map<std::string, int>& configMap,
                     JavaVM *g_jvm, jobject obj);

    virtual void stop();

    int getVideoFrame(uint8_t *picture_buf, int frame_size);

    int publishTimeOut();

    void statisticsBitrateCallback(int sendBitrate, int compressedBitrate);

    int statisticsCallback(long startTimeMills, int connectTimeMills, int publishDurationInSec,
                           float discardFrameRatio, float publishAVGBitRate, float expectedBitRate,
                           char *adaptiveBitrateChart);

    void adaptiveVideoQuality(int videoQuality);

    void hotAdaptiveVideoQuality(int maxBitrate, int avgBitrate, int fps);

protected:
    JavaVM *g_jvm;
    jobject obj;
};

#endif //SONGSTUDIO_BASE_VIDEO_PACKET_CONSUMER_H
