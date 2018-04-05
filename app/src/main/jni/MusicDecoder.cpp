#include "com_changba_songstudio_decoder_MusicDecoder.h"

#define LOG_TAG "SongDecoderService"
LiveSongDecoderController* decoderController = NULL;

JNIEXPORT void JNICALL Java_com_changba_songstudio_decoder_MusicDecoder_init(JNIEnv * env, jobject obj, jfloat packetBufferTimePercent, jint vocalSamplerate) {
	decoderController = new LiveSongDecoderController();
	decoderController->init(packetBufferTimePercent, vocalSamplerate);
}

JNIEXPORT jint JNICALL Java_com_changba_songstudio_decoder_MusicDecoder_readSamples(JNIEnv * env, jobject obj, jshortArray array, jint size, jintArray extraSlientSampleSize, jintArray extraAccompanyType){
	if(NULL != decoderController) {
		jshort* target = env->GetShortArrayElements(array, 0);
		jint* slientSizeArr = env->GetIntArrayElements(extraSlientSampleSize, 0);
		jint* extraAccompanyTypeArr = env->GetIntArrayElements(extraAccompanyType, 0);
		int result = decoderController->readSamplesAndProducePacket(target, size, slientSizeArr, extraAccompanyTypeArr);
		env->ReleaseIntArrayElements(extraSlientSampleSize, slientSizeArr, 0);
		env->ReleaseIntArrayElements(extraAccompanyType, extraAccompanyTypeArr, 0);
		env->ReleaseShortArrayElements(array, target, 0);
		return result;
	}
	return -1;
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_decoder_MusicDecoder_setAccompanyVolume(JNIEnv * env, jobject obj, jfloat volume, jfloat accompanyMax){
	if(NULL != decoderController) {
		decoderController->setVolume(volume, accompanyMax);
	}
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_decoder_MusicDecoder_startAccompany(JNIEnv * env, jobject obj, jstring accompanyFilePathParam){
	if(NULL != decoderController){
		const char* accompanyFilePath = env->GetStringUTFChars(accompanyFilePathParam, NULL);
		decoderController->startAccompany(accompanyFilePath);
		env->ReleaseStringUTFChars(accompanyFilePathParam, accompanyFilePath);
	}
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_decoder_MusicDecoder_pauseAccompany(JNIEnv * env, jobject obj){
	if(NULL != decoderController){
		decoderController->pauseAccompany();
	}
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_decoder_MusicDecoder_resumeAccompany(JNIEnv * env, jobject obj){
	if(NULL != decoderController){
		decoderController->resumeAccompany();
	}
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_decoder_MusicDecoder_stopAccompany(JNIEnv * env, jobject obj){
	if(NULL != decoderController){
		decoderController->stopAccompany();
	}
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_decoder_MusicDecoder_destory(JNIEnv * env, jobject obj) {
	LOGI("enter Java_com_changba_songstudio_decoder_MusicDecoder_destory()");
	if(NULL != decoderController) {
		decoderController->destroy();
		delete decoderController;
		decoderController = NULL;
	}
}
