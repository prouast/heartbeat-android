//
//  RPPGMobile.cpp
//  Heartbeat
//
//  Created by Philipp Rouast on 21/05/2016.
//  Copyright © 2016 Philipp Roüast. All rights reserved.
//

#include "RPPGMobile.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/video/video.hpp>
#include "opencv.hpp"

#include <android/log.h>

using namespace cv;
using namespace std;

#define LOW_BPM 42
#define HIGH_BPM 240
#define REL_MIN_FACE_SIZE 0.2
#define SIGNAL_SIZE 5
#define SEC_PER_MIN 60

#define MAX_CORNERS 10
#define QUALITY_LEVEL 0.01
#define MIN_DISTANCE 10

#define LOG_TAG "Heartbeat::RPPGMobile"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

bool RPPGMobile::load(jobject listener, JNIEnv *jenv,
                      const int width, const int height,
                      const double timeBase,
                      const int samplingFrequency, const int rescanInterval,
                      const string &logFileName,
                      const string &faceClassifierFilename,
                      const string &leftEyeClassifierFilename,
                      const string &rightEyeClassifierFilename,
                      const bool log, const bool draw) {
    
    this->minFaceSize = cv::Size(cv::min(width, height) * REL_MIN_FACE_SIZE, cv::min(width, height) * REL_MIN_FACE_SIZE);
    this->rescanInterval = rescanInterval;
    this->samplingFrequency = samplingFrequency;
    this->timeBase = timeBase;
    this->logMode = log;
    this->drawMode = draw;
    this->updateFlag = false;

    // Save reference to Java VM
    jenv->GetJavaVM(&jvm);

    // Save global reference to listener object
    this->listener = jenv->NewGlobalRef(listener);

    // Load classifiers
    faceClassifier.load(faceClassifierFilename);
    leftEyeClassifier.load(rightEyeClassifierFilename);
    rightEyeClassifier.load(leftEyeClassifierFilename);
    
    // Setting up logfilepath
    std::ostringstream path_1;
    path_1 << logFileName << "_mobile";
    this->logfilepath = path_1.str();
    
    // Logging bpm according to sampling frequency
    std::ostringstream path_2;
    path_2 << logfilepath << "_bpm.csv";
    logfile.open(path_2.str().c_str());
    logfile << "time;mean;min;max\n";
    
    // Logging bpm detailed
    std::ostringstream path_3;
    path_3 << logfilepath << "_bpmDetailed.csv";
    logfileDetailed.open(path_3.str().c_str());
    logfileDetailed << "time;bpm\n";
    
    return true;
}

void RPPGMobile::exit(JNIEnv *jenv) {
    jenv->DeleteGlobalRef(listener);
    listener = NULL;
    logfile.close();
    logfileDetailed.close();
}

void RPPGMobile::processFrame(cv::Mat &frameRGB, cv::Mat &frameGray, int64_t time) {

    // Set time
    this->time = time;
    
    if (!valid) {
        
        LOGD("Not valid, finding a new face");
        
        lastScanTime = time;
        detectFace(frameRGB, frameGray);
        
    } else if ((time - lastScanTime) * timeBase >= rescanInterval) {
        
        LOGD("Valid, but rescanning face");
        
        lastScanTime = time;
        detectFace(frameRGB, frameGray);
        updateFlag = true;
        
    } else {
        
        LOGD("Tracking face");
        
        trackFace(frameGray);
    }
    
    if (valid) {
        
        fps = getFps(t, timeBase);
        
        // Remove old values from buffer
        while (g.rows > fps * SIGNAL_SIZE) {
            push(r);
            push(g);
            push(b);
            push(t);
            push(jumps);
        }
        
        // Add new values to buffer
        Scalar means = mean(frameRGB, mask);
        r.push_back<double>(means(0));
        g.push_back<double>(means(1));
        b.push_back<double>(means(2));
        jumps.push_back<bool>(updateFlag ? true : false);
        t.push_back<long>(time);
        
        fps = getFps(t, timeBase);
        
        updateFlag = false;
        
        // If buffer is large enough, send off to estimation
        if (g.rows / fps >= SIGNAL_SIZE) {
            
            // Save raw signal
            g.copyTo(signal);
            
            // Apply filters
            extractSignal_den_detr_mean();
            //extractSignal();

            // PSD estimation
            estimateHeartrate();
        }
        
        draw(frameRGB);
    }

    updateFlag = false;
    
    frameGray.copyTo(lastFrameGray);
}

void RPPGMobile::detectFace(cv::Mat &frameRGB, cv::Mat &frameGray) {
    
    LOGD("Scanning for faces…");
    
    // Detect faces with Haar classifier
    std::vector<cv::Rect> boxes;
    faceClassifier.detectMultiScale(frameGray, boxes, 1.1, 2, CV_HAAR_SCALE_IMAGE, minFaceSize);
    
    if (boxes.size() > 0) {
        
        LOGD("Found a face");
        
        setNearestBox(boxes);
        detectEyes(frameRGB);
        detectCorners(frameGray);
        updateMask(frameGray);
        valid = true;
        
    } else {
        
        LOGD("Found no face");
        
        valid = false;
    }
}

void RPPGMobile::setNearestBox(std::vector<cv::Rect> boxes) {
    int index = 0;
    cv::Point p = box.tl() - boxes.at(0).tl();
    int min = p.x * p.x + p.y * p.y;
    for (int i = 1; i < boxes.size(); i++) {
        p = box.tl() - boxes.at(i).tl();
        int d = p.x * p.x + p.y * p.y;
        if (d < min) {
            min = d;
            index = i;
        }
    }
    box = boxes.at(index);
}

void RPPGMobile::detectEyes(cv::Mat &frameRGB) {
    
    Rect leftEyeROI = Rect(box.tl().x + box.width/16,
                           box.tl().y + box.height/4.5,
                           (box.width - 2*box.width/16)/2,
                           box.height/3.0);
    
    Rect rightEyeROI = Rect(box.tl().x + box.width/16 + (box.width - 2*box.width/16)/2,
                            box.tl().y + box.height/4.5,
                            (box.width - 2*box.width/16)/2,
                            box.height/3.0);
    
    Mat leftSub = frameRGB(leftEyeROI);
    Mat rightSub = frameRGB(rightEyeROI);
    
    // Detect eyes with Haar classifier
    std::vector<cv::Rect> eyeBoxesLeft;
    leftEyeClassifier.detectMultiScale(leftSub, eyeBoxesLeft, 1.1, 2, 0);
    std::vector<cv::Rect> eyeBoxesRight;
    rightEyeClassifier.detectMultiScale(rightSub, eyeBoxesRight, 1.1, 2, 0);
    
    if (eyeBoxesLeft.size() > 0) {
        Rect leftEye = eyeBoxesLeft.at(0);
        Point tl = Point(leftEyeROI.x + leftEye.x, leftEyeROI.y + leftEye.y);
        Point br = Point(leftEyeROI.x + leftEye.x + leftEye.width,
                         leftEyeROI.y + leftEye.y + leftEye.height);
        this->leftEye = Rect(tl, br);
    } else {
        LOGD("No left eye found");
    }
    
    if (eyeBoxesRight.size() > 0) {
        Rect rightEye = eyeBoxesRight.at(0);
        Point tl = Point(rightEyeROI.x + rightEye.x, rightEyeROI.y + rightEye.y);
        Point br = Point(rightEyeROI.x + rightEye.x + rightEye.width,
                         rightEyeROI.y + rightEye.y + rightEye.height);
        this->rightEye = Rect(tl, br);
    } else {
        LOGD("No right eye found");
    }
}

void RPPGMobile::detectCorners(cv::Mat &frameGray) {
    
    // Define tracking region
    cv::Mat trackingRegion = cv::Mat::zeros(frameGray.rows, frameGray.cols, CV_8UC1);
    Point points[1][4];
    points[0][0] = Point(box.tl().x + 0.25 * box.width,
                         box.tl().y + 0.21 * box.height);
    points[0][1] = Point(box.tl().x + 0.75 * box.width,
                         box.tl().y + 0.21 * box.height);
    points[0][2] = Point(box.tl().x + 0.30 * box.width,
                         box.tl().y + 0.79 * box.height);
    points[0][3] = Point(box.tl().x + 0.70 * box.width,
                         box.tl().y + 0.79 * box.height);
    const Point *pts[1] = {points[0]};
    int npts[] = {4};
    cv::fillPoly(trackingRegion, pts, npts, 1, cv::WHITE);
    
    // Apply corner detection
    goodFeaturesToTrack(frameGray,
                        corners,
                        MAX_CORNERS,
                        QUALITY_LEVEL,
                        MIN_DISTANCE,
                        trackingRegion,
                        3,
                        false,
                        0.04);
}

void RPPGMobile::trackFace(cv::Mat &frameGray) {
    
    // Make sure enough corners are available
    if (corners.size() < 0.5 * MAX_CORNERS) {
        detectCorners(frameGray);
    }

    Contour2f corners_1;
    Contour2f corners_0;
    std::vector<uchar> cornersFound_1;
    std::vector<uchar> cornersFound_0;
    cv::Mat err;

    // Track face features with Kanade-Lucas-Tomasi (KLT) algorithm
    cv::calcOpticalFlowPyrLK(lastFrameGray, frameGray, corners, corners_1, cornersFound_1, err);
    // Backtrack once to make it more robust
    cv::calcOpticalFlowPyrLK(frameGray, lastFrameGray, corners_1, corners_0, cornersFound_0, err);

    // Exclude no-good corners
    Contour2f corners_1v;
    Contour2f corners_0v;
    for (size_t j = 0; j < corners.size(); j++) {
        if (cornersFound_1[j] && cornersFound_0[j]
            && cv::norm(corners[j]-corners_0[j]) < 2) {
            corners_0v.push_back(corners_0[j]);
            corners_1v.push_back(corners_1[j]);
        } else {
            std::cout << "Mis!" << std::endl;
        }
    }

    if (corners_1v.size() > 3) {

        // Save updated features
        corners = corners_1v;

        // Estimate affine transform
        Mat transform = estimateRigidTransform(corners_0v, corners_1v, false);

        // Update box
        Contour2f boxCoords;
        boxCoords.push_back(box.tl());
        boxCoords.push_back(box.br());
        Contour2f transformedBoxCoords;
        cv::transform(boxCoords, transformedBoxCoords, transform);
        box = Rect(transformedBoxCoords[0], transformedBoxCoords[1]);

        // Update left eye
        Contour2f leftEyeCoords;
        leftEyeCoords.push_back(leftEye.tl());
        leftEyeCoords.push_back(leftEye.br());
        Contour2f transformedLeftEyeCoords;
        cv::transform(leftEyeCoords, transformedLeftEyeCoords, transform);
        leftEye = Rect(transformedLeftEyeCoords[0], transformedLeftEyeCoords[1]);

        // Update right eye
        Contour2f rightEyeCoords;
        rightEyeCoords.push_back(rightEye.tl());
        rightEyeCoords.push_back(rightEye.br());
        Contour2f transformedRightEyeCoords;
        cv::transform(rightEyeCoords, transformedRightEyeCoords, transform);
        rightEye = Rect(transformedRightEyeCoords[0], transformedRightEyeCoords[1]);

        // Update ROI
        Contour2f roiCoords;
        roiCoords.push_back(roi.tl());
        roiCoords.push_back(roi.br());
        Contour2f transformedRoiCoords;
        cv::transform(roiCoords, transformedRoiCoords, transform);
        roi = Rect(transformedRoiCoords[0], transformedRoiCoords[1]);

    } else {

        // Detect new corners and skip step
        detectCorners(frameGray);
    }
}

void RPPGMobile::updateMask(cv::Mat &frameGray) {
    
    LOGD("Update mask");

    // TODO define only with box? Could save myself detecting the eyes

    mask = cv::Mat::zeros(frameGray.rows, frameGray.cols, CV_8UC1);

    Point leftEyeCenter = Point(leftEye.tl().x + leftEye.width/2, leftEye.tl().y + leftEye.height/2);
    Point rightEyeCenter = Point(rightEye.tl().x + rightEye.width/2, rightEye.tl().y + rightEye.height/2);
    double d = (rightEyeCenter.x - leftEyeCenter.x)/4.0;

    this->roi = Rect(leftEyeCenter.x + 0.5 * d, leftEyeCenter.y - 2.5 * d, 3 * d, 1.5 * d);

    mask.setTo(BLACK);
    rectangle(mask, roi, WHITE, FILLED);

    /*

    mask = cv::Mat::zeros(frameGray.rows, frameGray.cols, CV_8UC1);
    mask.setTo(BLACK); // TODO needed?
    ellipse(mask,
            Point(box.tl().x + box.width/2.0, box.tl().y + box.height/2.0),
            Size(box.width/2.5, box.height/2.0),
            0, 0, 360, WHITE, FILLED);
    circle(mask,
           Point(leftEye.tl().x + leftEye.width/2.0, leftEye.tl().y + leftEye.height/2.0),
           (leftEye.width + leftEye.height)/4.0, BLACK, FILLED);
    circle(mask,
           Point(rightEye.tl().x + rightEye.width/2.0, rightEye.tl().y + rightEye.height/2.0),
           (rightEye.width + rightEye.height)/4.0, BLACK, FILLED);

    */
}

void RPPGMobile::extractSignal() {

    // Denoise signals
    Mat r_den = Mat(r.rows, r.rows, CV_32F);
    Mat g_den = Mat(g.rows, g.rows, CV_32F);
    Mat b_den = Mat(b.rows, b.rows, CV_32F);
    denoise(r, r_den, jumps);
    denoise(g, g_den, jumps);
    denoise(b, b_den, jumps);

    // Normalize raw signals
    Mat r_n = Mat(r_den.rows, r_den.cols, CV_32F);
    Mat g_n = Mat(g_den.rows, g_den.cols, CV_32F);
    Mat b_n = Mat(b_den.rows, b_den.cols, CV_32F);
    normalization(r_den, r_n);
    normalization(g_den, g_n);
    normalization(b_den, b_n);

    // Calculate X_s signal
    Mat x_s = Mat(r.rows, r.cols, CV_32F);
    addWeighted(r_n, 3, g_n, -2, 0, x_s);

    // Calculate Y_s signal
    Mat y_s = Mat(r.rows, r.cols, CV_32F);
    addWeighted(r_n, 1.5, g_n, 1, 0, y_s);
    addWeighted(y_s, 1, b_n, -1.5, 0, y_s);

    const int total = signal.rows;
    const int low = (int)(total * LOW_BPM / SEC_PER_MIN / fps);
    const int high = (int)(total * HIGH_BPM / SEC_PER_MIN / fps);

    // Bandpass
    Mat x_f = Mat(r.rows, r.cols, CV_32F);
    bandpass(x_s, x_f, low, high);
    Mat y_f = Mat(r.rows, r.cols, CV_32F);
    bandpass(y_s, y_f, low, high);

    // Calculate alpha
    Scalar mean_x_f;
    Scalar stddev_x_f;
    meanStdDev(x_f, mean_x_f, stddev_x_f);
    Scalar mean_y_f;
    Scalar stddev_y_f;
    meanStdDev(y_f, mean_y_f, stddev_y_f);
    double alpha = stddev_x_f.val[0]/stddev_y_f.val[0];

    // Calculate signal
    addWeighted(x_f, 1, y_f, -alpha, 0, signal);

    // Logging
    if (logMode) {
        std::ofstream log;
        std::ostringstream filepath;
        filepath << logfilepath << "_signal_" << time << ".csv";
        log.open(filepath.str().c_str());
        log << "r;g;b;r_den;g_den;b_den;x_s;y_s;x_f;y_f;signal\n";
        for (int i = 0; i < g.rows; i++) {
            log << r.at<double>(i, 0) << ";";
            log << g.at<double>(i, 0) << ";";
            log << b.at<double>(i, 0) << ";";
            log << r_den.at<double>(i, 0) << ";";
            log << g_den.at<double>(i, 0) << ";";
            log << b_den.at<double>(i, 0) << ";";
            log << x_s.at<double>(i, 0) << ";";
            log << y_s.at<double>(i, 0) << ";";
            log << x_f.at<float>(i, 0) << ";";
            log << y_f.at<float>(i, 0) << ";";
            log << signal.at<double>(i, 0) << "\n";
        }
        log.close();
    }
}

void RPPGMobile::extractSignal_den_detr_mean() {

    LOGD("prah");

    // Denoise
    Mat signalDenoised;
    denoise(signal, signalDenoised, jumps);
    
    // Detrend
    Mat signalDetrended;
    detrend(signalDenoised, signalDetrended, fps);
    
    // Moving average
    Mat signalMeaned;
    movingAverage(signalDetrended, signalMeaned, 3, fps/3);
    signalMeaned.copyTo(signal);
    
    // Logging
    if (logMode) {
        std::ofstream log;
        std::ostringstream filepath;
        filepath << logfilepath << "_signal_" << time << ".csv";
        log.open(filepath.str().c_str());
        log << "g;g_den;g_detr;g_avg\n";
        for (int i = 0; i < g.rows; i++) {
            log << g.at<double>(i, 0) << ";";
            log << signalDenoised.at<double>(i, 0) << ";";
            log << signalDetrended.at<double>(i, 0) << ";";
            log << signalMeaned.at<double>(i, 0) << "\n";
        }
        log.close();
    }
}

void RPPGMobile::estimateHeartrate() {
    
    powerSpectrum = cv::Mat(signal.size(), CV_32F);
    timeToFrequency(signal, powerSpectrum, true);
    
    // band mask
    const int total = signal.rows;
    const int low = (int)(total * LOW_BPM / SEC_PER_MIN / fps);
    const int high = (int)(total * HIGH_BPM / SEC_PER_MIN / fps);
    Mat bandMask = Mat::zeros(signal.size(), CV_8U);
    bandMask.rowRange(min(low, total), min(high, total)).setTo(ONE);
    
    if (!powerSpectrum.empty()) {
        
        // grab index of max power spectrum
        double min, max;
        Point pmin, pmax;
        minMaxLoc(powerSpectrum, &min, &max, &pmin, &pmax, bandMask);
        
        // calculate BPM
        double bpm = pmax.y * fps / total * SEC_PER_MIN;
        bpms.push_back(bpm);
        
        LOGD("FPS=%f Vals=%d Peak=%d BPM=%f", fps, powerSpectrum.rows, pmax.y, bpm);
        
        // Logging
        if (logMode) {
            std::ofstream log;
            std::ostringstream filepath;
            filepath << logfilepath << "_estimation_" << time << ".csv";
            log.open(filepath.str().c_str());
            log << "i;powerSpectrum\n";
            for (int i = 0; i < powerSpectrum.rows; i++) {
                if (low <= i && i <= high) {
                    log << i << ";";
                    log << powerSpectrum.at<float>(i, 0) << "\n";
                }
            }
            log.close();
        }
        
        logfileDetailed << time << ";";
        logfileDetailed << bpm << "\n";
    }
    
    if ((time - lastSamplingTime) * timeBase >= samplingFrequency) {
        lastSamplingTime = time;
        
        cv::sort(bpms, bpms, SORT_EVERY_COLUMN);
        
        // average calculated BPMs since last sampling time
        meanBpm = mean(bpms)(0);
        minBpm = bpms.at<double>(0, 0);
        maxBpm = bpms.at<double>(bpms.rows-1, 0);
        
        callback(now, meanBpm, minBpm, maxBpm);
        
        // Logging
        logfile << time << ";";
        logfile << meanBpm << ";";
        logfile << minBpm << ";";
        logfile << maxBpm << "\n";
        
        bpms.pop_back(bpms.rows);
    }
}

void RPPGMobile::callback(int64_t now, double meanBpm, double minBpm, double maxBpm) {

    JNIEnv *jenv;
    int stat = jvm->GetEnv((void **)&jenv, JNI_VERSION_1_6);

    if (stat == JNI_EDETACHED) {
        LOGD("GetEnv: not attached");
        if (jvm->AttachCurrentThread(&jenv, NULL) != 0) {
            LOGD("GetEnv: Failed to attach");
        } else {
            LOGD("GetEnv: Attached to %d", jenv);
        }
    } else if (stat == JNI_OK) {
        //
    } else if (stat == JNI_EVERSION) {
        LOGD("GetEnv: version not supported");
    }

    // Return object

    // Get Return object class reference
    jclass returnObjectClassRef = jenv->FindClass("com/prouast/heartbeat/RPPGResult");

    // Get Return object constructor method
    jmethodID constructorMethodID = jenv->GetMethodID(returnObjectClassRef, "<init>", "(JDDD)V");

    // Create Info class
    jobject returnObject = jenv->NewObject(returnObjectClassRef, constructorMethodID, now, meanBpm, minBpm, maxBpm);

    // Listener

    // Get the Listener class reference
    jclass listenerClassRef = jenv->GetObjectClass(listener);

    // Use Listener class reference to load the eventOccurred method
    jmethodID listenerEventOccuredMethodID = jenv->GetMethodID(listenerClassRef, "onRPPGResult", "(Lcom/prouast/heartbeat/RPPGResult;)V");

    // Invoke listener eventOccurred
    jenv->CallVoidMethod(listener, listenerEventOccuredMethodID, returnObject);

    // Cleanup
    jenv->DeleteLocalRef(returnObject);
}

void RPPGMobile::draw(cv::Mat &frameRGB) {

    // Draw roi
    rectangle(frameRGB, roi, cv::GREEN);

    // Draw face shape
    ellipse(frameRGB,
            Point(box.tl().x + box.width / 2.0, box.tl().y + box.height / 2.0),
            Size(box.width / 2.5, box.height / 2.0),
            0, 0, 360, cv::GREEN);
    circle(frameRGB,
           Point(leftEye.tl().x + leftEye.width / 2.0, leftEye.tl().y + leftEye.height / 2.0),
           (leftEye.width + leftEye.height) / 4.0,
           cv::GREEN);
    circle(frameRGB,
           Point(rightEye.tl().x + rightEye.width / 2.0, rightEye.tl().y + rightEye.height / 2.0),
           (rightEye.width + rightEye.height) / 4.0,
           cv::GREEN);
    
    // Draw signal
    if (!signal.empty() && !powerSpectrum.empty()) {
        
        // Display of signals with fixed dimensions
        double displayHeight = box.height/2.0;
        double displayWidth = box.width*0.8;
        
        // Draw signal
        double vmin, vmax;
        Point pmin, pmax;
        minMaxLoc(signal, &vmin, &vmax, &pmin, &pmax);
        double heightMult = displayHeight/(vmax - vmin);
        double widthMult = displayWidth/(signal.rows - 1);
        double drawAreaTlX = box.tl().x + box.width;
        double drawAreaTlY = box.tl().y;
        Point p1(drawAreaTlX, drawAreaTlY + (vmax - signal.at<double>(0, 0))*heightMult);
        Point p2;
        for (int i = 1; i < signal.rows; i++) {
            p2 = Point(drawAreaTlX + i * widthMult, drawAreaTlY + (vmax - signal.at<double>(i, 0))*heightMult);
            line(frameRGB, p1, p2, RED, 2);
            p1 = p2;
        }
        
        // Draw powerSpectrum
        const int total = signal.rows;
        const int low = (int)(total * LOW_BPM / SEC_PER_MIN / fps);
        const int high = (int)(total * HIGH_BPM / SEC_PER_MIN / fps);
        Mat bandMask = Mat::zeros(signal.size(), CV_8U);
        bandMask.rowRange(min(low, total), min(high, total)).setTo(ONE);
        minMaxLoc(powerSpectrum, &vmin, &vmax, &pmin, &pmax, bandMask);
        heightMult = displayHeight/(vmax - vmin);
        widthMult = displayWidth/(high - low);
        drawAreaTlX = box.tl().x + box.width;
        drawAreaTlY = box.tl().y + box.height/2.0;
        p1 = Point(drawAreaTlX, drawAreaTlY + (vmax - powerSpectrum.at<double>(low, 0))*heightMult);
        for (int i = low + 1; i <= high; i++) {
            p2 = Point(drawAreaTlX + (i - low) * widthMult, drawAreaTlY + (vmax - powerSpectrum.at<double>(i, 0)) * heightMult);
            line(frameRGB, p1, p2, RED, 2);
            p1 = p2;
        }
    }
    
    std::stringstream ss;
    
    // Draw BPM text
    if (valid) {
        ss.precision(3);
        ss << meanBpm << " bpm";
        putText(frameRGB, ss.str(), Point(box.tl().x, box.tl().y - 10), cv::FONT_HERSHEY_PLAIN, 2, cv::RED, 2);
    }
    
    // Draw FPS text
    ss.str("");
    ss << fps << " fps";
    putText(frameRGB, ss.str(), Point(box.tl().x, box.br().y + 40), cv::FONT_HERSHEY_PLAIN, 2, cv::GREEN, 2);
    
    // Draw corners
    /// Draw corners detected
    int r = 4;
    for (int i = 0; i < corners.size(); i++) {
        circle(frameRGB, corners[i], r, cv::WHITE, -1, 8, 0);
    }
}
