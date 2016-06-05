LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := FFmpegEncoder
LOCAL_LDLIBS := -llog -ljnigraphics -lz -landroid
LOCAL_C_INCLUDES += /Users/prouast/Library/Android/sdk/ndk-bundle/sources/ffmpeg-2.2.3/android/arm/include
LOCAL_SRC_FILES := FFmpegEncoder.cpp com_prouast_heartbeat_FFmpegEncoder.cpp
LOCAL_SHARED_LIBRARIES := libavformat-55 libavcodec-55 libavutil-52 libswscale-2
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
OPENCV_INSTALL_MODULES:=on
include /Users/prouast/Library/Android/sdk/ndk-bundle/sources/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk
LOCAL_MODULE := RPPGMobile
LOCAL_SRC_FILES := RPPGMobile.cpp opencv.cpp com_prouast_heartbeat_RPPGMobile.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_LDLIBS := -llog -ldl
include $(BUILD_SHARED_LIBRARY)

include /Users/prouast/Library/Android/sdk/ndk-bundle/sources/ffmpeg-2.2.3/android/arm/Android.mk