################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Z3LightSoc_2_callbacks.c 

OBJS += \
./Z3LightSoc_2_callbacks.o 

C_DEPS += \
./Z3LightSoc_2_callbacks.d 


# Each subdirectory must supply rules for building sources it contributes
Z3LightSoc_2_callbacks.o: ../Z3LightSoc_2_callbacks.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m4 -mthumb -std=c99 -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"Z3LightSoc_2_callbacks.d" -MT"Z3LightSoc_2_callbacks.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


