package com.changba.songstudio.recorder;

public class NativeRecordProcessor implements RecordProcessor {
	private int handle;

	@Override
	public void initAudioBufferSize(int audioSampleRate, int audioBufferSize) {
		handle = init(audioSampleRate, audioBufferSize);
	}

	@Override
	public void pushAudioBufferToQueue(short[] audioSamples,
			int audioSampleSize) {
		pushAudioBufferToQueue(handle, audioSamples, audioSampleSize);
	}

	@Override
	public void flushAudioBufferToQueue() {
		flushAudioBufferToQueue(handle);
	}

	private native int init(int audioSampleRate, int audioBufferSize);

	private native void flushAudioBufferToQueue(int handle);
	private native void destroy(int handle);

	private native int pushAudioBufferToQueue(int handle, short[] audioSamples,
			int audioSampleSize);

	@Override
	public void destroy() {
		destroy(handle);
	}

}
