################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../jni/libbpg/libavcodec/bit_depth_template.c \
../jni/libbpg/libavcodec/cabac.c \
../jni/libbpg/libavcodec/golomb.c \
../jni/libbpg/libavcodec/hevc.c \
../jni/libbpg/libavcodec/hevc_cabac.c \
../jni/libbpg/libavcodec/hevc_filter.c \
../jni/libbpg/libavcodec/hevc_mvs.c \
../jni/libbpg/libavcodec/hevc_ps.c \
../jni/libbpg/libavcodec/hevc_refs.c \
../jni/libbpg/libavcodec/hevc_sei.c \
../jni/libbpg/libavcodec/hevcdsp.c \
../jni/libbpg/libavcodec/hevcdsp_template.c \
../jni/libbpg/libavcodec/hevcpred.c \
../jni/libbpg/libavcodec/hevcpred_template.c \
../jni/libbpg/libavcodec/utils.c \
../jni/libbpg/libavcodec/videodsp.c \
../jni/libbpg/libavcodec/videodsp_template.c 

OBJS += \
./jni/libbpg/libavcodec/bit_depth_template.o \
./jni/libbpg/libavcodec/cabac.o \
./jni/libbpg/libavcodec/golomb.o \
./jni/libbpg/libavcodec/hevc.o \
./jni/libbpg/libavcodec/hevc_cabac.o \
./jni/libbpg/libavcodec/hevc_filter.o \
./jni/libbpg/libavcodec/hevc_mvs.o \
./jni/libbpg/libavcodec/hevc_ps.o \
./jni/libbpg/libavcodec/hevc_refs.o \
./jni/libbpg/libavcodec/hevc_sei.o \
./jni/libbpg/libavcodec/hevcdsp.o \
./jni/libbpg/libavcodec/hevcdsp_template.o \
./jni/libbpg/libavcodec/hevcpred.o \
./jni/libbpg/libavcodec/hevcpred_template.o \
./jni/libbpg/libavcodec/utils.o \
./jni/libbpg/libavcodec/videodsp.o \
./jni/libbpg/libavcodec/videodsp_template.o 

C_DEPS += \
./jni/libbpg/libavcodec/bit_depth_template.d \
./jni/libbpg/libavcodec/cabac.d \
./jni/libbpg/libavcodec/golomb.d \
./jni/libbpg/libavcodec/hevc.d \
./jni/libbpg/libavcodec/hevc_cabac.d \
./jni/libbpg/libavcodec/hevc_filter.d \
./jni/libbpg/libavcodec/hevc_mvs.d \
./jni/libbpg/libavcodec/hevc_ps.d \
./jni/libbpg/libavcodec/hevc_refs.d \
./jni/libbpg/libavcodec/hevc_sei.d \
./jni/libbpg/libavcodec/hevcdsp.d \
./jni/libbpg/libavcodec/hevcdsp_template.d \
./jni/libbpg/libavcodec/hevcpred.d \
./jni/libbpg/libavcodec/hevcpred_template.d \
./jni/libbpg/libavcodec/utils.d \
./jni/libbpg/libavcodec/videodsp.d \
./jni/libbpg/libavcodec/videodsp_template.d 


# Each subdirectory must supply rules for building sources it contributes
jni/libbpg/libavcodec/%.o: ../jni/libbpg/libavcodec/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I/home/alex/android-ndk-r10d/platforms/android-14/arch-arm/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


