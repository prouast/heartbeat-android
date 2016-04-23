package com.prouast.heartbeat;

/**
 * A result returned from the HeartRateMonitor
 */
public class RPPGResult {

    private double mean = Double.NaN;
    private double min = Double.NaN;
    private double max = Double.NaN;
    private long time = 0L;

    /**
     * Constructor
     * @param time
     * @param mean
     * @param min
     * @param max
     */
    public RPPGResult(long time, double mean, double min, double max) {
        this.time = time;
        this.mean = mean;
        this.min = min;
        this.max = max;
    }

    /**
     * Getter for time
     * @return time
     */
    public long getTime() {
        return time;
    }

    public double getMean() {
        return mean;
    }

    public double getMin() {
        return min;
    }

    public double getMax() {
        return max;
    }
}