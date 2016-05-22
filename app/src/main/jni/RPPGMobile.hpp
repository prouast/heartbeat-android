//
//  RPPGMobile.hpp
//  Heartbeat
//
//  Created by Philipp Rouast on 21/05/2016.
//  Copyright © 2016 Philipp Roüast. All rights reserved.
//

#ifndef RPPGMobile_hpp
#define RPPGMobile_hpp

#include <string>
#include <stdio.h>
#include <fstream>
#include <opencv2/objdetect/objdetect.hpp>

#include <jni.h>

class RPPGMobile {
    
public:
    
    // Constructor
    RPPGMobile() {;}
    
    // Load Settings
    bool load(jobject listener, JNIEnv *jenv,                                   // Listener and environment for Java callback
              const int width, const int height,
              const double timeBase,
              const int samplingFrequency, const int rescanInterval,
              const std::string &logFileName,
              const std::string &faceClassifierFilename,
              const std::string &leftEyeClassifierFilename,
              const std::string &rightEyeClassifierFilename,
              const bool log, const bool draw);
    
    void processFrame(cv::Mat &frameRGB, cv::Mat &frameGray, int64_t time);
    
    void exit(JNIEnv *jenv);
    
    typedef std::vector<cv::Point> Contour;
    typedef std::vector<cv::Point2f> Contour2f;
    
private:
    
    void detectFace(cv::Mat &frameRGB, cv::Mat &frameGray);
    void setNearestBox(std::vector<cv::Rect> boxes);
    void detectEyes(cv::Mat &frameRGB);
    void detectCorners(cv::Mat &frameGray);
    void trackFace(cv::Mat &frameGray);
    void updateMask(cv::Mat &frameGray);
    void extractSignal_den_detr_mean();
    void extractSignal();
    void estimateHeartrate();
    void draw(cv::Mat &frameRGB);

    void callback(int64_t now, double meanBpm, double minBpm, double maxBpm);   // Callback to Java

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
    int64_t time;
    double fps;
    int64_t lastSamplingTime;
    int64_t lastScanTime;
    int64_t now;
    bool valid;
    bool updateFlag;
    
    // Tracking
    cv::Mat lastFrameGray;
    Contour2f corners;
    
    // Mask
    cv::Rect box;
    cv::Rect rightEye;
    cv::Rect leftEye;
    cv::Mat mask;
    cv::Rect roi;
    
    // Signal
    cv::Mat1d r;
    cv::Mat1d g;
    cv::Mat1d b;
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

#endif /* RPPGMobile_hpp */
