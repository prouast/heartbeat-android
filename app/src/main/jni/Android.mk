LOCAL_PATH := $(call my-dir)

# Path to ffmpeg
# Has to be built and Android.mk written (libavformat, libavcodec, libavutil, libswscale needed)
# See https://enoent.fr/blog/2014/06/20/compile-ffmpeg-for-android/
FFMPEG_PATH := ../../../../sources/ffmpeg-2.2.3/android/arm

# Path to OpenCV
# http://sourceforge.net/projects/opencvlibrary/files/opencv-android/3.1.0/OpenCV-3.1.0-android-sdk.zip/download
OPENCV_PATH := ../../../../sources/OpenCV-android-sdk

include $(CLEAR_VARS)
LOCAL_MODULE := FFmpegEncoder
LOCAL_LDLIBS := -llog -ljnigraphics -lz -landroid
LOCAL_C_INCLUDES += $(FFMPEG_PATH)/include
LOCAL_SRC_FILES := FFmpegEncoder.cpp com_prouast_heartbeat_FFmpegEncoder.cpp
LOCAL_SHARED_LIBRARIES := libavformat-55 libavcodec-55 libavutil-52 libswscale-2
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
OPENCV_INSTALL_MODULES:=on
include $(OPENCV_PATH)/sdk/native/jni/OpenCV.mk
LOCAL_MODULE := RPPG
LOCAL_SRC_FILES := RPPG.cpp opencv.cpp com_prouast_heartbeat_RPPG.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_LDLIBS := -llog -ldl
include $(BUILD_SHARED_LIBRARY)

include $(FFMPEG_PATH)/Android.mk