################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ZNP/ZNP_Util/Znp_Util.c 

OBJS += \
./src/ZNP/ZNP_Util/Znp_Util.o 

C_DEPS += \
./src/ZNP/ZNP_Util/Znp_Util.d 


# Each subdirectory must supply rules for building sources it contributes
src/ZNP/ZNP_Util/%.o: ../src/ZNP/ZNP_Util/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DPC_RUNNING -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/Queue" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/SerialCommunication" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/universal" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/ZNP" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/zcl" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/DeviceManager" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/log" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/Actor" -include"/home/chaunm/Working/ZigbeeHost/RpiProj/src/universal/typesdef.h" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


