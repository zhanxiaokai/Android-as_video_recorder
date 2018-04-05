#include "publisher_rate_feed_back.h"
#include "live_packet_pool.h"

#define LOG_TAG "PublisherRateFeedback"

PublisherRateFeedback::PublisherRateFeedback() {
    sendLastWindowAVGBitrate = new WindowBitRate();
    sendCurWindowAVGBitrate = new WindowBitRate();
    compressLastWindowAVGBitrate = new WindowBitRate();
    compressCurWindowAVGBitrate = new WindowBitRate();
}

PublisherRateFeedback::~PublisherRateFeedback() {
    if (NULL != sendLastWindowAVGBitrate) {
        delete sendLastWindowAVGBitrate;
        sendLastWindowAVGBitrate = NULL;
    }
    if (NULL != sendCurWindowAVGBitrate) {
        delete sendCurWindowAVGBitrate;
        sendCurWindowAVGBitrate = NULL;
    }
    if (NULL != compressLastWindowAVGBitrate) {
        delete compressLastWindowAVGBitrate;
        compressLastWindowAVGBitrate = NULL;
    }
    if (NULL != compressCurWindowAVGBitrate) {
        delete compressCurWindowAVGBitrate;
        compressCurWindowAVGBitrate = NULL;
    }
}

//初始化静态成员
PublisherRateFeedback *PublisherRateFeedback::instance = new PublisherRateFeedback();

PublisherRateFeedback *PublisherRateFeedback::GetInstance() {
    return instance;
}

void PublisherRateFeedback::init(int audioBitrate, int qualityStrategy, const std::map<std::string, int>& configMap) {
    
    qualityAgent = VideoQualityAgent::GetInstance();
    qualityAgent->init(qualityStrategy, configMap);
    
    this->startTimeMills = platform_4_live::getCurrentTimeMills();
    this->windowStartTimeMills = -1;
    this->latestNotifyTimeMills = startTimeMills;
    this->avgPublishedBitRate = 0;
    this->sendSizeInWindow = 0;
    //Compressed
    this->compressedTotalLength = 0;
    this->compressedStartTimeMills = -1;
    this->compressedFrameCount = 0;
    this->audioBitrate = audioBitrate;
    this->avgCompressedBitRate = 0;
    this->windowSizeInSecs = qualityAgent->getWindowSizeInSecs();
    this->encoderReconfigInterval = qualityAgent->getReconfigInterval();
}

void PublisherRateFeedback::registerAdaptiveBitrateCallback(int (*on_adaptive_bitrate_callback)(PushVideoQuality videoQuality, void *context),
                                                            void *context) {
    qualityAgent->registerAdaptiveBitrateCallback(on_adaptive_bitrate_callback, context);
}

void PublisherRateFeedback::registerHotAdaptiveBitrateCallback(int (*hot_adaptive_bitrate_callback)(int maxBitrate, int avgBitrate, int fps, void *context), void *context) {
    qualityAgent->registerHotAdaptiveBitrateCallback(hot_adaptive_bitrate_callback, context);
}

void PublisherRateFeedback::registerStatisticsBitrateCallback(
                                                           int (*statistics_bitrate_callback)(int sendBitrate, int compressedBitrate,
                                                                                              void *context),
                                                           void *context) {
    if (NULL != qualityAgent) {
        qualityAgent->registerStatisticsBitrateCallback(statistics_bitrate_callback, context);
    }
}

void PublisherRateFeedback::publishPktSuccess(int pktSize) {
    sendSizeInWindow += pktSize;
}

void PublisherRateFeedback::prePublishPkt() {
    long beginSendTimeMills = platform_4_live::getCurrentTimeMills();
    if (-1 == windowStartTimeMills) {
        this->windowStartTimeMills = beginSendTimeMills;
    }
    if (this->windowStartTimeMills + windowSizeInSecs * 1000 < beginSendTimeMills) {
        float actualWindowSizeInSec =
        (float) (beginSendTimeMills - this->windowStartTimeMills) / 1000.0;
        int bitRate = (int) ((sendSizeInWindow / actualWindowSizeInSec) * 8 / 1024);
        int startTimeInSecs = (int) ((windowStartTimeMills - startTimeMills) / 1000);
        int endTimeInSecs = (int) ((beginSendTimeMills - startTimeMills) / 1000);
        this->avgPublishedBitRate = (this->avgPublishedBitRate + bitRate) / 2;
        sendLastWindowAVGBitrate->clone(sendCurWindowAVGBitrate);
        sendCurWindowAVGBitrate->update(startTimeInSecs, endTimeInSecs, bitRate);
        //        LOGI("In Window [%d, %d] publishAVGBitRate is %dKbps", startTimeInSecs, endTimeInSecs, bitRate);
        this->windowStartTimeMills = beginSendTimeMills;
        sendSizeInWindow = 0;
    }
}

bool PublisherRateFeedback::detectNotifyEncoderReConfigure() {
    bool ret = false;
    long currentTimeMills = platform_4_live::getCurrentTimeMills();
    if ((currentTimeMills - latestNotifyTimeMills) > encoderReconfigInterval) {
        latestNotifyTimeMills = currentTimeMills;
        int compressAVGBitrate = compressCurWindowAVGBitrate->bitRate;
        int sendAVGBitrate = sendCurWindowAVGBitrate->bitRate;
        int curCurDiff = abs(compressCurWindowAVGBitrate->startTimeInSecs -
                             sendCurWindowAVGBitrate->startTimeInSecs);
        int curLastDiff = abs(compressCurWindowAVGBitrate->startTimeInSecs -
                              sendLastWindowAVGBitrate->startTimeInSecs);
        int lastCurDiff = abs(compressLastWindowAVGBitrate->startTimeInSecs -
                              sendCurWindowAVGBitrate->startTimeInSecs);
        int lastLastDiff = abs(compressLastWindowAVGBitrate->startTimeInSecs -
                               sendLastWindowAVGBitrate->startTimeInSecs);
        if (curCurDiff <= curLastDiff && curCurDiff <= lastCurDiff && curCurDiff <= lastLastDiff) {
            compressAVGBitrate = compressCurWindowAVGBitrate->bitRate;
            sendAVGBitrate = sendCurWindowAVGBitrate->bitRate;
        } else if (curLastDiff < curCurDiff && curLastDiff <= lastCurDiff &&
                   curLastDiff <= lastLastDiff) {
            compressAVGBitrate = compressCurWindowAVGBitrate->bitRate;
            sendAVGBitrate = sendLastWindowAVGBitrate->bitRate;
        } else if (lastCurDiff < curCurDiff && lastCurDiff <= curLastDiff &&
                   lastCurDiff <= lastLastDiff) {
            compressAVGBitrate = compressLastWindowAVGBitrate->bitRate;
            sendAVGBitrate = sendCurWindowAVGBitrate->bitRate;
        } else if (lastLastDiff < curCurDiff && lastLastDiff <= curLastDiff &&
                   lastLastDiff <= lastCurDiff) {
            compressAVGBitrate = compressLastWindowAVGBitrate->bitRate;
            sendAVGBitrate = sendLastWindowAVGBitrate->bitRate;
        }
        qualityAgent->bitrateFeedback(sendAVGBitrate, compressAVGBitrate, videoPacketQueueState);
    }
    return ret;
}

int PublisherRateFeedback::getCompressedBitRate() {
    return avgCompressedBitRate;
}

int PublisherRateFeedback::getPublishedBitRate() {
    return avgPublishedBitRate;
}

void PublisherRateFeedback::statisticCompressData(double timeMills, int dataSize,
                                                  float videoPacketQueueState) {
    if (-1 == compressedStartTimeMills) {
        compressedStartTimeMills = timeMills;
    }
    if (windowSizeInSecs != 0 &&
        (timeMills - compressedStartTimeMills) >= windowSizeInSecs * 1000) {
        int bitRate =
        (int) (compressedTotalLength * 8 / windowSizeInSecs / 1024) + audioBitrate / 1024;
        int startTimeInSecs = (int) (compressedStartTimeMills / 1000);
        int endTimeInSecs = (int) (timeMills / 1000);
        this->avgCompressedBitRate = (this->avgCompressedBitRate + bitRate) / 2;
        compressLastWindowAVGBitrate->clone(compressCurWindowAVGBitrate);
        compressCurWindowAVGBitrate->update(startTimeInSecs, endTimeInSecs, bitRate);
        //        LOGI("In Window [%d, %d] avg Bit Rate is %dKbps fps is %d", startTimeInSecs, endTimeInSecs, bitRate, compressedFrameCount / WINDOW_SIZE_IN_SECS);
        compressedTotalLength = 0;
        compressedFrameCount = 0;
        compressedStartTimeMills = timeMills;
        detectNotifyEncoderReConfigure();
        //
    }
    compressedTotalLength += dataSize;
    compressedFrameCount += 1;
    this->videoPacketQueueState = videoPacketQueueState;
}
