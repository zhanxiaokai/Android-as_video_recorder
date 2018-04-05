package com.changba.songstudio;

import com.changba.songstudio.decoder.Mp3Decoder;
import com.changba.songstudio.decoder.MusicDecoder;
import com.changba.songstudio.recorder.NativeRecordProcessor;
import com.changba.songstudio.recorder.RecordProcessor;
import com.changba.songstudio.recording.service.impl.AudioRecordRecorderServiceImpl;

import android.content.Context;

public class Songstudio {
	private static Songstudio instance = new Songstudio();

	private Context context;

	private Songstudio() {
	}

	public static Songstudio getInstance() {
		return instance;
	}

	public void initWithContext(Context context) {
		this.context = context.getApplicationContext();
//		GPUModelDetector.getInstance().detectGPUSurpportAdvancedEffect();
	}

	public Context getContext() {
		return this.context;
	}

	private native void init();

	/**
	 * 保存leaktracer测试结果日志，如果JNI代码编译时没有传入AUTO_TEST参数，此方法不执行任何代码
	 * @param leakTraceLogFullFilePath 完整路径，包含文件路径和文件名称
	 * @param sleepInSeconds 为了保证其它资源的释放减少误报, 保存日志前的等待时间，单位为秒。<=0则不等待
	 */
	public native void saveLeakTracerLog(String leakTraceLogFullFilePath, int sleepInSeconds);

	/**
	 * 获取编译songstudio时工程的commit id: git 通过命令git rev-parse --short HEAD获得; svn通过命令svn info | awk '/^Revision:/ {print $2}' 获得
	 * @return 编译时工程的commit id或者revision id; 如果不是svn或git工程，则返回空字符串
	 */
	public native String getCommitVersion();

	public void stopAudioRecord() {
		AudioRecordRecorderServiceImpl.getInstance().stop();
	}

	/** 由于各种手机的录音的minBufferSize不一样大小 所以得计算每个buffer的大小 **/
	private PacketBufferTimeEnum packetBufferTime = PacketBufferTimeEnum.TWENTY_PERCENT;

	public PacketBufferTimeEnum getPacketBufferTime() {
		return packetBufferTime;
	}

	public void resetPacketBufferTime() {
		packetBufferTime = PacketBufferTimeEnum.TWENTY_PERCENT;
	}

	public void upPacketBufferTimeLevel() {
		packetBufferTime = packetBufferTime.Upgrade();
	}

	/** 录音核心的伴奏解码器 录音处理器 **/
	private Mp3Decoder currentDecoder;

	public Mp3Decoder getMp3Decoder() {
		if (null == currentDecoder || !(currentDecoder instanceof MusicDecoder)) {
			currentDecoder = new MusicDecoder();
		}
		// currentDecoder = new MockMp3Decoder();
		return currentDecoder;
	}

	private RecordProcessor audioRecorder;

	public RecordProcessor getAudioRecorder() {
		if (null == audioRecorder) {
			audioRecorder = new NativeRecordProcessor();
		}
		// currentDecoder = new MockAudioRecorder();
		return audioRecorder;
	}

}
