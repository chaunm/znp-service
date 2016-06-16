################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/zcl/ZclSensing/subdir.mk
-include src/zcl/ZclSecurity/subdir.mk
-include src/zcl/ZclGeneric/subdir.mk
-include src/zcl/subdir.mk
-include src/universal/subdir.mk
-include src/log/subdir.mk
-include src/ZNP/ZNP_ZDO/subdir.mk
-include src/ZNP/ZNP_Util/subdir.mk
-include src/ZNP/ZNP_System/subdir.mk
-include src/ZNP/ZNP_SimpleAPI/subdir.mk
-include src/ZNP/ZNP_AF/subdir.mk
-include src/ZNP/subdir.mk
-include src/SerialCommunication/subdir.mk
-include src/Queue/subdir.mk
-include src/DeviceManager/subdir.mk
-include src/Actor/common/subdir.mk
-include src/Actor/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: ZigbeeHostPC

# Tool invocations
ZigbeeHostPC: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "ZigbeeHostPC" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) ZigbeeHostPC
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
