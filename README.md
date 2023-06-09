# ����STM32F4��Ƭ�������Ž�ϵͳ

```
2023��6��9��09:42:30
```

[toc]

> ���:����Ŀ����STM32F4ϵ�е�Ƭ����Ƶ������Ž�ϵͳ,�漰��ģ����AS608ָ��ģ�顢RC522��Ƶģ�顢ESP8266WIFIģ�顢�����LCD����������Ŀ�漰�������С�����Լ���˴�������ú��ڽ��п�Դ��
>
> ESP8266WIFIģ��Ĵ����� Arduino���򵥶���д�ģ���������ļ���`Arduino`��
>
> �����Ŀ�����а����Ļ�����л��Ұ�æ���С����Star??��лл��

## 1.����˵��

ָ��ģ��:
  ����3
  PA4 -----> ָ��ģ��wkt ��ɫ
  PB10 ---->ָ��ģ�� ��ɫ
  PB11---->ָ��ģ�� ��ɫ
  ��ɫ(GND) ��ɫ  ��ɫ(3.3V)

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

LED:
  PA1 ----> LED

LCD������:
  PB0
  PB1
  PB2
  PB12
  PB13

���:
  PD12 ----> ���

����1ʹ��������

## 2.������Ϣ

?**����Ŀ��ʹ��Clion����,��Ҫ������Ӧ�Ŀ�������.**

���м��ļ�ʱ����Ҫ��CMakeLists.txt�����Ӧ���ļ���Ϣ

```
#ͷ�ļ�
include_directories(Core/Inc Drivers/STM32F4xx_HAL_Driver/Inc Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
Drivers/CMSIS/Device/ST/STM32F4xx/Include Drivers/CMSIS/Include
SYSTEM/
USER/
UTILS/
)

add_definitions(-DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx)
#�ļ���
file(GLOB_RECURSE SOURCES "Core/*.*" "Drivers/*.*"
"SYSTEM/*.*" "USER/*.*" "UTILS/*.*"
)
```

���ʹ��Keil������������,�봫ͳ����û���κ�����,ֻ����ʹ��STM32CubeMX������Ŀ����ѡ��Keil�Ϳ���

![image-20230609095143406](http://qnoss.xlike.cc/img/image-20230609095143406.png)

�����ʾ��Ƶ:  https://www.bilibili.com/video/BV1Ds4y1z7X6/

