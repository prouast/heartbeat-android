//
// Created by Philipp Rouast on 23/04/16.
//

#include "com_prouast_heartbeat_RPPGSimple.h"
#include "RPPGSimple.hpp"
#include <android/log.h>

#define LOG_TAG "Heartbeat::RPPGSimple"
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
 * Class:     com_prouast_heartbeat_RPPGSimple
 * Method:    _initialise
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_prouast_heartbeat_RPPGSimple__1initialise
  (JNIEnv *jenv, jclass) {
  LOGD("Java_com_prouast_heartbeat_RPPGSimple__1initialise enter");
  jlong result = 0;
  try {
     result = (jlong)new RPPGSimple();
  } catch (...) {
      jclass je = jenv->FindClass("java/lang/Exception");
      jenv->ThrowNew(je, "Unknown exception in JNI code.");
  }
  LOGD("Java_com_prouast_heartbeat_RPPGSimple__1initialise exit");
  return result;
}

/*
 * Class:     com_prouast_heartbeat_RPPGSimple
 * Method:    _load
 * Signature: (JLcom/prouast/heartbeat/RPPGSimple/RPPGListener;IIDIILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPGSimple__1load
  (JNIEnv *jenv, jclass, jlong self, jobject jlistener, jint jwidth, jint jheight, jdouble jtimeBase, jint jsamplingFrequency, jint jrescanInterval,
   jstring jlogFilePath, jstring jfaceClassifierFilename, jstring jleftEyeClassifierFilename, jstring jrightEyeClassifierFilename, jboolean jlog, jboolean jdraw) {
  LOGD("Java_com_prouast_heartbeat_RPPGSimple__1load enter");
  bool log = jlog;
  bool draw = jdraw;
  std::string logFilePath, faceClassifierFilename, leftEyeClassifierFilename, rightEyeClassifierFilename;
  try {
      GetJStringContent(jenv, jlogFilePath, logFilePath);
      GetJStringContent(jenv, jfaceClassifierFilename, faceClassifierFilename);
      GetJStringContent(jenv, jleftEyeClassifierFilename, leftEyeClassifierFilename);
      GetJStringContent(jenv, jrightEyeClassifierFilename, rightEyeClassifierFilename);
      ((RPPGSimple *)self)->load(jlistener, jenv, jwidth, jheight, jtimeBase, jsamplingFrequency, jrescanInterval,
                                 logFilePath, faceClassifierFilename, leftEyeClassifierFilename, rightEyeClassifierFilename,
                                 log, draw);
  } catch (...) {
      jclass je = jenv->FindClass("java/lang/Exception");
      jenv->ThrowNew(je, "Unknown exception in JNI code.");
  }
  LOGD("Java_com_prouast_heartbeat_RPPGSimple__1load exit");
}

/*
 * Class:     com_prouast_heartbeat_RPPGSimple
 * Method:    _processFrame
 * Signature: (JJJJ)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPGSimple__1processFrame
  (JNIEnv *jenv, jclass, jlong self, jlong jframeRGB, jlong jframeGray, jlong jtime) {
  LOGD("Java_com_prouast_heartbeat_RPPGSimple__1processFrame enter");
  try {
      int64_t time = jtime;
      ((RPPGSimple *)self)->processFrame(*((cv::Mat*)jframeRGB), *((cv::Mat*)jframeGray), time);
  } catch (...) {
      jclass je = jenv->FindClass("java/lang/Exception");
      jenv->ThrowNew(je, "Unknown exception in JNI code.");
  }
  LOGD("Java_com_prouast_heartbeat_RPPGSimple__1processFrame exit");
}

/*
 * Class:     com_prouast_heartbeat_RPPGSimple
 * Method:    _exit
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPGSimple__1exit
  (JNIEnv *jenv, jclass, jlong self) {
    LOGD("Java_com_prouast_heartbeat_RPPGSimple__1exit enter");
    try {
        ((RPPGSimple *)self)->exit(jenv);
    } catch (...) {
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_RPPGSimple__1exit exit");
}