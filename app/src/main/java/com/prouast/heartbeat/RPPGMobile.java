package com.prouast.heartbeat;

/**
 * Created by prouast on 22/05/16.
 */
public class RPPGMobile {

    /**
     * Listener must implement this interface.
     */
    public interface RPPGListener {
        void onRPPGResult(RPPGResult result);
    }

    public RPPGMobile() {
        self = _initialise();
    }

    public void load(RPPGListener listener,
                     int width, int height,
                     int samplingFrequency, int rescanInterval,
                     String logFilePath, String faceClassifierFilename, String leftEyeClassifierFilename, String rightEyeClassifierFilename,
                     boolean log, boolean draw) {
        _load(self, listener, width, height, 0.001, samplingFrequency, rescanInterval, logFilePath, faceClassifierFilename, leftEyeClassifierFilename, rightEyeClassifierFilename, log, draw);
    }

    public void exit() {
        _exit(self);
    }

    public void processFrame(long frameRGB, long frameGray, long now) {
        _processFrame(self, frameRGB, frameGray, now);
    }

    private long self = 0;
    private static native long _initialise();
    private static native void _load(long self, RPPGListener listener, int width, int height, double timeBase, int samplingFrequency, int rescanInterval, String logFilePath, String faceClassifierFilename, String leftEyeClassifierFilename, String rightEyeClassifierFilename, boolean log, boolean draw);
    private static native void _processFrame(long self, long frameRGB, long frameGray, long now);
    private static native void _exit(long self);
}
