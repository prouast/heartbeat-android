package com.prouast.heartbeat;

public class RPPGSimple {

    /**
     * Listener must implement this interface.
     */
    public interface RPPGListener {
        void onRPPGResult(RPPGResult result);
    }

    public RPPGSimple(RPPGListener listener,
                      int width, int height,
                      int samplingFrequency, int rescanInterval,
                      String logFilePath, String faceClassifierFilename, String leftEyeClassifierFilename, String rightEyeClassifierFilename,
                      boolean log, boolean draw) {
        self = _initialise(listener, width, height, samplingFrequency, rescanInterval, logFilePath, faceClassifierFilename, leftEyeClassifierFilename, rightEyeClassifierFilename, log, draw);
    }

    public void exit() {
        _exit(self);
    }

    public void processFrame(long frameRGB, long frameGray, long now) {
        _processFrame(self, frameRGB, frameGray, now);
    }

    private long self = 0;
    private static native long _initialise(RPPGListener listener, int width, int height, int samplingFrequency, int rescanInterval, String logFilePath, String faceClassifierFilename, String leftEyeClassifierFilename, String rightEyeClassifierFilename, boolean log, boolean draw);
    private static native void _processFrame(long self, long frameRGB, long frameGray, long now);
    private static native void _exit(long self);
}