ָ��ģ��:
    ����3
    PA4 -----> ָ��ģ��wkt ��ɫ
    PB10 ---->ָ��ģ�� ��ɫ
    PB11---->ָ��ģ�� ��ɫ
    ��ɫ(GND) ��ɫ��ɫ(3.3V)

RC522:
    PA5 ----> SCL
    PA6 ----> MISO
    PA7 ---->MOSI
    PC0 ---->RST
    PC1 ---->SDA
    IRQ ---->����


ESP8266:
    ����2
    PA2 ----> TX
    PA3 ----> RX


�ſ�:
 PA1 ----> �ſ�

LCD������:
    PB0
    PB1
    PB2
    PB12
    PB13

���:
    PD12    ----> ���


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
