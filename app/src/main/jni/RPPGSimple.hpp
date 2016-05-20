//
//  RPPGSimple.hpp
//  Heartbeat
//
//  Created by Philipp Rouast on 29/02/2016.
//  Copyright © 2016 Philipp Roüast. All rights reserved.
//

#ifndef RPPGSimple_hpp
#define RPPGSimple_hpp

#include <string>
#include <stdio.h>
#include <fstream>
#include <opencv2/objdetect/objdetect.hpp>

#include <jni.h>

class RPPGSimple {
    
public:

    // Constructor
    RPPGSimple() : jvm(NULL) {;}

    void load(//jobject listener, JNIEnv *jenv,
               int width);//, int height,
               //double timeBase,
               //int samplingFrequency, int rescanInterval,
               //const char *logFileName,
               //const char *faceClassifierFilename,
               //const char *leftEyeClassifierFilename,
               //onst char *rightEyeClassifierFilename,
               //bool log, bool draw);

    void exit();
    void processFrame(cv::Mat &frameRGB, cv::Mat &frameGray, long time);
    
private:

    void detectFace(cv::Mat &frameRGB, cv::Mat &frameGray);
    void setNearestBox(std::vector<cv::Rect> boxes);
    void detectEyes(cv::Mat &frameRGB);
    void updateMask();
    void extractSignal_den_detr_mean();
    void extractSignal_den_band();
    void estimateHeartrate();
    void callback(long now, double meanBpm, double minBpm, double maxBpm);
    void draw(cv::Mat &frameRGB);

    // The JavaVM
    JavaVM *jvm;

    // The listener
    jobject listener;

    // The classifiers
    cv::CascadeClassifier faceClassifier;
    cv::CascadeClassifier leftEyeClassifier;
    cv::CascadeClassifier rightEyeClassifier;

    // Settings
    cv::Size minFaceSize;
    double rescanInterval;
    int samplingFrequency;
    double timeBase;
    bool logMode;
    bool drawMode;

    // State variables
    long time;
    double fps;
    double lastSamplingTime;
    double lastScanTime;
    long now;
    bool valid;
    bool updateFlag;

    // Mask
    cv::Rect box;
    cv::Rect rightEye;
    cv::Rect leftEye;
    cv::Mat mask;

    // Signal
    cv::Mat1d g;
    cv::Mat1d t;
    cv::Mat1d jumps;
    cv::Mat1d signal;
    cv::Mat1d bpms;
    cv::Mat1d powerSpectrum;
    double meanBpm;
    double minBpm;
    double maxBpm;

    // Logfiles
    std::ofstream logfile;
    std::ofstream logfileDetailed;
    std::string logfilepath;
};

#endif /* RPPGSimple_hpp */
