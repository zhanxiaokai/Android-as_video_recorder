#ifndef LIVE_PUBLISHER_STATISTICS_H
#define LIVE_PUBLISHER_STATISTICS_H

#include "../platform_dependent/platform_4_live_common.h"
#include "./publisher_rate_feed_back.h"

class PublisherStatistics {
private:
	long 									startTimeMills;
	int 										connectTimeMills;
	int 										publishDurationInSec;
	int 										totalPushVideoFrameCnt;
	int 										discardVideoFrameCnt;
	float 									publishAVGBitRate;
	float 									expectedBitRate;

public:
	PublisherStatistics(float expectedBitRate);
	~PublisherStatistics();

	void connectSuccess();
	void discardVideoFrame(int discardVideoPacketSize);
	void pushVideoFrame();
    void stopPublish();
    char* getAdaptiveBitrateChart();

	long getStartTimeMills(){
		return startTimeMills;
	};

	int getConnectTimeMills(){
		return connectTimeMills;
	};

	int getPublishDurationInSec(){
		return publishDurationInSec;
	};
	float getDiscardFrameRatio() {
		if(totalPushVideoFrameCnt > 0){
			return (float) discardVideoFrameCnt / (float) totalPushVideoFrameCnt;
		} else {
			return 0;
		}
	};
    
    float getRealTimePublishBitRate(){
        return PublisherRateFeedback::GetInstance()->getRealTimePublishBitRate();
    };
    
    float getRealTimeCompressedBitRate(){
        return PublisherRateFeedback::GetInstance()->getRealTimeCompressedBitRate();
    };

	float getPublishAVGBitRate(){
		return publishAVGBitRate;
	};

	float getExpectedBitRate(){
		return expectedBitRate;
	};
};

#endif // LIVE_PUBLISHER_STATISTICS_H
