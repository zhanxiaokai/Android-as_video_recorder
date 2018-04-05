package com.changba.songstudio.recording.video.service.impl;

import com.changba.songstudio.recording.camera.exception.CameraParamSettingException;
import com.changba.songstudio.recording.camera.preview.ChangbaRecordingPreviewScheduler;
import com.changba.songstudio.recording.camera.preview.PreviewFilterType;
import com.changba.songstudio.recording.exception.AudioConfigurationException;
import com.changba.songstudio.recording.exception.StartRecordingException;
import com.changba.songstudio.recording.service.RecorderService;
import com.changba.songstudio.recording.video.service.MediaRecorderService;
import com.changba.songstudio.video.encoder.MediaCodecSurfaceEncoder;

import android.content.res.AssetManager;
import android.util.Log;

public class MediaRecorderServiceImpl implements MediaRecorderService {

	private RecorderService audioRecorderService;
	private ChangbaRecordingPreviewScheduler previewScheduler;

	public MediaRecorderServiceImpl(RecorderService recorderService, ChangbaRecordingPreviewScheduler scheduler) {
		this.audioRecorderService = recorderService;
		this.previewScheduler = scheduler;
	}

	public void switchCamera() throws CameraParamSettingException{
		previewScheduler.switchCameraFacing();
	}
	
	@Override
	public void initMetaData() throws AudioConfigurationException {
		audioRecorderService.initMetaData();
	}

	@Override
	public boolean initMediaRecorderProcessor() {
		return audioRecorderService.initAudioRecorderProcessor();
	}

	@Override
	public boolean start(int width, int height, int videoBitRate, int frameRate, boolean useHardWareEncoding, int strategy) throws StartRecordingException {
		audioRecorderService.start();
		if(useHardWareEncoding){
			if(MediaCodecSurfaceEncoder.IsInNotSupportedList()){
				useHardWareEncoding = false;
			}
		}
		previewScheduler.startEncoding(width, height, videoBitRate, frameRate, useHardWareEncoding, strategy);
		return useHardWareEncoding;
	}

	@Override
	public void destoryMediaRecorderProcessor() {
		audioRecorderService.destoryAudioRecorderProcessor();
	}

	@Override
	public void stop() {
		audioRecorderService.stop();
		previewScheduler.stop();
	}

	@Override
	public int getAudioBufferSize() {
		return audioRecorderService.getAudioBufferSize();
	}

	@Override
	public int getSampleRate() {
		return audioRecorderService.getSampleRate();
	}

	@Override
	public void switchPreviewFilter(AssetManager assetManager, PreviewFilterType filterType) {
		if(null != previewScheduler){
			previewScheduler.switchPreviewFilter(assetManager, filterType);;
		}
	}

	@Override
	public void enableUnAccom() {
		if (audioRecorderService != null) {
			audioRecorderService.enableUnAccom();
		}
	}

}
