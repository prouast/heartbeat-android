package com.prouast.heartbeat;

import android.content.Context;
import android.util.Log;

import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.MatOfDouble;
import org.opencv.core.MatOfInt;
import org.opencv.core.MatOfRect;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.imgproc.Imgproc;
import org.opencv.objdetect.CascadeClassifier;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * The Heartrate Monitor.
 * @author prouast
 */
public class HeartRateMonitor {

    /* Constants */
    private static final String TAG = "Heartbeat::HRM";
    private static final int CV_FILLED = -1;
    private static final Scalar BLACK = new Scalar(0, 0, 0);
    private static final Scalar WHITE = new Scalar(255, 255, 255);
    private static final Scalar RED = new Scalar(255, 0, 0);
    private static final Scalar GREEN = new Scalar(0, 255, 0);
    private static final Scalar ONE = new Scalar(1);
    private static final Scalar ZERO = new Scalar(0);
    private static final int HIGH_BPM = 240;
    private static final int LOW_BPM = 42;
    private static final double REL_MIN_FACE_SIZE = 0.2;
    private static final int SIGNAL_SIZE = 10; // Difference between peaks = 6 bpm
    private static final double SEC_PER_MIN = 60.0;
    private static final int JUMP = 1;
    private static final int NO_JUMP = 0;

    /* Application context */
    private Context applicationContext;

    /* The HRMListener */
    private HRMListener listener;

    /* Face and shape tracking */
    private CascadeClassifier faceClassifier;       // Haar classifier for face
    private CascadeClassifier leftEyeClassifier;    // Haar classifier for left eye
    private CascadeClassifier rightEyeClassifier;   // Haar classifier for right eye

    /* Customisable settings */
    private double rescanInterval;
    private int samplingFrequency;
    private Size minFaceSize;
    private boolean logMode;
    private boolean drawMode;

    /* State variables */
    private double fps;
    private double lastSamplingTC;
    private double lastScanTC;
    private double nowTC;
    private long now;
    private boolean valid;
    private boolean updateFlag;

    /* Features */
    private Rect box;
    private Rect rightEye;
    private Rect leftEye;
    private Mat mask;

    /* Physio data */
    private MatOfDouble g;
    private MatOfDouble t;
    private MatOfInt jumps;
    private MatOfDouble signal;
    private MatOfDouble bpms;
    private Mat powerSpectrum;
    private double meanBpm;
    private double minBpm;
    private double maxBpm;

    /* Logging */
    private FileOutputStream log;

    /**
     * Listener must implement this interface.
     */
    public interface HRMListener {
        void onHRMStarted();
        void onHRMStopped();
        void onHRMResult(RPPGResult result);
    }

    /**
     * Constructor
     * @param listener Listener will be notified of results
     * @param samplingFrequency Frequency of results
     */
    public HeartRateMonitor(HRMListener listener, Context context,
                            int samplingFrequency, int rescanInterval,
                            boolean log, boolean draw) {
        this.applicationContext = context;
        this.box = new Rect();
        this.drawMode = draw;
        this.leftEye = new Rect();
        this.listener = listener;
        this.logMode = log;
        this.updateFlag = false;
        this.rescanInterval = rescanInterval;
        this.rightEye = new Rect();
        this.samplingFrequency = samplingFrequency;
    }

    /**
     * Prepare the HRM for frame dimensions.
     * @param width frame width
     * @param height frame height
     */
    public void prepareHRM(int width, int height) {
        this.minFaceSize = new Size(Math.min(width, height) * REL_MIN_FACE_SIZE,
                Math.min(width, height) * REL_MIN_FACE_SIZE);
        this.mask = new Mat(height, width, CvType.CV_8UC1);
        listener.onHRMStarted();

        try {
            File file = new File(applicationContext.getExternalFilesDir(null), "android_bpm.csv");
            log = new FileOutputStream(file);
            log.write("t;bpm\n".getBytes());
        } catch (IOException e) {
            Log.i(TAG, "Error writing to log: " + e);
        }
    }

    /**
     * Load the classifiers for face and eyes.
     * @param faceClassifierFilename filename of the face classifier
     * @param leftEyeClassifierFilename filename of the left eye classifier
     * @param rightEyeClassifierFilename filename of the right eye classifier
     */
    public void loadClassifiers(String faceClassifierFilename,
                                String leftEyeClassifierFilename,
                                String rightEyeClassifierFilename) {

        faceClassifier = new CascadeClassifier(faceClassifierFilename);

        if (faceClassifier.empty()) {
            Log.e(TAG, "Failed to load face cascade classifier");
        } else {
            Log.i(TAG, "Loaded face cascade classifier from " + faceClassifierFilename);
        }

        leftEyeClassifier = new CascadeClassifier(leftEyeClassifierFilename);

        if (leftEyeClassifier.empty()) {
            Log.e(TAG, "Failed to load eye cascade classifier");
        } else {
            Log.i(TAG, "Loaded eye cascade classifier from " + leftEyeClassifierFilename);
        }

        rightEyeClassifier = new CascadeClassifier(rightEyeClassifierFilename);

        if (rightEyeClassifier.empty()) {
            Log.e(TAG, "Failed to load eye cascade classifier");
        } else {
            Log.i(TAG, "Loaded eye cascade classifier from " + rightEyeClassifierFilename);
        }
    }

    /**
     * Release the resources to avoid memory leak.
     */
    public void clearHRM() {
        try {
            log.close();
        } catch (IOException e) {
            Log.i(TAG, "Error closing log: " + e);
        }
        if (g != null) g.release();
        if (t != null) t.release();
        if (jumps != null) jumps.release();
        if (signal != null) signal.release();
        if (bpms != null) bpms.release();
        if (this.mask != null) this.mask.release();
        if (powerSpectrum != null) powerSpectrum.release();
        System.gc();
    }

    /**
     * Process a new frame from camera
     * @param frame RGB version
     * @param grayFrame Gray version
     */
    public void processFrame(Mat frame, Mat grayFrame, long now) {

        // Record timestamp
        nowTC = Core.getTickCount();
        this.now = now;

        if (!valid) {

            lastScanTC = Core.getTickCount();
            detectFace(grayFrame);

        } else if ((nowTC - lastScanTC)/Core.getTickFrequency() >= rescanInterval) {

            lastScanTC = Core.getTickCount();
            detectFace(grayFrame);
            updateFlag = true;

        }

        if (valid) {

            // Save the new signal values
            this.fps = getFps();
            while (g.rows() > fps * SIGNAL_SIZE) {
                push(g);
                push(t);
                push(jumps);
            }

            Scalar means = Core.mean(frame, mask);
            g.push_back(new MatOfDouble(means.val[1]));
            jumps.push_back(new MatOfInt(updateFlag ? JUMP : NO_JUMP));
            t.push_back(new MatOfDouble(Core.getTickCount()));

            // If signal is large enough, send off to estimation
            this.fps = getFps();
            updateFlag = false;
            if (g.rows() / fps >= SIGNAL_SIZE) {

                // Collect raw signals
                g.copyTo(signal);

                /* Apply filters */
                extractSignal_g_den_detr_mean();

                // PSD estimation
                estimateHeartrate();
            }

            // Draw on frame
            draw(frame);
        }
    }

    /**
     * Scan or rescan for face.
     */
    private void detectFace(Mat grayFrame) {

        // Detect faces with Haar classifier
        MatOfRect boxes = new MatOfRect();
        faceClassifier.detectMultiScale(grayFrame, boxes, 1.1, 2, 2, minFaceSize, new Size());

        if (boxes.rows() > 0) {
            Log.i(TAG, "Detected face");
            setNearestBox(boxes.toList());
            detectEyes(grayFrame);
            updateMask();
            if (!valid) {
                jumps = new MatOfInt();
                g = new MatOfDouble();
                t = new MatOfDouble();
                bpms = new MatOfDouble();
                signal = new MatOfDouble();
                powerSpectrum = new Mat();
                valid = true;
            }
        } else {
            if (jumps != null) jumps.release();
            if (g != null) g.release();
            if (t != null) t.release();
            if (bpms != null) bpms.release();
            if (signal != null) signal.release();
            if (powerSpectrum != null) powerSpectrum.release();
            valid = false;
        }

        boxes.release();
    }

    /**
     * Set box to the nearest
     * @param boxes choices
     */
    public void setNearestBox(List<Rect> boxes) {
        int index = 0;
        Point p = new Point(box.tl().x - boxes.get(0).tl().x,
                box.tl().y - boxes.get(0).tl().y);
        double min = p.x * p.x + p.y * p.y;
        for (int i = 1; i < boxes.size(); i++) {
            p.set(new double[]{box.tl().x - boxes.get(i).tl().x,
                    box.tl().y - boxes.get(i).tl().y});
            double d = p.x * p.x + p.y * p.y;
            if (d < min) {
                min = d;
                index = i;
            }
        }
        box = boxes.get(index);
    }

    /**
     * Detect eyes
     */
    private void detectEyes(Mat grayFrame) {

        Rect rightEyeROI = new Rect(
                box.x + box.width/16,
                (int)(box.y + (box.height/4.5)),
                (box.width - 2*box.width/16)/2,
                (int)(box.height/3.0)
        );
        Rect leftEyeROI = new Rect(
                box.x + box.width/16 + (box.width - 2*box.width/16)/2,
                (int)(box.y + (box.height/4.5)),
                (box.width - 2*box.width/16)/2,
                (int)(box.height/3.0)
        );

        Mat leftSub = grayFrame.submat(leftEyeROI);
        Mat rightSub = grayFrame.submat(rightEyeROI);

        // Detect eyes with Haar classifier
        MatOfRect eyeBoxesLeft = new MatOfRect();
        leftEyeClassifier.detectMultiScale(leftSub, eyeBoxesLeft, 1.1, 2, 0, new Size(), new Size());
        MatOfRect eyeBoxesRight = new MatOfRect();
        leftEyeClassifier.detectMultiScale(rightSub, eyeBoxesRight, 1.1, 2, 0, new Size(), new Size());

        if (eyeBoxesLeft.rows() > 0) {
            Rect leftEye = eyeBoxesLeft.toArray()[0];
            Point tl = new Point(leftEyeROI.x + leftEye.x, leftEyeROI.y + leftEye.y);
            Point br = new Point(leftEyeROI.x + leftEye.x + leftEye.width, leftEyeROI.y + leftEye.y + leftEye.height);
            this.leftEye = new Rect(tl, br);
        }
        if (eyeBoxesRight.rows() > 0) {
            Rect rightEye = eyeBoxesRight.toArray()[0];
            Point tl = new Point(rightEyeROI.x + rightEye.x, rightEyeROI.y + rightEye.y);
            Point br = new Point(rightEyeROI.x + rightEye.x + rightEye.width, rightEyeROI.y + rightEye.y + rightEye.height);
            this.rightEye = new Rect(tl, br);
        }

        eyeBoxesLeft.release();
        eyeBoxesRight.release();
    }

    /**
     * Update the mask
     */
    public void updateMask() {
        mask.setTo(BLACK);
        Imgproc.ellipse(mask,
                new Point(box.x + box.width / 2.0, box.y + box.height / 2.0),
                new Size(box.width / 2.5, box.height / 2.0),
                0, 0, 360, WHITE, CV_FILLED);
        Imgproc.circle(mask,
                new Point(leftEye.x + leftEye.width / 2.0, leftEye.y + leftEye.height / 2.0),
                (int)((leftEye.width + leftEye.height) / 4.0), BLACK, CV_FILLED);
        Imgproc.circle(mask,
                new Point(rightEye.x + rightEye.width / 2.0, rightEye.y + rightEye.height / 2.0),
                (int)((rightEye.width + rightEye.height) / 4.0), BLACK, CV_FILLED);
    }

    /**
     * Calculate frequency
     * @return fps in Hz
     */
    private double getFps() {

        double result;

        if (t.empty()) {
            result = Double.POSITIVE_INFINITY;
        } else {
            double diff = (t.get(t.rows() - 1, 0)[0] - t.get(0, 0)[0]) / Core.getTickFrequency();
            result = diff == 0 ? Double.POSITIVE_INFINITY : (t.rows() * 1.0) / diff;
        }

        return result;
    }

    /**
     * Remove the first entry
     * @param m MatOfDouble
     */
    private void push(MatOfDouble m) {
        final int length = m.rows();
        m.rowRange(1, length).copyTo(m.rowRange(0, length - 1)); // Copy back
        ArrayList<Double> temp = new ArrayList<>(m.toList());
        temp.remove(length - 1);
        m.fromList(temp);
    }

    /**
     * Remove the first entry
     * @param m MatOfInt
     */
    private void push(MatOfInt m) {
        final int length = m.rows();
        m.rowRange(1, length).copyTo(m.rowRange(0, length - 1)); // Copy back
        ArrayList<Integer> temp = new ArrayList<>(m.toList());
        temp.remove(length - 1);
        m.fromList(temp);
    }

    /**
     * TODO
     */
    private void extractSignal_g_den_detr_mean() {

        // Denoise
        MatOfDouble signalDenoised = new MatOfDouble(signal.rows(), signal.cols(), signal.type());
        denoiseFilter2(signal, signalDenoised, jumps);

        // Detrending filter
        MatOfDouble signalDetrended = new MatOfDouble(signal.rows(), signal.cols(), signal.type());
        detrendFilter(signalDenoised, (int)fps, signalDetrended);

        // Mean filtering
        meanFilter(signalDetrended, 3, Math.max((int)fps/3, 3), signal);

        if (logMode) {
            try {
                File file = new File(applicationContext.getExternalFilesDir(null), "android_signal_" + now + ".csv");
                FileOutputStream stream = new FileOutputStream(file);
                try {
                    stream.write("t;g;g_den;g_detr;g_avg\n".getBytes());
                    for (int i = 0; i < signal.rows(); i++) {
                        stream.write(("" +
                                t.get(i, 0)[0]+ ";" +
                                g.get(i, 0)[0] + ";" +
                                signalDenoised.get(i, 0)[0] + ";" +
                                signalDetrended.get(i, 0)[0] + ";" +
                                signal.get(i, 0)[0] +
                                "\n").getBytes());
                    }
                } finally {
                    stream.close();
                }
            } catch (Exception e) {
                Log.e(TAG, "Error with file stream: " + e);
            }
        }

        // Clean up
        signalDetrended.release();
        signalDenoised.release();
    }

    /**
     * Make estimation of the heart rate for every frame and report summary statistics according
     * to sampling frequency.
     */
    private void estimateHeartrate() {

        timeToFrequency(signal, powerSpectrum, true);

        // band mask
        final int total = signal.rows();
        final int low = (int)(total * LOW_BPM / SEC_PER_MIN / fps);
        final int high = (int)(total * HIGH_BPM / SEC_PER_MIN / fps);
        Mat bandMask = Mat.zeros(signal.size(), CvType.CV_8U);
        bandMask.rowRange(Math.min(low, total), Math.min(high, total)).setTo(ONE);

        if (!powerSpectrum.empty()) {

            // grab index of max power spectrum
            Core.MinMaxLocResult loc = Core.minMaxLoc(powerSpectrum, bandMask);

            // calculate BPM
            double bpm = loc.maxLoc.y * fps / total * SEC_PER_MIN;
            bpms.push_back(new MatOfDouble(bpm));

            Log.i(TAG, "FPS=" + fps + " Peak=" + loc.maxLoc.y + ", HR=" + bpm);

            if (logMode) {
                try {
                    File file = new File(applicationContext.getExternalFilesDir(null), "android_estimation_" + now + ".csv");
                    FileOutputStream stream = new FileOutputStream(file);
                    try {
                        stream.write("i;powerSpectrum\n".getBytes());
                        for (int i = 0; i < powerSpectrum.rows(); i++) {
                            if (low <= i && i <= high) {
                                stream.write(("" +
                                        i + ";" +
                                        powerSpectrum.get(i, 0)[0] +
                                        "\n").getBytes());
                            }
                        }
                    } finally {
                        stream.close();
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Error with file stream: " + e);
                }
            }

            try {
                log.write((now + ";" + bpm + "\n").getBytes());
            } catch (IOException e) {
                Log.i(TAG, "Error writing to log: " + e);
            }
        }

        // Clean up
        bandMask.release();

        // update BPM after one second
        if ((nowTC - lastSamplingTC) / Core.getTickFrequency() >= samplingFrequency/SEC_PER_MIN) {
            lastSamplingTC = Core.getTickCount();

            // average calculated BPMs since last time
            meanBpm = Core.mean(bpms).val[0];
            Core.sort(bpms, bpms, Core.SORT_EVERY_COLUMN);
            minBpm = bpms.get(0, 0)[0];
            maxBpm = bpms.get(bpms.rows()-1, 0)[0];

            listener.onHRMResult(new RPPGResult(now, meanBpm, minBpm, maxBpm));

            bpms.release();
            bpms = new MatOfDouble();
        }
    }

    /* Filter algorithms */

    /**
     * Remove noise from re-detecting face
     * @param _a the values
     * @param _b the result
     * @param jumps the jumps to be corrected
     */
    private void denoiseFilter2(Mat _a, Mat _b, Mat jumps) {

        Mat a = new Mat();
        _a.copyTo(a);

        // Calculate a mat with the first differences
        Mat diff = new Mat();
        Core.subtract(a.rowRange(1, a.rows()), a.rowRange(0, a.rows()-1), diff);

        //printMat("jumps", jumps);

        for (int i = 0; i < jumps.rows(); i++) {
            if (jumps.get(i, 0)[0] == JUMP) {
                Mat mask = Mat.zeros(a.col(0).size(), CvType.CV_8U);
                mask.rowRange(i, mask.rows()).setTo(ONE);
                Core.add(a, new Scalar(-diff.get(i-1, 0)[0]), a, mask);
            }
        }

        a.copyTo(_b);
    }

    /**
     * Detrending filter.
     * Based on a smoothness priors approach
     * @param _z Input signal
     * @param lambda Parameter
     * @param result result
     */
    private void detrendFilter(Mat _z, int lambda, Mat result) {
        Mat z = _z.total() == (int)_z.size().height ? _z : _z.t();
        if (z.total() < 3) {
            z.copyTo(result);
        } else {
            int t = (int)z.total();
            Mat i = Mat.eye(t, t, z.type());
            Mat d = new Mat(1, 3, z.type());
            d.put(0, 0, 1, -2, 1);
            Mat d2Aux = new Mat();
            Core.gemm(Mat.ones(t - 2, 1, z.type()), d, 1, new Mat(), 0, d2Aux, 0);
            Mat d2 = Mat.zeros(t - 2, t, z.type());
            for (int k = 0; k < 3; k++) {
                d2Aux.col(k).copyTo(d2.diag(k));
            }
            Mat temp = new Mat();
            Core.gemm(d2, d2, lambda * lambda, new Mat(), 0, temp, Core.GEMM_1_T);
            Core.add(i, temp, temp);
            Core.subtract(i, temp.inv(), temp);
            Core.gemm(temp, z, 1, new Mat(), 0, temp, 0);
            temp.copyTo(result);
            d2.release();
            d2Aux.release();
            d.release();
            i.release();
            temp.release();
        }
    }

    /**
     * Moving-average filter
     * @param _a Input signal
     * @param n n
     * @param s Kernel size
     * @param result result
     */
    private void meanFilter(Mat _a, int n, int s, Mat result) {
        Mat a = new Mat();
        _a.copyTo(a);
        for (int i = 0; i < n; i++) {
            Imgproc.blur(a, a, new Size(s, s));
        }
        a.copyTo(result);
        a.release();
    }

    /**
     * Create the Mat for a butterworth lowpass filter
     * @param filter the filter
     * @param cutoff the cutoff frequency
     * @param n the order
     */
    private void butterworth_lowpass_filter(Mat filter, double cutoff, int n) {
        if (cutoff <= 0) throw new AssertionError("cutoff cannot be zero!");
        Mat tmp = new Mat(filter.rows(), filter.cols(), CvType.CV_64FC1);
        for (int i = 0; i < filter.rows(); i++) {
            for (int j = 0; j < filter.cols(); j++) {
                tmp.put(i, j, 1. / (1 + Math.pow(i / cutoff, 2. * n)));
            }
        }
        ArrayList<Mat> toMerge = new ArrayList<>();
        toMerge.add(tmp);
        toMerge.add(tmp);
        Core.merge(toMerge, filter);
        tmp.release();
    }

    /**
     * Create the Mat for a butterworth bandpass filter
     * @param filter the filter
     * @param cutoff the cutoff frequency
     * @param cutin the cutoff frequency
     * @param n the order
     */
    private void butterworth_bandpass_filter(Mat filter, double cutoff, double cutin, int n) {
        Mat off = filter.clone();
        butterworth_lowpass_filter(off, cutoff, n);
        Mat in = filter.clone();
        butterworth_lowpass_filter(in, cutin, n);
        Core.subtract(in, off, filter);
        off.release();
        in.release();
    }

    /**
     * Apply a butterworth bandpass filter to data
     * - transfer to frequency domain
     * - apply the filter
     * - re-transfer to time domain
     * @param input data
     * @param output result
     * @param low the lower frequency
     * @param high the higher frequency
     */
    private void bandpassFilter(Mat input, Mat output, double low, double high) {

        if (input.total() < 3) {
            input.copyTo(output);
        } else {
            // Convert to frequency domain
            Mat frequencySpectrum = new Mat();
            timeToFrequency(input, frequencySpectrum, false);

            //Mat filter = new Mat(input.rows(), 1, CvType.CV_64FC2);
            Mat filter = frequencySpectrum.clone();
            butterworth_bandpass_filter(filter, low, high, 8);

            // Apply the filter
            Core.mulSpectrums(frequencySpectrum, filter, frequencySpectrum, 0);

            // Convert to time domain
            frequencyToTime(frequencySpectrum, output);

            // Clean up
            frequencySpectrum.release();
            filter.release();
        }
    }

    /* Frequency and time domain algorithms */

    /**
     * Algorithm selects selects the component which has the highest normalised spectral power
     * of a single frequency
     * @param input the signal
     * @param result the selected component
     */
    public void selectSignal(Mat input, MatOfDouble result) {
        final int total = input.rows();
        final int low = (int)(total * LOW_BPM/SEC_PER_MIN/fps);
        final int high = (int)(total * HIGH_BPM/SEC_PER_MIN/fps);
        double[] vals = new double[input.cols()];
        for (int i = 0; i < input.cols(); i++) {
            // Calculate spectral magnitudes
            Mat powerSpectrum = new Mat();
            timeToFrequency(input.col(i), powerSpectrum, true);
            // Grab index of max
            Mat bandMask = Mat.zeros(input.col(i).size(), CvType.CV_8U);
            bandMask.rowRange(Math.min(low, total), Math.min(high, total)).setTo(ONE);
            Core.MinMaxLocResult loc = Core.minMaxLoc(powerSpectrum, bandMask);
            double max = 0;
            double sum = 1;
            if (!powerSpectrum.empty()) {
                max = loc.maxVal;
                sum = Core.sumElems(powerSpectrum).val[0];
            }
            vals[i] = max/sum;
            powerSpectrum.release();
        }
        int idx = -1;
        double max = 0;
        for (int i = 0; i < vals.length; i++) {
            if (vals[i] > max) {
                max = vals[i];
                idx = i;
            }
        }
        if (idx == -1) {
            input.col(0).copyTo(result);
        } else {
            input.col(idx).copyTo(result);
        }
    }

    /**
     * Transform data from frequency domain (im) to time domain (re)
     * @param _input data in frequency domain
     * @param output data in time domain
     */
    private void frequencyToTime(Mat _input, Mat output) {

        Mat input = new Mat();
        _input.copyTo(input);

        // Inverse fourier transform
        Core.idft(input, input);

        // split into planes; plane 0 is output
        ArrayList<Mat> planes = new ArrayList<>();
        Core.split(input, planes);
        Core.normalize(planes.get(0), output, 0, 1, Core.NORM_MINMAX);

        // Clean up
        for (Mat plane: planes) {plane.release();}
    }

    /**
     * Transform data from time domain (re) to frequency domain (im)
     * @param _input data in time domain
     * @param output data in frequency domain
     * @param magnitude 1 = return re magnitude mat, 0 = return im spectrum mat
     */
    private void timeToFrequency(Mat _input, Mat output, boolean magnitude) {

        Mat input = new Mat();
        _input.copyTo(input);

        // Prepare planes
        ArrayList<Mat> planes = new ArrayList<>();
        planes.add(input);
        planes.add(Mat.zeros(input.size(), CvType.CV_64FC1));
        Mat powerSpectrum = new Mat(input.rows(), 2, CvType.CV_64FC1);
        Core.merge(planes, powerSpectrum);

        // Fourier transform
        Core.dft(powerSpectrum, powerSpectrum);

        if (magnitude) {
            Core.split(powerSpectrum, planes);
            Core.magnitude(planes.get(0), planes.get(1), planes.get(0));
            planes.get(0).copyTo(output);
        } else {
            powerSpectrum.copyTo(output);
        }

        // Clean up
        powerSpectrum.release();
        for (Mat plane : planes) {
            plane.release();
        }
    }

    /* Output */

    /**
     * Draw information on frame
     * @param frame frame to draw on
     */
    private void draw(Mat frame) {

        // Draw face shape
        Imgproc.ellipse(frame, new Point(box.x + box.width / 2.0, box.y + box.height / 2.0),
                new Size(box.width / 2.5, box.height / 2.0), 0, 0, 360, GREEN);
        Imgproc.circle(frame, new Point(leftEye.x + leftEye.width / 2.0, leftEye.y + leftEye.height / 2.0),
                (int) ((leftEye.width + leftEye.height) / 4.0), GREEN);
        Imgproc.circle(frame, new Point(rightEye.x + rightEye.width / 2.0, rightEye.y + rightEye.height / 2.0),
                (int) ((rightEye.width + rightEye.height) / 4.0), GREEN);

        if (drawMode && signal != null && !signal.empty() && powerSpectrum != null && !powerSpectrum.empty()) {

            // Display of signals with fixed dimensions
            double displayHeight = box.height/2.0;
            double displayWidth = box.width;

            // Draw signal
            Core.MinMaxLocResult loc = Core.minMaxLoc(signal);
            double heightMult = displayHeight/(loc.maxVal - loc.minVal);
            double widthMult = displayWidth/(signal.rows() - 1);
            double drawAreaTlX = box.tl().x + box.width;
            double drawAreaTlY = box.tl().y;
            Point p1 = new Point(drawAreaTlX, drawAreaTlY + (loc.maxVal - signal.get(0, 0)[0])*heightMult);
            Point p2;
            for (int i = 1; i < signal.rows(); i++) {
                p2 = new Point(drawAreaTlX + i * widthMult, drawAreaTlY + (loc.maxVal - signal.get(i, 0)[0])*heightMult);
                Imgproc.line(frame, p1, p2, RED, 2);
                p1 = p2.clone();
            }

            final int total = signal.rows();
            final int low = (int)(total * LOW_BPM / SEC_PER_MIN / fps);
            final int high = (int)(total * HIGH_BPM / SEC_PER_MIN / fps);
            Mat bandMask = Mat.zeros(signal.size(), CvType.CV_8U);
            bandMask.rowRange(Math.min(low, total), Math.min(high, total)).setTo(ONE);

            // Draw powerSpectrum
            loc = Core.minMaxLoc(powerSpectrum, bandMask);
            heightMult = displayHeight/(loc.maxVal - loc.minVal);
            widthMult = displayWidth/(high - low);
            drawAreaTlX = box.tl().x + box.width;
            drawAreaTlY = box.tl().y + box.height/2.0;
            p1 = new Point(drawAreaTlX, drawAreaTlY + (loc.maxVal - powerSpectrum.get(low, 0)[0])*heightMult);
            for (int i = low + 1; i <= high; i++) {
                p2 = new Point(drawAreaTlX + (i - low) * widthMult, drawAreaTlY + (loc.maxVal - powerSpectrum.get(i, 0)[0]) * heightMult);
                Imgproc.line(frame, p1, p2, RED, 2);
                p1 = p2.clone();
            }
        }

        // Draw BPM text
        if (valid) {
            Imgproc.putText(frame, "" + Math.round(meanBpm) + " bpm", new Point(box.tl().x, box.tl().y - 10), Core.FONT_HERSHEY_PLAIN, 3, RED, 2);
        }

        // Draw FPS text
        Imgproc.putText(frame, "" + Math.round(fps) + " fps", new Point(box.tl().x, box.br().y + 40), Core.FONT_HERSHEY_PLAIN, 3, GREEN, 2);
    }

    /**
     * Print information about a Mat
     * @param title the title
     * @param m the mat
     */
    private void printMatInfo(String title, Mat m) {
        Log.i(TAG, title + ": " + m.rows() + "x" + m.cols() +
                " channels=" + m.channels() +
                " depth=" + m.depth() +
                " isContinuous=" + (m.isContinuous() ? "true" : "false") +
                " isSub=" + (m.isSubmatrix() ? "true" : "false"));
    }

    /**
     * Print information and content of a Mat
     * @param title title of the mat
     * @param m the mat
     */
    private void printMat(String title, Mat m) {
        printMatInfo(title, m);

        for (int y = 0; y < m.rows(); y++) {
            System.out.print("[");
            for (int x = 0; x < m.cols(); x++) {
                double[] e = m.get(y, x);
                System.out.print("(" + e[0]);
                for (int c = 1; c < m.channels(); c++) {
                    System.out.print(", " + e[c]);
                }
                System.out.print(")");
            }
            System.out.println("]");
        }
        System.out.println("");
    }
}
