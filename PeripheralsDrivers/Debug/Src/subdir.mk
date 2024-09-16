################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/TFT_ILI9486.c \
../Src/adc_driver_hal.c \
../Src/exti_driver_hal.c \
../Src/gpio_driver_hal.c \
../Src/i2c_driver_hal.c \
../Src/pll_driver_hal.c \
../Src/pwm_driver_hal.c \
../Src/spi_driver_hal.c \
../Src/systick_driver_hal.c \
../Src/timer_driver_hal.c \
../Src/usart_driver_hal.c 

OBJS += \
./Src/TFT_ILI9486.o \
./Src/adc_driver_hal.o \
./Src/exti_driver_hal.o \
./Src/gpio_driver_hal.o \
./Src/i2c_driver_hal.o \
./Src/pll_driver_hal.o \
./Src/pwm_driver_hal.o \
./Src/spi_driver_hal.o \
./Src/systick_driver_hal.o \
./Src/timer_driver_hal.o \
./Src/usart_driver_hal.o 

C_DEPS += \
./Src/TFT_ILI9486.d \
./Src/adc_driver_hal.d \
./Src/exti_driver_hal.d \
./Src/gpio_driver_hal.d \
./Src/i2c_driver_hal.d \
./Src/pll_driver_hal.d \
./Src/pwm_driver_hal.d \
./Src/spi_driver_hal.d \
./Src/systick_driver_hal.d \
./Src/timer_driver_hal.d \
./Src/usart_driver_hal.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE -c -I"/home/imjeviz/STM32CubeIDE/workspace_1.10.1/CMSIS-repo/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"/home/imjeviz/STM32CubeIDE/workspace_1.10.1/CMSIS-repo/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Include" -I"/home/imjeviz/github/Jemunozm_TallerV/PeripheralsDrivers/Inc" -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/TFT_ILI9486.cyclo ./Src/TFT_ILI9486.d ./Src/TFT_ILI9486.o ./Src/TFT_ILI9486.su ./Src/adc_driver_hal.cyclo ./Src/adc_driver_hal.d ./Src/adc_driver_hal.o ./Src/adc_driver_hal.su ./Src/exti_driver_hal.cyclo ./Src/exti_driver_hal.d ./Src/exti_driver_hal.o ./Src/exti_driver_hal.su ./Src/gpio_driver_hal.cyclo ./Src/gpio_driver_hal.d ./Src/gpio_driver_hal.o ./Src/gpio_driver_hal.su ./Src/i2c_driver_hal.cyclo ./Src/i2c_driver_hal.d ./Src/i2c_driver_hal.o ./Src/i2c_driver_hal.su ./Src/pll_driver_hal.cyclo ./Src/pll_driver_hal.d ./Src/pll_driver_hal.o ./Src/pll_driver_hal.su ./Src/pwm_driver_hal.cyclo ./Src/pwm_driver_hal.d ./Src/pwm_driver_hal.o ./Src/pwm_driver_hal.su ./Src/spi_driver_hal.cyclo ./Src/spi_driver_hal.d ./Src/spi_driver_hal.o ./Src/spi_driver_hal.su ./Src/systick_driver_hal.cyclo ./Src/systick_driver_hal.d ./Src/systick_driver_hal.o ./Src/systick_driver_hal.su ./Src/timer_driver_hal.cyclo ./Src/timer_driver_hal.d ./Src/timer_driver_hal.o ./Src/timer_driver_hal.su ./Src/usart_driver_hal.cyclo ./Src/usart_driver_hal.d ./Src/usart_driver_hal.o ./Src/usart_driver_hal.su

.PHONY: clean-Src

