INCLUDE_FLAGS += -I./sources/sys/peripheral/peripheral_libraries/STM32F4xx_StdPeriph_Driver/inc

VPATH += ./sources/sys/peripheral/peripheral_libraries/STM32F4xx_StdPeriph_Driver/src

# C source files
SOURCE_C += misc.c
SOURCE_C += stm32f4xx_gpio.c
SOURCE_C += stm32f4xx_rcc.c
SOURCE_C += stm32f4xx_usart.c
SOURCE_C += stm32f4xx_i2c.c
SOURCE_C += stm32f4xx_spi.c
SOURCE_C += stm32f4xx_flash.c
SOURCE_C += stm32f4xx_dma.c
SOURCE_C += stm32f4xx_iwdg.c
SOURCE_C += stm32f4xx_tim.c
SOURCE_C += stm32f4xx_exti.c
SOURCE_C += stm32f4xx_syscfg.c
