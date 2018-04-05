package com.changba.songstudio.recording.video.service.factory;

import com.changba.songstudio.recording.RecordingImplType;
import com.changba.songstudio.recording.camera.preview.ChangbaRecordingPreviewScheduler;
import com.changba.songstudio.recording.service.RecorderService;
import com.changba.songstudio.recording.service.impl.AudioRecordRecorderServiceImpl;
import com.changba.songstudio.recording.video.service.MediaRecorderService;
import com.changba.songstudio.recording.video.service.impl.MediaRecorderServiceImpl;

public class MediaRecorderServiceFactory {
	private static MediaRecorderServiceFactory instance = new MediaRecorderServiceFactory();
	private MediaRecorderServiceFactory() {}
	public static MediaRecorderServiceFactory getInstance() {
		return instance;
	}

	public MediaRecorderService getRecorderService(ChangbaRecordingPreviewScheduler scheduler, RecordingImplType recordingImplType) {
		RecorderService recorderService = getAudioRecorderService(recordingImplType);
		MediaRecorderService result = new MediaRecorderServiceImpl(recorderService, scheduler);
		return result;
	}

	protected RecorderService getAudioRecorderService(
			RecordingImplType recordingImplType) {
		RecorderService recorderService = null;
		switch (recordingImplType) {
		case ANDROID_PLATFORM:
			recorderService = AudioRecordRecorderServiceImpl.getInstance();
			break;
		default:
			break;
		}
		return recorderService;
	}

}
