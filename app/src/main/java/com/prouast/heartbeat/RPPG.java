package com.prouast.heartbeat;

/**
 * Created by prouast on 22/05/16.
 */
public class RPPG {

    public enum RPPGAlgorithm {
        g, pca, xminay
    }

    /**
     * Listener must implement this interface.
     */
    public interface RPPGListener {
        void onRPPGResult(RPPGResult result);
    }

    public RPPG() {
        self = _initialise();
    }

    public void load(RPPGListener listener,
                     RPPGAlgorithm algorithm,
                     int width, int height, double timeBase, int downsample,
                     double samplingFrequency, double rescanFrequency,
                     int minSignalSize, int maxSignalSize,
                     String logPath, String classifierPath,
                     boolean log, boolean gui) {
        _load(self, listener, algorithm.ordinal(), width, height, timeBase, downsample, samplingFrequency, rescanFrequency, minSignalSize, maxSignalSize, logPath, classifierPath, log, gui);
    }

    public void exit() {
        _exit(self);
    }

    public void processFrame(long frameRGB, long frameGray, long now) {
        _processFrame(self, frameRGB, frameGray, now);
    }

    private long self = 0;
    private static native long _initialise();
    private static native void _load(long self, RPPGListener listener, int algorithm, int width, int height, double timeBase, int downsample, double samplingFrequency, double rescanFrequency, int minSignalSize, int maxSignalSize, String logPath, String classifierPath, boolean log, boolean gui);
    private static native void _processFrame(long self, long frameRGB, long frameGray, long time);
    private static native void _exit(long self);
}
