//
// Created by Philipp Rouast on 28/03/16.
//

#include "com_prouast_heartbeat_FFmpegEncoder.h"
#include "FFmpegEncoder.hpp"
#include <android/log.h>

#define LOG_TAG "Heartbeat::FFmpegEncoder"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

/*
 * Class:     com_prouast_heartbeat_FFmpegEncoder
 * Method:    _initialise
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_prouast_heartbeat_FFmpegEncoder__1initialise
        (JNIEnv *jenv, jclass) {
    LOGD("Java_com_prouast_heartbeat_FFmpegEncoder__1initialise enter");
    jlong result = 0;
    try {
        result = (jlong)new FFmpegEncoder();
    } catch (...) {
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_FFmpegEncoder__1initialise exit");
    return result;
}

/*
 * Class:     com_prouast_heartbeat_FFmpegEncoder
 * Method:    _openFile
 * Signature: (JLjava/lang/String;IIII)Z
 */
JNIEXPORT jboolean JNICALL Java_com_prouast_heartbeat_FFmpegEncoder__1openFile
        (JNIEnv *jenv, jclass, jlong self, jstring jfilename, jint jwidth, jint jheight, jint jbitrate, jint jframerate) {
    LOGD("Java_com_prouast_heartbeat_FFmpegEncoder__1openFile enter");
    jboolean result = false;
    const char *filename = (*jenv).GetStringUTFChars(jfilename, 0); // TODO correct? see wikipedia
    try {
        if (self) {
            result = ((FFmpegEncoder *)self)->OpenFile(filename, jwidth, jheight, jbitrate, jframerate);
        }
    } catch (...) {
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    jenv->ReleaseStringUTFChars(jfilename, filename);
    LOGD("Java_com_prouast_heartbeat_FFmpegEncoder__1openFile exit");
    return result;
}

/*
 * Class:     com_prouast_heartbeat_FFmpegEncoder
 * Method:    _writeFrame
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_FFmpegEncoder__1writeFrame
        (JNIEnv *jenv, jclass, jlong self, jlong jDataAddr, jlong jTime) {
    LOGD("Java_com_prouast_heartbeat_FFmpegEncoder__1writeFrame enter");
    try {
        if (self) {
            uint8_t *dataAddr = (uint8_t *)jDataAddr;
            ((FFmpegEncoder *)self)->WriteFrame(dataAddr, jTime);
            //((FFmpegEncoder *)self)->WriteFrame(*((cv::Mat*)jnativeObj), jTime);
        }
    } catch (...) {
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_FFmpegEncoder__1writeFrame exit");
}

/*
 * Class:     com_prouast_heartbeat_FFmpegEncoder
 * Method:    _closeFile
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_FFmpegEncoder__1closeFile
(JNIEnv *jenv, jclass, jlong self) {
    LOGD("Java_com_prouast_heartbeat_FFmpegEncoder__1closeFile enter");
    try {
        if (self) {
            ((FFmpegEncoder *)self)->WriteBufferedFrames();
            ((FFmpegEncoder *)self)->CloseFile();
        }
    } catch (...) {
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_FFmpegEncoder__1closeFile exit");
}