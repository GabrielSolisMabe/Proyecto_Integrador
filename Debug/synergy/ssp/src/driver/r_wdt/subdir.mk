################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../synergy/ssp/src/driver/r_wdt/r_wdt.c 

OBJS += \
./synergy/ssp/src/driver/r_wdt/r_wdt.o 

C_DEPS += \
./synergy/ssp/src/driver/r_wdt/r_wdt.d 


# Each subdirectory must supply rules for building sources it contributes
synergy/ssp/src/driver/r_wdt/%.o: ../synergy/ssp/src/driver/r_wdt/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	C:\Renesas\e2_studio\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -D_RENESAS_SYNERGY_ -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy_cfg\ssp_cfg\bsp" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy_cfg\ssp_cfg\driver" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy\ssp\inc" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy\ssp\inc\bsp" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy\ssp\inc\bsp\cmsis\Include" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy\ssp\inc\driver\api" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy\ssp\inc\driver\instances" -I"D:\Repositorio_TYP\Proyecto_Integrador\src" -I"D:\Repositorio_TYP\Proyecto_Integrador\src\synergy_gen" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy_cfg\ssp_cfg\framework" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy\ssp\inc\framework\api" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy\ssp\inc\framework\instances" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy\ssp\inc\framework\tes" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy_cfg\ssp_cfg\framework\el" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy\ssp\inc\framework\el" -I"D:\Repositorio_TYP\Proyecto_Integrador\synergy\ssp\src\framework\el\tx" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


