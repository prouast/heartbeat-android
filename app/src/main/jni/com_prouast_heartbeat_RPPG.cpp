//
// Created by Philipp Rouast on 8/07/16.
//

#include "com_prouast_heartbeat_RPPG.h"
#include <android/log.h>
#include "RPPG.hpp"

#define LOG_TAG "Heartbeat::RPPG"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

void GetJStringContent(JNIEnv *AEnv, jstring AStr, std::string &ARes) {
  if (!AStr) {
    ARes.clear();
    return;
  }
  const char *s = AEnv->GetStringUTFChars(AStr,NULL);
  ARes=s;
  AEnv->ReleaseStringUTFChars(AStr,s);
}

/*
 * Class:     com_prouast_heartbeat_RPPG
 * Method:    _initialise
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_prouast_heartbeat_RPPG__1initialise
(JNIEnv *jenv, jclass) {
    LOGD("Java_com_prouast_heartbeat_RPPG__1initialise enter");
    jlong result = 0;
    try {
        result = (jlong)new RPPG();
    } catch (...) {
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_RPPG__1initialise exit");
    return result;
}

/*
 * Class:     com_prouast_heartbeat_RPPG
 * Method:    _load
 * Signature: (JLcom/prouast/heartbeat/RPPG/RPPGListener;IIIDIDDIILjava/lang/String;Ljava/lang/String;ZZ)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPG__1load
(JNIEnv *jenv, jclass, jlong self, jobject jlistener, jint jalgorithm, jint jwidth, jint jheight,
jdouble jtimeBase, jint jdownsample, jdouble jsamplingFrequency, jdouble jrescanFrequency,
jint jminSignalSize, jint jmaxSignalSize, jstring jlogPath, jstring jclassifierPath,
jboolean jlog, jboolean jgui) {
    LOGD("Java_com_prouast_heartbeat_RPPG__1load enter");
    bool log = jlog;
    bool gui = jgui;
    std::string logPath, classifierPath;
    try {
        GetJStringContent(jenv, jlogPath, logPath);
        GetJStringContent(jenv, jclassifierPath, classifierPath);
        ((RPPG *)self)->load(jlistener, jenv, jalgorithm, jwidth, jheight, jtimeBase, jdownsample,
                                   jsamplingFrequency, jrescanFrequency, jminSignalSize, jmaxSignalSize,
                                   logPath, classifierPath, log, gui);
    } catch (...) {
      jclass je = jenv->FindClass("java/lang/Exception");
      jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_RPPG__1load exit");
}

/*
 * Class:     com_prouast_heartbeat_RPPG
 * Method:    _processFrame
 * Signature: (JJJJ)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPG__1processFrame
(JNIEnv *jenv, jclass, jlong self, jlong jframeRGB, jlong jframeGray, jlong jtime) {
    LOGD("Java_com_prouast_heartbeat_RPPG__1processFrame enter");
    try {
        int64_t time = jtime;
        ((RPPG *)self)->processFrame(*((cv::Mat*)jframeRGB), *((cv::Mat*)jframeGray), time);
    } catch (...) {
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_RPPG__1processFrame exit");
}

/*
 * Class:     com_prouast_heartbeat_RPPG
 * Method:    _exit
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPG__1exit
(JNIEnv *jenv, jclass, jlong self) {
    LOGD("Java_com_prouast_heartbeat_RPPG__1exit enter");
    try {
        ((RPPG *)self)->exit(jenv);
    } catch (...) {
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_RPPG__1exit exit");
}