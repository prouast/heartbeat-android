#APP_BUILD_SCRIPT := /Users/prouast/Library/Android/sdk/ndk-bundle/sources/ffmpeg-2.2.3/android/arm/Android.mk
APP_ABI := armeabi-v7a
APP_PLATFORM := android-14
# APP_STL := stlport_static
APP_CFLAGS += -fexceptions
APP_STL := gnustl_shared

# Enable c++11 extentions in source code
APP_CPPFLAGS += -std=c++11