#include "live_video_quality_agent.h"

#define LOG_TAG "VideoQualityAgent"

VideoQualityAgent::VideoQualityAgent() {
    
}

VideoQualityAgent::~VideoQualityAgent() {
    
}

//初始化静态成员
VideoQualityAgent *VideoQualityAgent::instance = new VideoQualityAgent();

VideoQualityAgent *VideoQualityAgent::GetInstance() {
    return instance;
}

void VideoQualityAgent::registerStatisticsBitrateCallback(int (*statistics_bitrate_callback)(int, int, void *), void *context) {
    this->statisticsBitrateCallback = statistics_bitrate_callback;
    this->adaptiveBitrateContext = context;
}

void VideoQualityAgent::registerHotAdaptiveBitrateCallback(int (*hot_adaptive_bitrate_callback)(int, int, int, void *), void *context) {
    this->hotAdaptiveBitrateCallback = hot_adaptive_bitrate_callback;
    this->adaptiveBitrateContext = context;
}

void VideoQualityAgent::registerAdaptiveBitrateCallback(int (*on_adaptive_bitrate_callback)(PushVideoQuality, void *), void *context) {
    this->onAdaptiveBitrateCallback = on_adaptive_bitrate_callback;
    this->adaptiveBitrateContext = context;
}

void VideoQualityAgent::init(int strategy, const std::map<std::string, int>& configMap) {
    this->strategy = strategy;
    
    if (configMap.find("adaptiveBitrateWindowSizeInSecs") != configMap.end()){
        this->windowSizeInSecs = configMap.find("adaptiveBitrateWindowSizeInSecs")->second;
    }
    if (configMap.find("adaptiveBitrateEncoderReconfigInterval") != configMap.end()){
        this->reconfigInterval = configMap.find("adaptiveBitrateEncoderReconfigInterval")->second;
    }
    if (configMap.find("adaptiveBitrateWarCntThreshold") != configMap.end()){
        this->threshold = configMap.find("adaptiveBitrateWarCntThreshold")->second; //新策略没用这个参数
    }
    if (configMap.find("adaptiveMinimumBitrate") != configMap.end()){
        this->min_bit_rate = configMap.find("adaptiveMinimumBitrate")->second; //最低推码率
    }
    if (configMap.find("adaptiveMaximumBitrate") != configMap.end()){
        this->max_bit_rate = configMap.find("adaptiveMaximumBitrate")->second; //最低推码率
    }
    
    if (this->windowSizeInSecs <= 0){
        this->windowSizeInSecs = DEFAULT_WINDOWSIZE;
    }
    
    if (this->reconfigInterval <= 0){
        this->reconfigInterval = (strategy == STRATEGY_REDUCTION_RISE) ? DEFAULT_CONFIG_INTERVAL_REDUCTION_RISE : DEFAULT_CONFIG_INTERVAL_REDUCTION_ONLY;
    }
    
    if (this->max_bit_rate <= 0){
        this->max_bit_rate = DEFAULT_MAXIMUM_BITRATE;
    }
    
    if (this->min_bit_rate <= 0){
        this->min_bit_rate = DEFAULT_MINIMUM_BITRATE;
    }
    
    this->curQuality = HIGH_QUALITY;
    this->pubRateWarningCnt = 0;
    this->curBitrate = 0;
    this->startTimeMills = platform_4_live::getCurrentTimeMills();
    
    this->observeCnt = 0;
    
    this->adptiveBitrateChart = (strategy == STRATEGY_REDUCTION_RISE) ? "Auto" : "High";
    
    LOGI("VideoQualityAgent::init(strategy:%d, windowSizeInSecs:%d, reconfigInterval:%d, max_bit_rate:%d, min_bit_rate:%d)",strategy,
         windowSizeInSecs, reconfigInterval, max_bit_rate, min_bit_rate);
}

int VideoQualityAgent::getWindowSizeInSecs() {
    return windowSizeInSecs;
}

int VideoQualityAgent::getReconfigInterval() {
    return reconfigInterval;
}

int  VideoQualityAgent::findBestFps(int bitrate) {
    for (int i = FPS_KIND_NUMBER-1;i >= 0; --i){
        if (bitrateThreshold[i] <= bitrate){
            return fitFpsForBitrate[i];
        }
    }
    
    return fitFpsForBitrate[0];
}

int VideoQualityAgent::getMaxBitrate(int bitrate) {
#ifdef __ANDROID__
    return bitrate;
#elif defined(__APPLE__)
    if (bitrate > 550) {
        return bitrate + 50;
    } else if (bitrate >= 350 && bitrate <= 550) {
        return bitrate + 25;
    } else {
        return bitrate + 20;
    }
#endif
}

void VideoQualityAgent::adaptBitrate() {
    int adaptiveBitrate = 0;
    if (avgQueueStatePerCircle > QUEUE_EMPTY_THRESHOLD && avgQueueStatePerCircle < QUEUE_WARNING_THRESHOLD) {
        adaptiveBitrate = avgSendRatePerCircle;
        LOGI("码率根据发送码率调整:%d", avgSendRatePerCircle);
        adjustmentBitrate(adaptiveBitrate, BITRATE_NOT_NEED_ADJUSTMENT);
    }else if (avgQueueStatePerCircle <= QUEUE_EMPTY_THRESHOLD ){
#ifdef __ANDROID__
        float upRate = (avgSendRatePerCircle > preAdaptiveSendRate) ?
        avgSendRatePerCircle : preAdaptiveSendRate;
        
        adaptiveBitrate = (1 + BITERATE_CHANGERATIO) * upRate;
        LOGI("升码率:%d", adaptiveBitrate);
        adjustmentBitrate(adaptiveBitrate, BITRATE_NEED_ADJUSTMENT);
#elif defined(__APPLE__)
        adaptiveBitrate = (1 + BITERATE_CHANGERATIO) * avgSendRatePerCircle;
        if (adaptiveBitrate > preAdaptiveSendRate) {
            LOGI("升码率:%d", adaptiveBitrate);
            adjustmentBitrate(adaptiveBitrate, BITRATE_NEED_ADJUSTMENT);
        }
#endif
    }else if (avgQueueStatePerCircle >= QUEUE_WARNING_THRESHOLD){
        adaptiveBitrate = (1 - BITERATE_CHANGERATIO)*avgSendRatePerCircle;
        LOGI("降码率:%d", adaptiveBitrate);
        adjustmentBitrate(adaptiveBitrate, BITRATE_NEED_ADJUSTMENT);
    }
}

void VideoQualityAgent::bitrateFeedback(int sendAVGBitrate, int compressedAVG, float queueState) {
    LOGI("preSetBitrate:%d sendAVGBitrate %d, compressedAVGBitrate %d queueState %f",
         preAdaptiveSendRate,
         sendAVGBitrate, compressedAVG,
         queueState);
    
    if (strategy == STRATEGY_REDUCTION_ONLY) {
        if (sendAVGBitrate < (compressedAVG - 50)) {
            pubRateWarningCnt++;
            if (sendAVGBitrate < (compressedAVG - 100)) {
                pubRateWarningCnt++;
                if (sendAVGBitrate < (compressedAVG - 150)) {
                    pubRateWarningCnt++;
                }
            }
        } else if (sendAVGBitrate >= (compressedAVG + 50)) {
            pubRateWarningCnt--;
            if (sendAVGBitrate >= (compressedAVG + 100)) {
                pubRateWarningCnt--;
                if (sendAVGBitrate >= (compressedAVG + 150)) {
                    pubRateWarningCnt--;
                }
            }
        }
        if (pubRateWarningCnt >= threshold && NULL != onAdaptiveBitrateCallback) {
            int publishDurationInSecs = (int) (
                                               (platform_4_live::getCurrentTimeMills() - this->startTimeMills) /
                                               1000);
            std::stringstream ss;
            std::string adaptiveInSecs;
            ss << publishDurationInSecs;
            ss >> adaptiveInSecs;
            //作用到编码器去降低码率和帧率
            if (curQuality == HIGH_QUALITY) {
                curQuality = MIDDLE_QUALITY;
                adptiveBitrateChart.append("-");
                adptiveBitrateChart.append(adaptiveInSecs);
                adptiveBitrateChart.append(":Middle");
            } else if (curQuality == MIDDLE_QUALITY) {
                curQuality = LOW_QUALITY;
                adptiveBitrateChart.append("-");
                adptiveBitrateChart.append(adaptiveInSecs);
                adptiveBitrateChart.append(":Low");
            } else {
                //已经降到最低码率了, 提示用户 切换网络环境
                curQuality = INVALID_LIVE_FLAG;
                adptiveBitrateChart.append("-");
                adptiveBitrateChart.append(adaptiveInSecs);
                adptiveBitrateChart.append(":Invalid");
            }
            onAdaptiveBitrateCallback(curQuality, adaptiveBitrateContext);
            pubRateWarningCnt = 0;
        }
    } else if (strategy == STRATEGY_REDUCTION_RISE) {
        observeCnt = (observeCnt + 1) % STRATEGY_REDUCTION_RISE_CIRCLE;
        LOGI("current observeCnt:%d", observeCnt);
        if (sendAVGBitrate != 0){
            sendBitRatesVec.push_back(sendAVGBitrate);
        }
        queueStatesVec.push_back(queueState);
        
        if (compressedAVG != 0){
            compressBitRatesVec.push_back(compressedAVG);
        }
        
        if ((abs(preAdaptiveSendRate - sendAVGBitrate) > RAPID_CHANGE_THRESHOLD && preAdaptiveSendRate != 0)
            || (compressedAVG - sendAVGBitrate > RAPID_CHANGE_THRESHOLD && sendAVGBitrate >= min_bit_rate)){
            ++rapidChangeTimes;
            if (rapidChangeTimes >= MAX_RAPID_CHANGE_TIME_THRESHOLD){
                //触发调整
                LOGI("网络变化较大，立刻调整");
                adjustmentBitrate(sendAVGBitrate, BITRATE_NEED_ADJUSTMENT);
                resetAvgData();
            }
        }else if (observeCnt == 0) {
        	//一个周期
            rapidChangeTimes = 0;
            getAvgVecDataInCircle();
            adaptBitrate();
            resetAvgData();
        }
    }
}

void VideoQualityAgent::getAvgVecDataInCircle() {
    float totalSendBitRates = 0.f;
    float totalCompressBitRates = 0.f;
    float totalQueueStates = 0.f;
    
    int sendVecSize = (int)sendBitRatesVec.size();
    int compressVecSize = (int)compressBitRatesVec.size();
    int queueVecSize = (int)queueStatesVec.size();
    
    for (int i = 0; i < sendVecSize; ++i) {
        totalSendBitRates += sendBitRatesVec[i];
        
        if (i == sendVecSize - 1) {
            avgSendRatePerCircle = (int) (totalSendBitRates / sendVecSize);
        }
    }
    
    for (int i = 0; i < compressVecSize; ++i) {
        totalCompressBitRates += compressBitRatesVec[i];
        
        if (i == compressVecSize - 1) {
            avgCompressRatePerCircle = (int) (totalCompressBitRates / compressVecSize);
        }
    }
    
    for (int i = 0; i < queueVecSize; ++i) {
        totalQueueStates += queueStatesVec[i];
        
        if (i == queueVecSize - 1) {
            avgQueueStatePerCircle =  (totalQueueStates / queueVecSize);
        }
    }
    
    if (statisticsBitrateCallback != NULL) {
        statisticsBitrateCallback(avgSendRatePerCircle, avgCompressRatePerCircle, adaptiveBitrateContext);
    }
    LOGI("avgSendRatePerCircle:%d, avgCompressRatePerCircle:%d, avgQueueStatePerCircle:%f",
         avgSendRatePerCircle,
         avgCompressRatePerCircle,
         avgQueueStatePerCircle);
}

void VideoQualityAgent::adjustmentBitrate(int adaptBitrate, int mode) {
    if (adaptBitrate < min_bit_rate) {
        sendLowBitrateTimes++;
        
        if (sendLowBitrateTimes >= MAX_LOW_BITRATE_TIME_THRESHOLD){
            //需要停止推流
            adaptBitrate = -1;
        }else{
            adaptBitrate = min_bit_rate;
        }
        
        LOGI("已低于最低码率,%d", sendLowBitrateTimes);
    }else{
        if (sendLowBitrateTimes != 0){
            LOGI("重置最低码计数");
            sendLowBitrateTimes = 0;
        }
    }
    
    if (adaptBitrate > max_bit_rate) {
        adaptBitrate = max_bit_rate;
    }
    
    
    if (mode == BITRATE_NEED_ADJUSTMENT){
        //按新速率调
        int fitFps = findBestFps(adaptBitrate);
        int maxBitrate = getMaxBitrate(adaptBitrate);
        LOGI("变化模式：调整码率:%d, fps:%d", adaptBitrate, fitFps);
        preAdaptiveSendRate = adaptBitrate;
        if (hotAdaptiveBitrateCallback != NULL){
            if (adaptBitrate == -1) {
                int publishDurationInSecs = (int)((platform_4_live::getCurrentTimeMills() - this->startTimeMills) / 1000);
                std::stringstream ss;
                std::string adaptiveInSecs;
                ss << publishDurationInSecs;
                ss >> adaptiveInSecs;
                adptiveBitrateChart.append("-");
                adptiveBitrateChart.append(adaptiveInSecs);
                adptiveBitrateChart.append(":Invalid");
            }
            hotAdaptiveBitrateCallback(maxBitrate ,
                                       adaptBitrate ,
                                       fitFps,
                                       adaptiveBitrateContext);
        }
    }else{
        //找到稳定模式
        if (preAdaptiveSendRate == BITRATE_NOT_NEED_ADJUSTMENT) {
            preAdaptiveSendRate = adaptBitrate;
        }
        
        adaptBitrate = preAdaptiveSendRate;
        int fitFps = findBestFps(adaptBitrate);
        int maxBitrate = getMaxBitrate(adaptBitrate);
        LOGI("稳定模式：按之前的速度继续调整:%d", preAdaptiveSendRate);
        if (hotAdaptiveBitrateCallback != NULL) {
            hotAdaptiveBitrateCallback(maxBitrate ,
                                       adaptBitrate ,
                                       fitFps,
                                       adaptiveBitrateContext);
        }
    }
}

void VideoQualityAgent::resetAvgData() {
    //需要重新统计平均值了
    observeCnt = 0;
    rapidChangeTimes = 0;
    
    sendBitRatesVec.clear();
    sendBitRatesVec.reserve(STRATEGY_REDUCTION_RISE_CIRCLE);
    compressBitRatesVec.clear();
    compressBitRatesVec.reserve(STRATEGY_REDUCTION_RISE_CIRCLE);
    queueStatesVec.clear();
    queueStatesVec.reserve(STRATEGY_REDUCTION_RISE_CIRCLE);
}
