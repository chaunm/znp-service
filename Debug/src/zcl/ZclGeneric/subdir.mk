################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/zcl/ZclGeneric/zcl_gen_alarm.c \
../src/zcl/ZclGeneric/zcl_gen_basic.c \
../src/zcl/ZclGeneric/zcl_gen_group.c \
../src/zcl/ZclGeneric/zcl_gen_identity.c \
../src/zcl/ZclGeneric/zcl_gen_level.c \
../src/zcl/ZclGeneric/zcl_gen_onoff.c \
../src/zcl/ZclGeneric/zcl_gen_onoffsw.c \
../src/zcl/ZclGeneric/zcl_gen_powerconfig.c \
../src/zcl/ZclGeneric/zcl_gen_scene.c \
../src/zcl/ZclGeneric/zcl_gen_tempconfig.c 

OBJS += \
./src/zcl/ZclGeneric/zcl_gen_alarm.o \
./src/zcl/ZclGeneric/zcl_gen_basic.o \
./src/zcl/ZclGeneric/zcl_gen_group.o \
./src/zcl/ZclGeneric/zcl_gen_identity.o \
./src/zcl/ZclGeneric/zcl_gen_level.o \
./src/zcl/ZclGeneric/zcl_gen_onoff.o \
./src/zcl/ZclGeneric/zcl_gen_onoffsw.o \
./src/zcl/ZclGeneric/zcl_gen_powerconfig.o \
./src/zcl/ZclGeneric/zcl_gen_scene.o \
./src/zcl/ZclGeneric/zcl_gen_tempconfig.o 

C_DEPS += \
./src/zcl/ZclGeneric/zcl_gen_alarm.d \
./src/zcl/ZclGeneric/zcl_gen_basic.d \
./src/zcl/ZclGeneric/zcl_gen_group.d \
./src/zcl/ZclGeneric/zcl_gen_identity.d \
./src/zcl/ZclGeneric/zcl_gen_level.d \
./src/zcl/ZclGeneric/zcl_gen_onoff.d \
./src/zcl/ZclGeneric/zcl_gen_onoffsw.d \
./src/zcl/ZclGeneric/zcl_gen_powerconfig.d \
./src/zcl/ZclGeneric/zcl_gen_scene.d \
./src/zcl/ZclGeneric/zcl_gen_tempconfig.d 


# Each subdirectory must supply rules for building sources it contributes
src/zcl/ZclGeneric/%.o: ../src/zcl/ZclGeneric/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -DPI_RUNNING -DAMA0_UART_ZNP -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/DeviceManager" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/log" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/zcl" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/ZNP" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/Queue" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/SerialCommunication" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/universal" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/Actor" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/lib/jansson" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/lib/uuid/include" -include"/home/chaunm/Working/ZigbeeHost/RpiProj/src/universal/typesdef.h" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


