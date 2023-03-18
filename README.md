指纹模块:
    串口3
    PA4 -----> 指纹模块wkt 蓝色
    PB10 ---->指纹模块 白色
    PB11---->指纹模块 黄色
    黑色(GND) 绿色红色(3.3V)

RC522:
    PA5 ----> SCL
    PA6 ----> MISO
    PA7 ---->MOSI
    PC0 ---->RST
    PC1 ---->SDA
    IRQ ---->悬空


ESP8266:
    串口2
    PA2 ----> TX
    PA3 ----> RX


门口:
 PA1 ----> 门口

LCD触摸屏:
    PB0
    PB1
    PB2
    PB12
    PB13

舵机:
    PD12    ----> 舵机


include_directories(Core/Inc Drivers/STM32F4xx_HAL_Driver/Inc Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
Drivers/CMSIS/Device/ST/STM32F4xx/Include Drivers/CMSIS/Include
SYSTEM/
USER/
UTILS/
)

add_definitions(-DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx)

file(GLOB_RECURSE SOURCES "Core/*.*" "Drivers/*.*"
"SYSTEM/*.*" "USER/*.*" "UTILS/*.*"
)
