#!/bin/bash
# 以下路径需要修改成自己的NDK目录
TOOLCHAIN=/mnt/fileroot/zhao.yi/Tools/android-ndk-r20/toolchains/llvm/prebuilt/linux-x86_64
# 最低支持的android sdk版本
API=28


notExce(){

#removed files

libavformat/ac3dec.c 
libavformat/spdifdec.c 
libavformat/spdifenc.c 
libavcodec/ac3.c 
libavcodec/ac3dec_data.c 
libavcodec/ac3dec_fixed.c 
libavcodec/ac3dec_float.c 
libavcodec/ac3dsp.c 
libavcodec/ac3enc.c 
libavcodec/ac3enc_fixed.c 
libavcodec/ac3enc_float.c 
libavcodec/amrnbdec.c 
libavcodec/arm/ac3dsp_arm.S 
libavcodec/arm/ac3dsp_armv6.S 
libavcodec/arm/ac3dsp_init_arm.c 
libavcodec/arm/ac3dsp_neon.S 
libavcodec/arm/fmtconvert_init_arm.c 
libavcodec/arm/fmtconvert_neon.S 
libavcodec/arm/fmtconvert_vfp.S 
libavcodec/arm/synth_filter_init_arm.c 
libavcodec/arm/synth_filter_neon.S 
libavcodec/arm/synth_filter_vfp.S 
libavcodec/cavs.c 
libavcodec/cavsdata.c 
libavcodec/cavsdec.c 
libavcodec/cavsdsp.c 
libavcodec/dca.c 
libavcodec/dca_core.c 
libavcodec/dca_exss.c 
libavcodec/dca_lbr.c 
libavcodec/dca_xll.c 
libavcodec/dcaadpcm.c 
libavcodec/dcadata.c 
libavcodec/dcadct.c 
libavcodec/dcadec.c 
libavcodec/dcadsp.c 
libavcodec/dcaenc.c 
libavcodec/dcahuff.c 
libavcodec/eac3_data.c 
libavcodec/eac3enc.c 
libavcodec/flacenc.c 
libavcodec/fmtconvert.c 
libavcodec/mlpenc.c 
libavcodec/synth_filter.c 
libavcodec/utvideoenc.c 




libavformat/ac3dec.c 
libavformat/spdifdec.c 
libavformat/spdifenc.c 
libavcodec/aarch64/fmtconvert_init.c 
libavcodec/aarch64/fmtconvert_neon.S 
libavcodec/aarch64/synth_filter_init.c 
libavcodec/aarch64/synth_filter_neon.S 
libavcodec/ac3.c 
libavcodec/ac3dec_data.c 
libavcodec/ac3dec_fixed.c 
libavcodec/ac3dec_float.c 
libavcodec/ac3dsp.c 
libavcodec/ac3enc.c 
libavcodec/ac3enc_fixed.c 
libavcodec/ac3enc_float.c 
libavcodec/amrnbdec.c 
libavcodec/cavs.c 
libavcodec/cavsdata.c 
libavcodec/cavsdec.c 
libavcodec/cavsdsp.c 
libavcodec/dca.c 
libavcodec/dca_core.c 
libavcodec/dca_exss.c 
libavcodec/dca_lbr.c 
libavcodec/dca_xll.c 
libavcodec/dcaadpcm.c 
libavcodec/dcadata.c 
libavcodec/dcadct.c 
libavcodec/dcadec.c 
libavcodec/dcadsp.c 
libavcodec/dcaenc.c 
libavcodec/dcahuff.c 
libavcodec/eac3_data.c 
libavcodec/eac3enc.c 
libavcodec/flacenc.c 
libavcodec/fmtconvert.c 
libavcodec/mlpenc.c 
libavcodec/synth_filter.c 
libavcodec/utvideoenc.c 
}

function build_android
{
echo "Compiling FFmpeg for $CPU"
./configure \
 --prefix=$PREFIX \
 --disable-hwaccels \
 --disable-gpl \
 --disable-postproc \
 --enable-shared \
 --disable-jni \
 --disable-mediacodec \
 --enable-small \
 --disable-opencl \
 --disable-static \
 --disable-doc \
 --disable-programs \
 --disable-ffmpeg \
 --disable-ffplay \
 --disable-ffprobe \
 --disable-avdevice \
 --disable-symver \
 --cross-prefix=$CROSS_PREFIX \
 --target-os=android \
 --arch=$ARCH \
 --cpu=$CPU \
 --cc=$CC \
 --cxx=$CXX \
 --enable-cross-compile \
 --sysroot=$SYSROOT \
 --extra-cflags="-mno-stackrealign -Os -fpic $OPTIMIZE_CFLAGS -march=armv7-a -mfloat-abi=softfp -mfpu=neon" \
 --extra-ldflags="$ADDI_LDFLAGS" \
 --disable-parser='dca,mlp' \
 --disable-bsf=dca_core \
 --disable-zlib \
 --enable-pic \
 --enable-cross-compile \
 --disable-decoder='cavs,ac3_fixed,ac3,eac3,mlp,dca,amrnb,h264_mediacodec' \
 --disable-encoder='utvideo,flac,ac3,ac3_fixed,eac3,mlp,dca,truehd' \
 --disable-demuxer='ac3,eac3,dts,dtshd,spdif,truehd' \
 --disable-muxer='ac3,eac3,dts,dtshd,spdif' \
 --enable-armv5te --enable-armv6 --enable-armv6t2 --enable-neon --disable-inline-asm
 $ADDITIONAL_CONFIGURE_FLAG
//make clean
//make -j12
//make install
echo "The Compilation of FFmpeg for $CPU is completed"
}



function build_android_enableall
{
echo "Compiling FFmpeg for $CPU"
./configure \
 --prefix=$PREFIX \
 --disable-hwaccels \
 --disable-gpl \
 --disable-jni \
 --disable-postproc \
 --enable-shared \
 --disable-mediacodec \
 --enable-small \
 --disable-opencl \
 --disable-static \
 --disable-doc \
 --disable-programs \
 --disable-ffmpeg \
 --disable-ffplay \
 --disable-ffprobe \
 --disable-avdevice \
 --disable-symver \
 --cross-prefix=$CROSS_PREFIX \
 --target-os=android \
 --arch=$ARCH \
 --cpu=$CPU \
 --cc=$CC \
 --cxx=$CXX \
 --enable-cross-compile \
 --sysroot=$SYSROOT \
 --extra-cflags="-mno-stackrealign -Os -fpic $OPTIMIZE_CFLAGS -march=armv7-a -mfloat-abi=softfp -mfpu=neon" \
 --extra-ldflags="$ADDI_LDFLAGS" \
 --disable-parser='dca,mlp' \
 --disable-bsf=dca_core \
 --disable-zlib \
 --enable-pic \
 --enable-cross-compile \
 --enable-armv5te --enable-armv6 --enable-armv6t2 --enable-neon --disable-inline-asm
 $ADDITIONAL_CONFIGURE_FLAG
//make clean
//make -j12
//make install
echo "The Compilation of FFmpeg for $CPU is completed"
}


# 交叉编译工具目录,对应关系如下
# armv8a -> arm64 -> aarch64-linux-android-
# armv7a -> arm -> arm-linux-androideabi-
# x86 -> x86 -> i686-linux-android-
# x86_64 -> x86_64 -> x86_64-linux-android-


#armv8-a
make clean
ARCH=arm64
CPU=armv8-a
# r21版本的ndk中所有的编译器都在/toolchains/llvm/prebuilt/darwin-x86_64/目录下（clang）
CC=$TOOLCHAIN/bin/aarch64-linux-android$API-clang
CXX=$TOOLCHAIN/bin/aarch64-linux-android$API-clang++
# NDK头文件环境
SYSROOT=$TOOLCHAIN/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android-
# so输出路径
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-march=$CPU"
build_android_enableall
find . -name *.o -exec rm -rf {} \;
find . -name *.d -exec rm -rf {} \;
cp -rf config.h config.enableall.arm64.h
make -j12 > make.enableall.arm64.log


# CPU架构
#armv7-a
make clean
ARCH=arm
CPU=armv7-a
CC=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang
CXX=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang++
SYSROOT=$TOOLCHAIN/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU "
build_android_enableall
find . -name *.o -exec rm -rf {} \;
find . -name *.d -exec rm -rf {} \;
cp -rf config.h config.enableall.arm.h
make -j12 > make.enableall.arm.log


#armv8-a
make clean
ARCH=arm64
CPU=armv8-a
# r21版本的ndk中所有的编译器都在/toolchains/llvm/prebuilt/darwin-x86_64/目录下（clang）
CC=$TOOLCHAIN/bin/aarch64-linux-android$API-clang
CXX=$TOOLCHAIN/bin/aarch64-linux-android$API-clang++
# NDK头文件环境
SYSROOT=$TOOLCHAIN/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android-
# so输出路径
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-march=$CPU"
build_android
find . -name *.o -exec rm -rf {} \;
find . -name *.d -exec rm -rf {} \;
cp -rf config.h config.arm64.h
make -j12 > make.arm64.log


# CPU架构
#armv7-a
make clean
ARCH=arm
CPU=armv7-a
CC=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang
CXX=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang++
SYSROOT=$TOOLCHAIN/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU "
build_android
find . -name *.o -exec rm -rf {} \;
find . -name *.d -exec rm -rf {} \;
cp -rf config.h config.arm.h
make -j12 > make.arm.log


cat make.arm.log |sed -e '/GEN\t/d;/LD\t/d;s/AS\t\(.*\)\.o$/\1.S /g;s/CC\t\(.*\)\.o/\1.c /g' > ffmpeg.arm.src
cat make.arm64.log |sed -e '/GEN\t/d;/LD\t/d;s/AS\t\(.*\)\.o$/\1.S /g;s/CC\t\(.*\)\.o/\1.c /g' > ffmpeg.arm64.src


notExce(){


make clean
# CPU架构
#armv7-a
ARCH=arm
CPU=armv7-a
CC=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang
CXX=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang++
SYSROOT=$TOOLCHAIN/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU "
build_android
find . -name *.o -exec rm -rf {} \;
find . -name *.d -exec rm -rf {} \;

make clean
#armv8-a
ARCH=arm64
CPU=armv8-a
# r21版本的ndk中所有的编译器都在/toolchains/llvm/prebuilt/darwin-x86_64/目录下（clang）
CC=$TOOLCHAIN/bin/aarch64-linux-android$API-clang
CXX=$TOOLCHAIN/bin/aarch64-linux-android$API-clang++
# NDK头文件环境
SYSROOT=$TOOLCHAIN/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android-
# so输出路径
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-march=$CPU"
build_android


#x86
ARCH=x86
CPU=x86
CC=$TOOLCHAIN/bin/i686-linux-android$API-clang
CXX=$TOOLCHAIN/bin/i686-linux-android$API-clang++
SYSROOT=$TOOLCHAIN/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/i686-linux-android-
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32"
build_android

#x86_64
ARCH=x86_64
CPU=x86-64
CC=$TOOLCHAIN/bin/x86_64-linux-android$API-clang
CXX=$TOOLCHAIN/bin/x86_64-linux-android$API-clang++
SYSROOT=$TOOLCHAIN/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/x86_64-linux-android-
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-march=$CPU -msse4.2 -mpopcnt -m64 -mtune=intel"
# 方法调用
build_android
}
