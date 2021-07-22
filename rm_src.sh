#!/bin/bash
#set -x
ALL_C_SRC_FILES=$(find ./libavcodec/ -name *.c)
ALL_S_SRC_FILES=$(find ./libavcodec/ -name *.S)
ALL_SRC_FILES="$ALL_C_SRC_FILES $ALL_C_SRC_FILES"

COMPILED_ARM_FILES=$(cat ./ffmpeg.arm.src)
COMPILED_ARM64_FILES=$(cat ./ffmpeg.arm64.src)
COMPILED_FILES="$COMPILED_ARM_FILES $COMPILED_ARM64_FILES"

function is_src_include() {
    for a in ${ALL_SRC_FILES[@]}
    do
        fpath=$1
        fname=${fpath##*/}
        grep "#include .*$fname" $a > /dev/null
        if [ $? -eq 0 ];then
            echo 1
            return 1
        fi
    done
    echo 0
    return 0
}

#is_src_include $1
#exit 0

function is_src_compiled() { 
    for f in ${COMPILED_FILES[@]}
    do
        if [ "./$f" = $1 ]; then
            echo 1
            return 1
        fi
    done
    echo 0
    return 0
}


for f_c in ${ALL_C_SRC_FILES[@]}
do
    is_compiled=$(is_src_compiled $f_c)
    if [[ $is_compiled == 0 ]]; then
        is_include=$(is_src_include $f_c)
        if [[ $is_include == 0 ]]; then
            echo $f_c "is not compiled."
            #rm $f_c
        fi
    fi
done

for f_c in ${ALL_S_SRC_FILES[@]}
do
    is_compiled=$(is_src_compiled $f_c)
    if [[ $is_compiled == 0 ]]; then
        is_include=$(is_src_include $f_c)
        if [[ $is_include == 0 ]]; then
            echo $f_c "is not compiled."
            #rm $f_c
        fi
    fi
done


notExce(){


BAK_FILES=$(find ./libavcodec/ -name *.bak)
for f_c in ${BAK_FILES[@]}
do
    mv $f_c $(echo $f_c | sed 's/\.bak$//g')
done


rm_files="libavformat/ac3dec.c libavformat/spdifdec.c libavformat/spdifenc.c libavcodec/aarch64/fmtconvert_init.c libavcodec/aarch64/fmtconvert_neon.S libavcodec/aarch64/synth_filter_init.c libavcodec/aarch64/synth_filter_neon.S libavcodec/ac3.c libavcodec/ac3dec_data.c libavcodec/ac3dec_fixed.c libavcodec/ac3dec_float.c libavcodec/ac3dsp.c libavcodec/ac3enc.c libavcodec/ac3enc_fixed.c libavcodec/ac3enc_float.c libavcodec/amrnbdec.c libavcodec/arm/ac3dsp_arm.S libavcodec/arm/ac3dsp_armv6.S libavcodec/arm/ac3dsp_init_arm.c libavcodec/arm/ac3dsp_neon.S libavcodec/arm/fmtconvert_init_arm.c libavcodec/arm/fmtconvert_neon.S libavcodec/arm/fmtconvert_vfp.S libavcodec/arm/synth_filter_init_arm.c libavcodec/arm/synth_filter_neon.S libavcodec/arm/synth_filter_vfp.S libavcodec/cavs.c libavcodec/cavsdata.c libavcodec/cavsdec.c libavcodec/cavsdsp.c libavcodec/dca.c libavcodec/dca_core.c libavcodec/dca_exss.c libavcodec/dca_lbr.c libavcodec/dca_xll.c libavcodec/dcaadpcm.c libavcodec/dcadata.c libavcodec/dcadct.c libavcodec/dcadec.c libavcodec/dcadsp.c libavcodec/dcaenc.c libavcodec/dcahuff.c libavcodec/eac3_data.c libavcodec/eac3enc.c libavcodec/flacenc.c libavcodec/fmtconvert.c libavcodec/mlpenc.c libavcodec/synth_filter.c libavcodec/utvideoenc.c"
echo $rm_files

for s in ${rm_files[@]}
	#也可以写成for element in ${array[*]}
do
	echo $s
	rm -rf $s
done

#rm -rf "$rm_files"

}

