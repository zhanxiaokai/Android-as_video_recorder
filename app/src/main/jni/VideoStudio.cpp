#include "com_changba_songstudio_Videostudio.h"

#define LOG_TAG "Videostudio"

VideoPacketConsumerThread* videoPacketConsumerThread = NULL;
jobject g_obj = 0;

JNIEXPORT jint JNICALL Java_com_changba_songstudio_Videostudio_startCommonVideoRecord(JNIEnv * env, jobject obj, jstring outputPath,
		jint videoWidth, jint videoheight, jint videoFrameRate, jint videoBitRate,jint audioSampleRate, jint audioChannels,
		jint audioBitRate,
		jint qualityStrategy,
		jint adaptiveBitrateWindowSizeInSecs,
		jint adaptiveBitrateEncoderReconfigInterval,
		jint adaptiveBitrateWarCntThreshold,
		jint adaptiveMinimumBitrate,
		jint adaptiveMaximumBitrate
		) {
	int initCode = 0;
	JavaVM *g_jvm = NULL;
	env->GetJavaVM(&g_jvm);
	g_obj = env->NewGlobalRef(obj);
	char* videoPath = (char*) (env->GetStringUTFChars(outputPath, NULL));
	//如果视频临时文件存在则删除掉
	remove(videoPath);
	videoPacketConsumerThread = new VideoPacketConsumerThread();
	/** 预先初始化3个队列, 防止在init过程中把Statistics重新置为空的问题；
	 * 由于先初始化消费者，在初始化生产者，所以队列初始化放到这里 **/
	LiveCommonPacketPool::GetInstance()->initRecordingVideoPacketQueue();
	LiveCommonPacketPool::GetInstance()->initAudioPacketQueue(audioSampleRate);
	LiveAudioPacketPool::GetInstance()->initAudioPacketQueue();

	std::map<std::string, int> configMap;

	//设置码率变化策略参数
	configMap["adaptiveBitrateWindowSizeInSecs"] = adaptiveBitrateWindowSizeInSecs;
	configMap["adaptiveBitrateEncoderReconfigInterval"] = adaptiveBitrateEncoderReconfigInterval;
	configMap["adaptiveBitrateWarCntThreshold"] = adaptiveBitrateWarCntThreshold;
	configMap["adaptiveMinimumBitrate"] = adaptiveMinimumBitrate;
	configMap["adaptiveMaximumBitrate"] = adaptiveMaximumBitrate;

	initCode = videoPacketConsumerThread->init(videoPath, videoWidth, videoheight, videoFrameRate, videoBitRate, audioSampleRate,
			audioChannels, audioBitRate, "libfdk_aac", qualityStrategy, configMap,g_jvm, g_obj);
	LOGI("initCode is %d...qualityStrategy:%d", initCode,qualityStrategy);
	if(initCode >= 0){
		videoPacketConsumerThread->startAsync();
	} else{
		/** 如果初始化失败, 那么就把队列销毁掉 **/
		LiveCommonPacketPool::GetInstance()->destoryRecordingVideoPacketQueue();
		LiveCommonPacketPool::GetInstance()->destoryAudioPacketQueue();
		LiveAudioPacketPool::GetInstance()->destoryAudioPacketQueue();
	}
	env->ReleaseStringUTFChars(outputPath, videoPath);
	return initCode;
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_Videostudio_stopVideoRecord(JNIEnv * env, jobject obj) {
	if (NULL != videoPacketConsumerThread) {
		videoPacketConsumerThread->stop();
		delete videoPacketConsumerThread;
		videoPacketConsumerThread = NULL;

		if (g_obj != 0){
			env->DeleteGlobalRef(g_obj);
			g_obj = 0;
		}
	}
}
