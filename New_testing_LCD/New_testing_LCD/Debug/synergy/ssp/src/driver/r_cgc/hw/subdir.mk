################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../synergy/ssp/src/driver/r_cgc/hw/hw_cgc.c 

OBJS += \
./synergy/ssp/src/driver/r_cgc/hw/hw_cgc.o 

C_DEPS += \
./synergy/ssp/src/driver/r_cgc/hw/hw_cgc.d 


# Each subdirectory must supply rules for building sources it contributes
synergy/ssp/src/driver/r_cgc/hw/%.o: ../synergy/ssp/src/driver/r_cgc/hw/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	C:\Renesas\e2_studio\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -D_RENESAS_SYNERGY_ -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy_cfg\ssp_cfg\bsp" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy_cfg\ssp_cfg\driver" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy\ssp\inc" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy\ssp\inc\bsp" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy\ssp\inc\bsp\cmsis\Include" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy\ssp\inc\driver\api" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy\ssp\inc\driver\instances" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\src" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\src\synergy_gen" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy_cfg\ssp_cfg\framework" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy\ssp\inc\framework\api" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy\ssp\inc\framework\instances" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy\ssp\inc\framework\tes" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy_cfg\ssp_cfg\framework\el" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy\ssp\inc\framework\el" -I"D:\My Documents\Downloads\Renesas_SE\New_testing_LCD\synergy\ssp\src\framework\el\tx" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


