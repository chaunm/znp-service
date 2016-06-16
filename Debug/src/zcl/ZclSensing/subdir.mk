################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/zcl/ZclSensing/zcl_ss_occupancy.c \
../src/zcl/ZclSensing/zcl_ss_rehumid.c \
../src/zcl/ZclSensing/zcl_ss_temp.c 

OBJS += \
./src/zcl/ZclSensing/zcl_ss_occupancy.o \
./src/zcl/ZclSensing/zcl_ss_rehumid.o \
./src/zcl/ZclSensing/zcl_ss_temp.o 

C_DEPS += \
./src/zcl/ZclSensing/zcl_ss_occupancy.d \
./src/zcl/ZclSensing/zcl_ss_rehumid.d \
./src/zcl/ZclSensing/zcl_ss_temp.d 


# Each subdirectory must supply rules for building sources it contributes
src/zcl/ZclSensing/%.o: ../src/zcl/ZclSensing/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -DPI_RUNNING -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/DeviceManager" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/log" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/zcl" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/ZNP" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/Queue" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/SerialCommunication" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/universal" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/Actor" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/lib/jansson" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/lib/uuid/include" -include"/home/chaunm/Working/ZigbeeHost/RpiProj/src/universal/typesdef.h" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


