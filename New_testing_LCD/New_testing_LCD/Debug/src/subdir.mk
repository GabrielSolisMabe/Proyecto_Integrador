################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/hal_entry.c \
../src/system_thread_entry.c 

OBJS += \
./src/hal_entry.o \
./src/system_thread_entry.o 

C_DEPS += \
./src/hal_entry.d \
./src/system_thread_entry.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	C:\Renesas\e2_studio\e2_studio_7.4.0\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -D_RENESAS_SYNERGY_ -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\synergy_cfg\ssp_cfg\bsp" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\synergy_cfg\ssp_cfg\driver" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\synergy\ssp\inc" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\synergy\ssp\inc\bsp" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\synergy\ssp\inc\bsp\cmsis\Include" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\synergy\ssp\inc\driver\api" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\synergy\ssp\inc\driver\instances" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\src" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\src\synergy_gen" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\synergy_cfg\ssp_cfg\framework" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\synergy_cfg\ssp_cfg\framework\el" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\synergy\ssp\inc\framework\el" -I"C:\Users\SEI403\Documents\GitHub\Proyecto_Integrador\New_testing_LCD\New_testing_LCD\synergy\ssp\src\framework\el\tx" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


