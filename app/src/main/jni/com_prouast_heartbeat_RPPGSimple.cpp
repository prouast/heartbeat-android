//
// Created by Philipp Rouast on 23/04/16.
//

#define LOG_TAG "Heartbeat::RPPGSimple"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

/*
 * Class:     com_prouast_heartbeat_RPPGSimple
 * Method:    _initialise
 * Signature: (Lcom/prouast/heartbeat/RPPGSimple/RPPGListener;IIIILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ)J
 */
JNIEXPORT jlong JNICALL Java_com_prouast_heartbeat_RPPGSimple__1initialise
    (JNIEnv *jenv, jclass, jobject jlistener, jint jwidth, jint jheight, jint jsamplingFrequency, jint jrescanInterval, jstring jlogFilePath, jstring jfaceClassifierFilename, jstring jleftEyeClassifierFilename, jstring jrightEyeClassifierFilename, jboolean jlog, jboolean jdraw) {
      LOGD("Java_com_prouast_heartbeat_RPPGSimple__1initialise enter");

      jlong result = 0;
      try {
          result = (jlong)new RPPGSimple(jlistener, jenv, jwidth, jheight, jsamplingFrequency, jrescanInterval, jlogFilePath, jfaceClassifierFilename, jleftEyeClassifierFilename, jrightEyeClassifierFilename, jlog, jdraw);
      } catch (...) {
          jclass je = jenv->FindClass("java/lang/Exception");
          jenv->ThrowNew(je, "Unknown exception in JNI code.");
      }
      LOGD("Java_com_prouast_heartbeat_RPPGSimple__1initialise exit");
      return result;
  }

/*
 * Class:     com_prouast_heartbeat_RPPGSimple
 * Method:    _processFrame
 * Signature: (JJJJ)V
 */
JNIEXPORT void JNICALL Java_com_prouast_heartbeat_RPPGSimple__1processFrame
  (JNIEnv *, jclass, jlong self, jlong jframeRGB, jlong jframeGray, jlong jtime) {
    LOGD("Java_com_prouast_heartbeat_RPPGSimple__1processFrame enter");
    try {
        ((RPPGSimple *)self)->processFrame(*((cv::Mat*)jframeRGB), *((cv::Mat*)jframeGray), jtime, jvm);
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
  (JNIEnv *, jclass, jlong self) {
    LOGD("Java_com_prouast_heartbeat_RPPGSimple__1exit enter");
    try {
        ((RPPGSimple *)self)->exit();
    } catch (...) {
        jclass je = jenv->FindClass("java/lang/Exception");
        jenv->ThrowNew(je, "Unknown exception in JNI code.");
    }
    LOGD("Java_com_prouast_heartbeat_RPPGSimple__1exit exit");
}