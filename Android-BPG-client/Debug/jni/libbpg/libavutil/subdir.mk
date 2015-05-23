################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../jni/libbpg/libavutil/buffer.c \
../jni/libbpg/libavutil/frame.c \
../jni/libbpg/libavutil/log2_tab.c \
../jni/libbpg/libavutil/md5.c \
../jni/libbpg/libavutil/mem.c \
../jni/libbpg/libavutil/pixdesc.c 

OBJS += \
./jni/libbpg/libavutil/buffer.o \
./jni/libbpg/libavutil/frame.o \
./jni/libbpg/libavutil/log2_tab.o \
./jni/libbpg/libavutil/md5.o \
./jni/libbpg/libavutil/mem.o \
./jni/libbpg/libavutil/pixdesc.o 

C_DEPS += \
./jni/libbpg/libavutil/buffer.d \
./jni/libbpg/libavutil/frame.d \
./jni/libbpg/libavutil/log2_tab.d \
./jni/libbpg/libavutil/md5.d \
./jni/libbpg/libavutil/mem.d \
./jni/libbpg/libavutil/pixdesc.d 


# Each subdirectory must supply rules for building sources it contributes
jni/libbpg/libavutil/%.o: ../jni/libbpg/libavutil/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I/home/alex/android-ndk-r10d/platforms/android-14/arch-arm/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


