package com.changba.songstudio;

import com.changba.songstudio.recording.video.VideoRecordingStudio.RecordingStudioStateCallback;

public class Videostudio {

    private static Videostudio instance = new Videostudio();

    private Videostudio() {
    }

    public static Videostudio getInstance() {
        return instance;
    }

    /**
     * 开启普通录制歌曲的视频的后台Publisher线程
     **/
    public native int startCommonVideoRecord(String outputPath, int videoWidth,
                                             int videoheight, int videoFrameRate, int videoBitRate,
                                             int audioSampleRate, int audioChannels, int audioBitRate,
                                             int qualityStrategy,
                                             int adaptiveBitrateWindowSizeInSecs, 
                                             int adaptiveBitrateEncoderReconfigInterval,
                                             int adaptiveBitrateWarCntThreshold,
                                             int adaptiveMinimumBitrate,
                                             int adaptiveMaximumBitrate);

    public int startVideoRecord(String outputPath, int videoWidth,
                                int videoheight, int videoFrameRate, int videoBitRate,
                                int audioSampleRate, int audioChannels, int audioBitRate,
                                int qualityStrategy, int adaptiveBitrateWindowSizeInSecs, int adaptiveBitrateEncoderReconfigInterval,
                                int adaptiveBitrateWarCntThreshold,int adaptiveMinimumBitrate,
                                int adaptiveMaximumBitrate,
                                RecordingStudioStateCallback recordingStudioStateCallback) {
        this.setRecordingStudioStateCallback(recordingStudioStateCallback);
        return this.startCommonVideoRecord(outputPath, videoWidth, videoheight, videoFrameRate, videoBitRate, audioSampleRate,
                audioChannels, audioBitRate, qualityStrategy,adaptiveBitrateWindowSizeInSecs, adaptiveBitrateEncoderReconfigInterval,
                adaptiveBitrateWarCntThreshold, adaptiveMinimumBitrate, adaptiveMaximumBitrate);
    }

    /**
     * 停止录制视频
     **/
    public void stopRecord() {
        this.stopVideoRecord();
        this.recordingStudioStateCallback = null;
    }

    public native void stopVideoRecord();

    private RecordingStudioStateCallback recordingStudioStateCallback;

    public void setRecordingStudioStateCallback(RecordingStudioStateCallback recordingStudioStateCallback) {
        this.recordingStudioStateCallback = recordingStudioStateCallback;
    }

    public void publishTimeOutFromNative() {
        if (null != recordingStudioStateCallback) {
            recordingStudioStateCallback.onPublishTimeOut();
        }
    }

    private long connectSuccessTimeMills;

    public void connectSuccess() {
        connectSuccessTimeMills = System.currentTimeMillis();
    }

    public void statisticsCallbackFromNative(int connectTimeMills, int publishDurationInSec,
                                             float discardFrameRatio, float publishAVGBitRate, float expectedBitRate, String adaptiveBitrateChart) {
        if (null != recordingStudioStateCallback) {
            long startTimeMills = connectSuccessTimeMills - connectTimeMills;
            recordingStudioStateCallback.statisticsCallback(startTimeMills, connectTimeMills, publishDurationInSec,
                    discardFrameRatio, publishAVGBitRate, expectedBitRate, adaptiveBitrateChart);
        }
    }

    public void adaptiveVideoQualityFromNative(int videoQuality) {
        if (null != recordingStudioStateCallback) {
            recordingStudioStateCallback.adaptiveVideoQuality(videoQuality);
        }
    }

    public void hotAdaptiveVideoQualityFromNative(int maxBitrate, int avgBitrate, int fps) {
        if (null != recordingStudioStateCallback) {
            recordingStudioStateCallback.hotAdaptiveVideoQuality(maxBitrate, avgBitrate, fps);
        }
    }
    
    public void statisticsBitrateFromNative(int sendBitrate, int compressedBitrate) {
        if (null != recordingStudioStateCallback) {
            recordingStudioStateCallback.statisticsBitrateCallback(sendBitrate, compressedBitrate);
        }
    }

}