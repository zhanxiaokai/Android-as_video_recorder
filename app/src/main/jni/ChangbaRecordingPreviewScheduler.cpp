#include "com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler.h"

#define LOG_TAG "ChangbaRecordingPreviewScheduler"

static MVRecordingPreviewController *previewController = 0;
static jobject g_obj = 0;

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_switchPreviewFilter(
        JNIEnv *env, jobject obj, jint filterType, jobject assetManager, jstring filename) {
    if (NULL != previewController) {
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_switchCameraFacing(
        JNIEnv *env, jobject obj) {
    if (NULL != previewController) {
        previewController->switchCameraFacing();
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_adaptiveVideoQuality(
        JNIEnv *env, jobject obj, jint maxBitRate, jint avgBitRate, jint fps) {
    if (NULL != previewController) {
        previewController->adaptiveVideoQuality(maxBitRate, avgBitRate, fps);
    }
}

JNIEXPORT void JNICALL
Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_hotConfigQuality(
        JNIEnv *env, jobject instance, jint max, jint avg, jint fps) {
    if (NULL != previewController) {
        previewController->hotConfigQuality(max, avg, fps);
    }

}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_startEncoding(
        JNIEnv *env, jobject obj, jint width, jint height, jint videoBitRate, jint frameRate,
        jboolean useHardWareEncoding, jint strategy) {
    if (NULL != previewController) {
        previewController->startEncoding(width, height, videoBitRate, frameRate,
                                         useHardWareEncoding, strategy);
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_stopEncoding(
        JNIEnv *env, jobject obj) {
    if (NULL != previewController) {
        previewController->stopEncoding();
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_createWindowSurface(
        JNIEnv *env, jobject obj, jobject surface) {
    if (surface != 0 && NULL != previewController) {
        ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
        if (window != NULL) {
            previewController->createWindowSurface(window);
        }
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_destroyWindowSurface(
        JNIEnv *env, jobject obj) {
    if (NULL != previewController) {
        previewController->destroyWindowSurface();
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_prepareEGLContext(
        JNIEnv *env, jobject obj, jobject surface, jint screenWidth, jint screenHeight,
        jint cameraFacingId) {
    previewController = new MVRecordingPreviewController();
    JavaVM *g_jvm = NULL;
    env->GetJavaVM(&g_jvm);
    g_obj = env->NewGlobalRef(obj);
    if (surface != 0 && NULL != previewController) {
        ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
        if (window != NULL) {
            previewController->prepareEGLContext(window, g_jvm, g_obj, screenWidth, screenHeight,
                                                 cameraFacingId);
        }
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_resetRenderSize(
        JNIEnv *env, jobject obj, jint screenWidth, jint screenHeight) {
    if (NULL != previewController) {
        previewController->resetRenderSize(screenWidth, screenHeight);
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_destroyEGLContext(
        JNIEnv *env, jobject obj) {
    if (NULL != previewController) {
        previewController->destroyEGLContext();
        delete previewController;
        previewController = NULL;

        if (g_obj != 0) {
            env->DeleteGlobalRef(g_obj);
            g_obj = 0;
        }
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_notifyFrameAvailable(
        JNIEnv *env, jobject obj) {
    if (NULL != previewController) {
        previewController->notifyFrameAvailable();
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_updateTexMatrix(
        JNIEnv *env, jobject obj, jfloatArray array) {
    if (NULL != previewController) {
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_switchPauseRecordingPreviewState(
        JNIEnv *env, jobject obj) {
    if (NULL != previewController) {
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_switchCommonPreviewState(
        JNIEnv *env, jobject obj) {
    if (NULL != previewController) {
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_hotConfig(
        JNIEnv *env, jobject instance, jint bitRate, jint fps, jint gopSize) {
    LOGI(" %s", "Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_hotConfig");
    if (NULL != previewController) {
        previewController->hotConfig(bitRate, fps, gopSize);
    }
}

JNIEXPORT void JNICALL Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_setBeautifyParam(JNIEnv* env, jobject obj, jint key, jfloat value) {
	LOGI("Java_com_changba_songstudio_recording_camera_preview_ChangbaRecordingPreviewScheduler_setBeautifyParam");
	LOGI("setbeautify: %d, %f", key, value);
	if(NULL != previewController) {
	}
}


