/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_prouast_heartbeat_RPPG */

#ifndef _Included_com_prouast_heartbeat_RPPG
#define _Included_com_prouast_heartbeat_RPPG
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_prouast_heartbeat_RPPG
 * Method:    _initialise
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_prouast_heartbeat_RPPG__1initialise
  (JNIEnv *, jclass);

/*
 * Class:     com_prouast_heartbeat_RPPG
 * Method:    _load
 * Signature: (JLcom/prouast/heartbeat/RPPG/RPPGListener;IIIDIDDIILjava/lang/String;Ljava/lang/String;ZZ)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPG__1load
  (JNIEnv *, jclass, jlong, jobject, jint, jint, jint, jdouble, jint, jdouble, jdouble, jint, jint, jstring, jstring, jboolean, jboolean);

/*
 * Class:     com_prouast_heartbeat_RPPG
 * Method:    _processFrame
 * Signature: (JJJJ)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPG__1processFrame
  (JNIEnv *, jclass, jlong, jlong, jlong, jlong);

/*
 * Class:     com_prouast_heartbeat_RPPG
 * Method:    _exit
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPG__1exit
  (JNIEnv *, jclass, jlong);

#ifdef __cplusplus
}
#endif
#endif