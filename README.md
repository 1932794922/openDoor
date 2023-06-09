# 基于STM32F4单片机智能门禁系统

```
2023年6月9日09:42:30
```

[toc]

> 简介:本项目基于STM32F4系列单片机设计的智能门禁系统,涉及的模块有AS608指纹模块、RC522射频模块、ESP8266WIFI模块、舵机、LCD触摸屏。项目涉及到的软件小程序以及后端代码整理好后在进行开源。
>
> ESP8266WIFI模块的代码是 Arduino程序单独编写的，代码放在文件夹`Arduino`里
>
> 最后本项目对您有帮助的话，感谢大家帮忙点个小星星Star??，谢谢！

## 1.引脚说明

指纹模块:
  串口3
  PA4 -----> 指纹模块wkt 蓝色
  PB10 ---->指纹模块 白色
  PB11---->指纹模块 黄色
  黑色(GND) 绿色  红色(3.3V)

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

LED:
  PA1 ----> LED

LCD触摸屏:
  PB0
  PB1
  PB2
  PB12
  PB13

舵机:
  PD12 ----> 舵机

串口1使用做调试

## 2.配置信息

?**本项目是使用Clion开发,需要配置相应的开发环境.**

自行加文件时候需要再CMakeLists.txt添加相应的文件信息

```
#头文件
include_directories(Core/Inc Drivers/STM32F4xx_HAL_Driver/Inc Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
Drivers/CMSIS/Device/ST/STM32F4xx/Include Drivers/CMSIS/Include
SYSTEM/
USER/
UTILS/
)

add_definitions(-DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx)
#文件夹
file(GLOB_RECURSE SOURCES "Core/*.*" "Drivers/*.*"
"SYSTEM/*.*" "USER/*.*" "UTILS/*.*"
)
```

如果使用Keil开发无需配置,与传统开发没有任何区别,只是在使用STM32CubeMX生成项目代码选择Keil就可以

![image-20230609095143406](http://qnoss.xlike.cc/img/image-20230609095143406.png)

相关演示视频:  https://www.bilibili.com/video/BV1Ds4y1z7X6/

