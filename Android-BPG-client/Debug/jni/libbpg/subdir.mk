################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../jni/libbpg/libbpg.c 

OBJS += \
./jni/libbpg/libbpg.o 

C_DEPS += \
./jni/libbpg/libbpg.d 


# Each subdirectory must supply rules for building sources it contributes
jni/libbpg/%.o: ../jni/libbpg/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I/home/alex/android-ndk-r10d/platforms/android-14/arch-arm/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


