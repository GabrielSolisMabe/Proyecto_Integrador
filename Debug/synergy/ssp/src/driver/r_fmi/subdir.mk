################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../synergy/ssp/src/driver/r_fmi/r_fmi.c 

OBJS += \
./synergy/ssp/src/driver/r_fmi/r_fmi.o 

C_DEPS += \
./synergy/ssp/src/driver/r_fmi/r_fmi.d 


# Each subdirectory must supply rules for building sources it contributes
synergy/ssp/src/driver/r_fmi/%.o: ../synergy/ssp/src/driver/r_fmi/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	C:\Renesas\e2_studio\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -D_RENESAS_SYNERGY_ -I"C:\REP\1\synergy_cfg\ssp_cfg\bsp" -I"C:\REP\1\synergy_cfg\ssp_cfg\driver" -I"C:\REP\1\synergy\ssp\inc" -I"C:\REP\1\synergy\ssp\inc\bsp" -I"C:\REP\1\synergy\ssp\inc\bsp\cmsis\Include" -I"C:\REP\1\synergy\ssp\inc\driver\api" -I"C:\REP\1\synergy\ssp\inc\driver\instances" -I"C:\REP\1\src" -I"C:\REP\1\src\synergy_gen" -I"C:\REP\1\synergy_cfg\ssp_cfg\framework" -I"C:\REP\1\synergy\ssp\inc\framework\api" -I"C:\REP\1\synergy\ssp\inc\framework\instances" -I"C:\REP\1\synergy\ssp\inc\framework\tes" -I"C:\REP\1\synergy_cfg\ssp_cfg\framework\el" -I"C:\REP\1\synergy\ssp\inc\framework\el" -I"C:\REP\1\synergy\ssp\src\framework\el\tx" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


