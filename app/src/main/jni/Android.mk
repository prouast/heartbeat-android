LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := FFmpegEncoder
LOCAL_LDLIBS := -llog -ljnigraphics -lz -landroid
LOCAL_C_INCLUDES := /Users/prouast/Library/Android/sdk/ndk-bundle/sources/ffmpeg-2.2.3/android/arm/include
LOCAL_SRC_FILES := FFmpegEncoder.cpp com_prouast_heartbeat_FFmpegEncoder.cpp
LOCAL_SHARED_LIBRARIES := libavformat-55 libavcodec-55 libavutil-52 libswscale-2
include $(BUILD_SHARED_LIBRARY)

include /Users/prouast/Library/Android/sdk/ndk-bundle/sources/ffmpeg-2.2.3/android/arm/Android.mk
include /Users/prouast/Library/Android/sdk/ndk-bundle/sources/opencv/Android.mk