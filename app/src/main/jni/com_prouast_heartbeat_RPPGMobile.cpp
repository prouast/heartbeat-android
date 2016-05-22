//
// Created by Philipp Rouast on 22/05/16.
//

#include "com_prouast_heartbeat_RPPGMobile.h"
#include "RPPGMobile.hpp"
#include <android/log.h>

#define LOG_TAG "Heartbeat::RPPGMobile"
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
 * Class:     com_prouast_heartbeat_RPPGMobile
 * Method:    _initialise
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_prouast_heartbeat_RPPGMobile__1initialise
  (JNIEnv *jenv, jclass) {
    LOGD("Java_com_prouast_heartbeat_RPPGMobile__1initialise enter");
    jlong result = 0;
    try {
     result = (jlong)new RPPGMobile();
    } catch (...) {
      jclass je = jenv->FindClass("java/lang/Exception");
      jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_RPPGMobile__1initialise exit");
    return result;
  }

/*
 * Class:     com_prouast_heartbeat_RPPGMobile
 * Method:    _load
 * Signature: (JLcom/prouast/heartbeat/RPPGMobile/RPPGListener;IIDIILjava/lang/String;Ljava/lang/String;ZZ)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPGMobile__1load
  (JNIEnv *jenv, jclass, jlong self, jobject jlistener, jint jwidth, jint jheight, jdouble jtimeBase, jint jsamplingFrequency, jint jrescanInterval,
      jstring jlogFilePath, jstring jclassifierFilename, jboolean jlog, jboolean jdraw) {
    LOGD("Java_com_prouast_heartbeat_RPPGMobile__1load enter");
    bool log = jlog;
    bool draw = jdraw;
    std::string logFilePath, classifierFilename;
    try {
      GetJStringContent(jenv, jlogFilePath, logFilePath);
      GetJStringContent(jenv, jclassifierFilename, classifierFilename);
      ((RPPGMobile *)self)->load(jlistener, jenv, jwidth, jheight, jtimeBase, jsamplingFrequency, jrescanInterval,
                                 logFilePath, classifierFilename, log, draw);
    } catch (...) {
      jclass je = jenv->FindClass("java/lang/Exception");
      jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_RPPGMobile__1load exit");
  }

/*
 * Class:     com_prouast_heartbeat_RPPGMobile
 * Method:    _processFrame
 * Signature: (JJJJ)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPGMobile__1processFrame
  (JNIEnv *jenv, jclass, jlong self, jlong jframeRGB, jlong jframeGray, jlong jtime) {
    LOGD("Java_com_prouast_heartbeat_RPPGMobile__1processFrame enter");
    try {
      int64_t time = jtime;
      ((RPPGMobile *)self)->processFrame(*((cv::Mat*)jframeRGB), *((cv::Mat*)jframeGray), time);
    } catch (...) {
      jclass je = jenv->FindClass("java/lang/Exception");
      jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_RPPGMobile__1processFrame exit");
  }

/*
 * Class:     com_prouast_heartbeat_RPPGMobile
 * Method:    _exit
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPGMobile__1exit
  (JNIEnv *jenv, jclass, jlong self) {
    LOGD("Java_com_prouast_heartbeat_RPPGMobile__1exit enter");
    try {
        ((RPPGMobile *)self)->exit(jenv);
    } catch (...) {
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_RPPGMobile__1exit exit");
  }