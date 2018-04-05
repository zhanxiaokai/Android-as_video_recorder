//
//  publisher_rate_feed_back.hpp
//  liveDemo
//
//  Created by apple on 16/5/17.
//  Copyright © 2016年 changba. All rights reserved.
//

#ifndef LIVE_PUBLISHER_RATE_FEED_BACK_H
#define LIVE_PUBLISHER_RATE_FEED_BACK_H

#define WINDOW_SIZE_IN_SECS 3

#include "../platform_dependent/platform_4_live_common.h"
#include "live_video_quality_agent.h"
#include <vector>
#include <string>
#include <sstream>
#include <map>

using namespace std;

typedef struct WindowBitRate {
    int startTimeInSecs;
    int endTimeInSecs;
    int bitRate;
    
    WindowBitRate() {
        this->startTimeInSecs = 0;
        this->endTimeInSecs = 0;
        this->bitRate = 0;
    }
    
    void update(int startTimeInSecs, int endTimeInSecs, int bitRate) {
        this->startTimeInSecs = startTimeInSecs;
        this->endTimeInSecs = endTimeInSecs;
        this->bitRate = bitRate;
    }
    
    void clone(WindowBitRate *windowBitrate) {
        this->startTimeInSecs = windowBitrate->startTimeInSecs;
        this->endTimeInSecs = windowBitrate->endTimeInSecs;
        this->bitRate = windowBitrate->bitRate;
    }
    
    ~WindowBitRate() {
    }
} WindowBitRate;

class PublisherRateFeedback {
private:
    PublisherRateFeedback(); //注意:构造方法私有
    static PublisherRateFeedback *instance; //惟一实例
    /** 最关键的3各参数, 会影响我们对于网络速度对实际码率的容忍度或者灵敏程度 **/
    int windowSizeInSecs;
    int encoderReconfigInterval;
    /** 检测当前发送窗口内的平均码率 **/
    long startTimeMills;
    long windowStartTimeMills;
    long sendSizeInWindow;
    
    VideoQualityAgent *qualityAgent;
    
    WindowBitRate *sendLastWindowAVGBitrate;
    WindowBitRate *sendCurWindowAVGBitrate;
    
    bool detectNotifyEncoderReConfigure();
    
    long latestNotifyTimeMills;
    
    /** 统计压缩数据的变量 **/
    int audioBitrate;
    long compressedTotalLength;
    double compressedStartTimeMills;
    int compressedFrameCount;
    
    float videoPacketQueueState;
    
    WindowBitRate *compressLastWindowAVGBitrate;
    WindowBitRate *compressCurWindowAVGBitrate;
    
    /** 当由于网络速度变化, 我们做的自适应码率的调节 **/
    typedef int (*on_adaptive_bitrate_callback)(PushVideoQuality videoQuality, void *context);
    
    int avgCompressedBitRate;
    int avgPublishedBitRate;
public:
    static PublisherRateFeedback *GetInstance(); //工厂方法(用来获得实例)
    virtual ~PublisherRateFeedback();
    
    void init(int audioBitrate, int qualityStrategy, const std::map<std::string, int>& configMap);
    
    void publishPktSuccess(int pktSize);
    
    void prePublishPkt();
    
    int getCompressedBitRate();
    
    int getPublishedBitRate();
    
    char *getAdaptiveBitrateChart() {
        return qualityAgent->getAdaptiveBitrateChart();
    };
    
    float getRealTimePublishBitRate() {
        return sendCurWindowAVGBitrate->bitRate;
    };
    
    float getRealTimeCompressedBitRate() {
        return compressCurWindowAVGBitrate->bitRate;
    };
    
    void statisticCompressData(double timeMills, int dataSize, float packetQueueSize);
    
    virtual void registerAdaptiveBitrateCallback(
                                                 int (*on_adaptive_bitrate_callback)(PushVideoQuality videoQuality, void *context),
                                                 void *context);
    
    virtual void registerHotAdaptiveBitrateCallback(
                                                    int (*hot_adaptive_bitrate_callback)(int maxBitrate, int avgBitrate, int fps,
                                                                                         void *context),
                                                    void *context);
    
    virtual void registerStatisticsBitrateCallback(
                                                   int (*statistics_bitrate_callback)(int sendBitrate, int compressedBitrate,
                                                                                      void *context),
                                                   void *context);
    
};

#endif /* LIVE_PUBLISHER_RATE_FEED_BACK_H */
