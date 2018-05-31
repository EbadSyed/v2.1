################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
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
all: Z3LightSoc_2.axf

# Tool invocations
Z3LightSoc_2.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m4 -mthumb -T "Z3LightSoc_2.ld" -nostdlib -Xlinker --gc-sections -Xlinker -Map="Z3LightSoc_2.map" -mfpu=fpv4-sp-d16 -mfloat-abi=softfp --specs=nano.specs -o Z3LightSoc_2.axf "./Z3LightSoc_2_callbacks.o" 
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: Z3LightSoc_2.hex'
	arm-none-eabi-objcopy -O ihex "Z3LightSoc_2.axf" "Z3LightSoc_2.hex"
	@echo ' '

	@echo 'Building bin file: Z3LightSoc_2.bin'
	arm-none-eabi-objcopy -O binary "Z3LightSoc_2.axf" "Z3LightSoc_2.bin"
	@echo ' '

	@echo 'Building s37 file: Z3LightSoc_2.s37'
	arm-none-eabi-objcopy -O srec "Z3LightSoc_2.axf" "Z3LightSoc_2.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "Z3LightSoc_2.axf"
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) Z3LightSoc_2.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
