package com.prouast.heartbeat;

public class FFmpegEncoder {

    public FFmpegEncoder() {
        self = _initialise();
    }

    public boolean openFile(String filename, int width, int height, int bitrate, int framerate) {
        return _openFile(self, filename, width, height, bitrate, framerate);
    }

    public void writeFrame(long dataAddr, long time) {
        _writeFrame(self, dataAddr, time);
    }

    public void closeFile() {
        _closeFile(self);
    }

    private long self = 0;
    private static native long _initialise();
    private static native boolean _openFile(long self, String filename, int width, int height, int bitrate, int framerate);
    private static native void _writeFrame(long self, long dataAddr, long time);
    private static native void _closeFile(long self);
}