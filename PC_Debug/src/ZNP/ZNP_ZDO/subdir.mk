################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ZNP/ZNP_ZDO/Znp_Zdo.c 

OBJS += \
./src/ZNP/ZNP_ZDO/Znp_Zdo.o 

C_DEPS += \
./src/ZNP/ZNP_ZDO/Znp_Zdo.d 


# Each subdirectory must supply rules for building sources it contributes
src/ZNP/ZNP_ZDO/%.o: ../src/ZNP/ZNP_ZDO/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DPC_RUNNING -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/Queue" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/SerialCommunication" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/universal" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/ZNP" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/zcl" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/DeviceManager" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/log" -I"/home/chaunm/Working/ZigbeeHost/RpiProj/src/Actor" -include"/home/chaunm/Working/ZigbeeHost/RpiProj/src/universal/typesdef.h" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


