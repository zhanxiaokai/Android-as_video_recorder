//
// Created by ckb on 2017/3/4.
//

#ifndef LIVE_STUDIO_ANDROID_LIVE_VIDEO_QUALITY_AGENT_H
#define LIVE_STUDIO_ANDROID_LIVE_VIDEO_QUALITY_AGENT_H

#include "../platform_dependent/platform_4_live_common.h"
//#include "live_packet_pool.h"
#include <vector>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#define STRATEGY_REDUCTION_ONLY 0 //分成三档的模式 (只降)
#define STRATEGY_REDUCTION_RISE 1 //寻找平衡点的模式


enum PushVideoQuality {
    INVALID_LIVE_FLAG = -1,
    HIGH_QUALITY = 0,
    MIDDLE_QUALITY = 1,
    LOW_QUALITY = 2,
};

class VideoQualityAgent {
private:
    VideoQualityAgent();
    
    ~VideoQualityAgent();
    
    static VideoQualityAgent *instance;
    
private:
    int strategy;
    
    int windowSizeInSecs;
    int reconfigInterval;
    int threshold;
    
    long startTimeMills;
    PushVideoQuality curQuality;
    int pubRateWarningCnt;
    
    int observeCnt;
    
    const int STRATEGY_REDUCTION_RISE_CIRCLE = 5;
    const static int FPS_KIND_NUMBER = 3;
    int fitFpsForBitrate[FPS_KIND_NUMBER] = {18, 20, 24};
    int bitrateThreshold[FPS_KIND_NUMBER] = {300, 400, 500};
    
    const static int DEFAULT_MAXIMUM_BITRATE = 800;
    const static int DEFAULT_MINIMUM_BITRATE = 300;
    const static int DEFAULT_WINDOWSIZE = 3;
    const static int DEFAULT_CONFIG_INTERVAL_REDUCTION_RISE = 3;
    const static int DEFAULT_CONFIG_INTERVAL_REDUCTION_ONLY = 15;
    
    int max_bit_rate = 800;
    int min_bit_rate = 300;
    
    std::vector<int> sendBitRatesVec;
    std::vector<int> compressBitRatesVec;
    std::vector<float> queueStatesVec;
    
    const int MAX_LOW_BITRATE_TIME_THRESHOLD = 3;
    int sendLowBitrateTimes = 0;                        //统计发送低码率的次数，若连续MAX_LOW_BITRATE_TIME_THRESHOLD次 < MINIMUM_BITRATE，停止推流
    int rapidChangeTimes = 0;                           //记录网络状态变化较大的情况，连续两次触发调整，而不是等一个周期再调整
    const int MAX_RAPID_CHANGE_TIME_THRESHOLD = 2;
    const float BITERATE_CHANGERATIO = 0.1;             //码率调整率
    const float QUEUE_EMPTY_THRESHOLD = 0.1;           //需要升码率时的队列长度阈值
    const float QUEUE_WARNING_THRESHOLD = 0.7;          //需要降码率时的队列长度阈值
    const int RAPID_CHANGE_THRESHOLD = 300;             //网络急剧变化的幅度阈值
    
    int avgSendRatePerCircle = 0;
    int avgCompressRatePerCircle = 0;
    float avgQueueStatePerCircle = 0.0;
    
    const int BITRATE_NOT_NEED_ADJUSTMENT = 0;
    const int BITRATE_NEED_ADJUSTMENT = 1;
    
    int curBitrate;
    /** 自适应码率变化曲线 **/
    std::string adptiveBitrateChart;
    
    typedef int (*statistics_bitrate_callback)(int sendBitrate, int compressedBitrate, void *context);
    statistics_bitrate_callback statisticsBitrateCallback;
    
    typedef int (*hot_adaptive_bitrate_callback)(int maxBitrate, int avgBitrate, int fps,
    void *context);
    
    hot_adaptive_bitrate_callback hotAdaptiveBitrateCallback;
    
    /** 当由于网络速度变化, 我们做的自适应码率的调节 **/
    typedef int (*on_adaptive_bitrate_callback)(PushVideoQuality videoQuality, void *context);
    
    /** 当我们发送过程中, 网络发生抖动，或者网速比较低速，我们要进行自适应码率的调整，这里是反馈机制 **/
    on_adaptive_bitrate_callback onAdaptiveBitrateCallback;
    void *adaptiveBitrateContext;
    
    //自适应策略：根据发送比特率调整fps
    int findBestFps(int bitrate);
    int preAdaptiveSendRate = 0;
    
    void getAvgVecDataInCircle();
    void resetAvgData();
    void adaptBitrate();
    
    int getMaxBitrate(int bitrate);
public:
    
    static VideoQualityAgent *GetInstance(); //工厂方法(用来获得实例)
    
    void init(int strategy, const std::map<std::string, int>& configMap);
    
    int getWindowSizeInSecs();
    
    int getReconfigInterval();
    
    virtual void registerStatisticsBitrateCallback(int (*statistics_bitrate_callback)(int sendBitrate, int compressedBitrate, void *context), void *context);
    
    virtual void registerHotAdaptiveBitrateCallback(int (*hot_adaptive_bitrate_callback)(int maxBitrate, int avgBitrate, int fps, void *context), void *context);
    
    virtual void registerAdaptiveBitrateCallback(int (*on_adaptive_bitrate_callback)(PushVideoQuality videoQuality, void *context), void *context);
    
    char *getAdaptiveBitrateChart() {
        return (char *) (adptiveBitrateChart.c_str());
    };
    
    void bitrateFeedback(int sendAVGBitrate, int compressedAVGBitrate, float queueState);
    
    void adjustmentBitrate(int adaptBitrate, int mode);
    
    
};

#endif //LIVE_STUDIO_ANDROID_LIVE_VIDEO_QUALITY_AGENT_H
