#!/bin/bash
NDK=/Users/prouast/Library/Android/sdk/ndk-bundle
SYSROOT=$NDK/platforms/android-9/arch-arm/
TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64

CPU=arm
PREFIX=$(pwd)/android/$CPU
ADDI_CFLAGS="-marm -I/usr/local/include"
ADDI_LDFLAGS="-L/usr/local/lib"

function build_one
{
./configure --prefix=$PREFIX --enable-shared --disable-ffmpeg --disable-ffplay --disable-ffprobe --disable-ffserver --disable-avdevice --disable-doc --disable-symver --enable-gpl --enable-libx264 --enable-protocol=concat --enable-memalign-hack --cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- --target-os=linux --arch=arm --enable-cross-compile --sysroot=$SYSROOT --enable-protocol=file --extra-cflags="-Os -fpic $ADDI_CFLAGS" --extra-ldflags="$ADDI_LDFLAGS"
make clean
make
}

build_one

