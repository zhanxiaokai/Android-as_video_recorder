package com.changba.songstudio.decoder;

public class MusicDecoder implements Mp3Decoder {

	/** 主播开始播放 初始化播放器 **/
	public native void init(float packetBufferTimePercent, int vocalSampleRate);
	
	/** 读取要播放的数据 **/
	@Override
	public int readSamples(short[] samples, int[] slientSizeArr, int[] extraSampleType) {
		return readSamples(samples, samples.length, slientSizeArr, extraSampleType);
	}
	private native int readSamples(short[] samples, int size, int[] slientSizeArr, int[] extraSampleType);
	
	/** 由于允许在唱歌的时候可以调节伴奏音量并且要实时听到效果  这里就是设置音量的接口 **/
	public native void setAccompanyVolume(float volume, float accompanyMax);
	
	/** 开启一个新的伴奏 **/
	@Override
	public native void startAccompany(String path);

	/** 暂停当前播放的伴奏 **/
	@Override
	public native void pauseAccompany();

	/** 继续播放当前的伴奏 **/
	@Override
	public native void resumeAccompany();

	/** 结束当前播放的伴奏 **/
	@Override
	public native void stopAccompany();

	/** 主播结束直播 关掉播放器  **/
	public native void destory();

}
