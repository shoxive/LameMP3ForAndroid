package com.shoxive.lamemp3;

/**
 * Created by shoxive on 17/11/20.
 */

public class LameSDK {
    /**
     * 更新进度，供C++调用
     *
     * @param progress
     */
    public void updateProgress(int progress) {
    }

    public native static String getLameVersion();

    public native static void close();

    public native static void convert(String pcm, String mp3);

    public native static int encode(short[] buffer_l, short[] buffer_r, int samples, byte[] mp3buf);

    public native static int flush(byte[] mp3buf);

    public native static void init(int inSampleRate, int outChannel, int outSampleRate, int outBitrate, int quality);

    public static void init(int inSampleRate, int outChannel, int outSampleRate, int outBitrate) {
        init(inSampleRate, outChannel, outSampleRate, outBitrate, 7);
    }
}
