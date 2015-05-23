################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../jni/bpg-utils/decoder.c 

OBJS += \
./jni/bpg-utils/decoder.o 

C_DEPS += \
./jni/bpg-utils/decoder.d 


# Each subdirectory must supply rules for building sources it contributes
jni/bpg-utils/%.o: ../jni/bpg-utils/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I/home/alex/android-ndk-r10d/platforms/android-14/arch-arm/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


