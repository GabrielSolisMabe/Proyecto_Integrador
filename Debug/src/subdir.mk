################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/hal_entry.c \
../src/lcd_thread_entry.c \
../src/system_thread_entry.c 

OBJS += \
./src/hal_entry.o \
./src/lcd_thread_entry.o \
./src/system_thread_entry.o 

C_DEPS += \
./src/hal_entry.d \
./src/lcd_thread_entry.d \
./src/system_thread_entry.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	C:\Renesas\e2_studio740\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -D_RENESAS_SYNERGY_ -I"C:\repos\Proyecto_Integrador\synergy_cfg\ssp_cfg\bsp" -I"C:\repos\Proyecto_Integrador\synergy_cfg\ssp_cfg\driver" -I"C:\repos\Proyecto_Integrador\synergy\ssp\inc" -I"C:\repos\Proyecto_Integrador\synergy\ssp\inc\bsp" -I"C:\repos\Proyecto_Integrador\synergy\ssp\inc\bsp\cmsis\Include" -I"C:\repos\Proyecto_Integrador\synergy\ssp\inc\driver\api" -I"C:\repos\Proyecto_Integrador\synergy\ssp\inc\driver\instances" -I"C:\repos\Proyecto_Integrador\src" -I"C:\repos\Proyecto_Integrador\src\synergy_gen" -I"C:\repos\Proyecto_Integrador\synergy_cfg\ssp_cfg\framework" -I"C:\repos\Proyecto_Integrador\synergy_cfg\ssp_cfg\framework\el" -I"C:\repos\Proyecto_Integrador\synergy\ssp\inc\framework\el" -I"C:\repos\Proyecto_Integrador\synergy\ssp\src\framework\el\tx" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


