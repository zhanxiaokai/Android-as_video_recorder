package com.changba.songstudio.video.encoder;

import android.annotation.TargetApi;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Build;
import android.util.Log;
import android.view.Surface;

public class MediaCodecEncoderLifeCycle {
	protected MediaCodecMemoryEncoder memoryEncoder;
	protected MediaCodecSurfaceEncoder surfaceEncoder;
	Surface surface = null;
	
	private static boolean isUnsupportSimulator() {
		boolean result = false;
		result = (null != Build.BRAND && Build.BRAND.toLowerCase().contains("bluestacks"))
				|| (null != Build.DEVICE && Build.DEVICE.equalsIgnoreCase("nox"))
				|| (null != Build.DEVICE && Build.DEVICE.equalsIgnoreCase("vbox86p"));
		return result;
	}
	
	@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
	private static boolean testMediaCodecSurface() {
		MediaCodec instance = null;
		try {
			instance = MediaCodec.createEncoderByType("video/avc");

			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
				MediaFormat format = MediaFormat.createVideoFormat("video/avc", 720, 720);

				format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
				format.setInteger(MediaFormat.KEY_BIT_RATE, 500000);
				format.setInteger(MediaFormat.KEY_FRAME_RATE, 24);
				format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);

				instance.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
				instance.createInputSurface();
			}

			return true;

		} catch (Throwable t) {
			return false;

		} finally {
			if (instance != null) {
				try {
					instance.release();
					instance = null;
				} catch (Throwable t) {
				}
			}
		}
	}
	
	@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
	private static boolean testMediaCodecMemory() {
		MediaCodec instance = null;
		try {
			instance = MediaCodec.createEncoderByType("video/avc");
			
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
				MediaFormat format = MediaFormat.createVideoFormat("video/avc", 480, 480);

//		        format.setInteger(MediaFormat.KEY_COLOR_FORMAT,
//		                MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
//		        format.setInteger(MediaFormat.KEY_BIT_RATE, 500000);
//		        format.setInteger(MediaFormat.KEY_FRAME_RATE, 24);
//		        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);
//
//		        instance.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
			}
			
			return true;
			
		} catch (Throwable t) {
			return false;
			
		} finally {
			if (instance != null) {
				try {
					instance.release();
					instance = null;
				} catch (Throwable t) {
				}
			}
		}
	}
	
	public enum EncoderEnum {
        EncoderFFMPEG,
        EncoderMediacodecMemory,
        EncoderMediacodecSurface;
    }
	
	// decide use which encoder: FFMPEG, Mediacodec for memory or Mediacodec for surface
	public EncoderEnum considerUseWhichEncoder() {
		// simulator
		if (isUnsupportSimulator())
			return EncoderEnum.EncoderFFMPEG;
		// system above 4.3
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
			if (testMediaCodecSurface()) {
				if (MediaCodecSurfaceEncoder.IsInNotSupportedList()) {
					return EncoderEnum.EncoderFFMPEG;
				} else {
					return EncoderEnum.EncoderMediacodecSurface;
				}
			} else {
				return EncoderEnum.EncoderFFMPEG;	
			}
		} else if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
			if (testMediaCodecMemory()) {
				if (MediaCodecMemoryEncoder.IsInNotSupportedList()) {
					return EncoderEnum.EncoderFFMPEG;
				} else {
					return EncoderEnum.EncoderMediacodecMemory;
				}
			} else {
				return EncoderEnum.EncoderFFMPEG;
			}
		} else {
			return EncoderEnum.EncoderFFMPEG;
		}
	}
	
	/* MediaCodecMemoryEncoder */
	public int getSupportedColorFormat() {
		memoryEncoder = new MediaCodecMemoryEncoder();
		return memoryEncoder.GetSupportedColorFormat();
	}
	
	public boolean isInWrongColorFormatList() {
		return memoryEncoder.IsInWrongColorFormatList();
	}
	
	public boolean createVideoEncoder(int width, int height, int frameRate, int colorFormat, int iFrameInterval, int bitRate) {
		return memoryEncoder.CreateVideoEncoder(width, height, frameRate, colorFormat, iFrameInterval, bitRate);
	}
	
	public long videoEncodeFromBuffer(byte[] inputData, long timeStamp, byte[] outputData) {
		return memoryEncoder.VideoEncodeFromBuffer(inputData, timeStamp, outputData);
	}
	
	public void clearUpVideoEncoder() {
		memoryEncoder.ClearUp();
	}
	
	/* MediaCodecSurfaceEncoder */
	public void createMediaCodecSurfaceEncoder(int width, int height, int bitRate, int frameRate, String outputPath) {
		try {
			surfaceEncoder = new MediaCodecSurfaceEncoder(width, height, bitRate, frameRate, outputPath);
			surface = surfaceEncoder.getInputSurface();
		}
		catch (Exception e) {
			Log.e("problem", "createMediaCodecSurfaceEncoder failed");
		}
	}
	
	public Surface getEncodeSurface() {
		return surface;
	}
	
	public void closeMediaCodec() {
		if (null != surfaceEncoder) {
			surfaceEncoder.shutdown();
		}
	}

	public void drainEncodedData() {
		if (null != surfaceEncoder) {
			surfaceEncoder.drainEncoder();
		}
	}
}
