# luatos-soc-air32f103

本芯片属于平替产品, 请注意与 air103 的区分.

## 介绍

官方wiki https://wiki.luatos.com/chips/air32f103/index.html

## DEMO目录索引

ModuleDemo目录里有各种外设使用示例，结构如下

```
├─ADC
│  ├─ADC_AnalogWatchdog ADC模拟看门狗
│  ├─ADC_Double		    ADC同步注入
│  ├─ADC_Single		    ADC单通道
│  └─ADC_VrefintTemper  ADC内部基准和内部温度传感器
├─AT_Air724_Air600U 	AT驱动合宙Air724，Air780等进行联网
├─BENCHMARK
│  └─COREMARK           跑分
├─BKP
│  ├─BKP_BackupData     BKP区读写
│  └─BKP_Tamper         备份寄存器和侵入检查
├─CAN
│  ├─CAN_LoopBack       CAN自回环测试
│  └─CAN_Normal         CAN收发测试
├─CRC
│  └─CRC_DifferentCrcMode CRC校验
├─CRYPT                 硬件SM，AES等加解密库测试
├─DAC
│  ├─DAC_OutAudio       DAC播放音频
│  ├─DAC_OutDMASineWave DAC的DMA模式
│  ├─DAC_OutNoiseWave  	DAC白噪声输出
│  └─DAC_OutTriangleWave DAC输出三角波
├─FLASH
│  ├─FLASH_Eeprom       EEPROM模拟
│  └─FLASH_EraseProgram FLASH擦写
├─FreeRTOS
│  ├─FreeRTOS       FreeRTOS移植工程
├─GCC_DEMO			GCC+xmake工程
├─GPIO
│  ├─GPIO_IOInput       GPIO输入
│  └─GPIO_IOOut         GPIO输出
├─IAP
│  ├─APP                IAP升级用户代码
│  └─BootLoader           IAP升级Bootloader
├─IIC
│  └─IIC_IntTransmit    IIC中断发送
├─IIS
│  ├─IIS_CS4344         IIS驱动CS4344
│  ├─IIS_Dma            IIS DMA模式
│  └─IIS_Int            IIS中断模式
├─IWDG
│  └─IWDG_Reset 		独立看门狗复位
├─LVGL_FreeRTOS
│  └─air32f103cc-gc9306x-lvgl   LVGL+FreeRTOS
├─MCO
│  └─MCO_PllDiv 		MCO输出，air32支持更多分频系数
├─MPU
│  └─MPU_Test   		MPU测试
├─NES					NES模拟器
├─OTP
│  └─OTP_WriteRead  	OTP写读
├─PWR
│  ├─PWR_PVD    		PVD电压监测
│  ├─PWR_Standby    	电源模式切换为待机模式
│  └─PWR_Stop       	电源模式切换为停止模式
├─RCC
│  └─RCC_ClockConfig    时钟配置，使用216M时钟参考这个例程，最高可达256M
├─RTC
│  ├─RTC_Calendar   	RTC日历
│  └─RTC_LSICalib   	RTC LSI校准
├─RT_Thread_Nano		RT_Thread_Nano
├─SDIO
│  └─SDIO_SDCardFatfs   SD卡读写
├─SPI
│  ├─SPI_Air10x_LCD 	SPI驱动Air10x的LCD
│  ├─SPI_DMA    		SPI DMA模式
│  └─SPI_Flash  		SPI驱动Flash
├─TIM
│  ├─TIM_Basic  		TIM基本定时器
│  └─TIM_Touch  		TIM触摸屏
|  └─TIM_PWM_DMA  		TIM PWM with DMA输出
├─TRNG
│  ├─TRNG_IntTest   	TRNG中断测试
│  └─TRNG_PollingTest   TRNG轮询测试
├─USART
│  ├─USART_Asyn 		USART异步收发
│  ├─USART_Irda 		USART红外收发
│  ├─USART_LIN  		USART LIN收发
│  ├─USART_Rs485    	USART RS485收发
│  └─USART_SmartCard    USART 智能卡收发
├─USB
│  ├─Device_Firmware_Upgrade    USB DFU
│  ├─Keyboard   				USB键盘
│  ├─Mass_Storage   			USB 储存设备
│  └─Virtual_COM_Port   		USB虚拟串口
├─WS2812    			WS2812驱动
└─WWDG
│  ├─WWDG_Int   		WWDG中断
│  └─WWDG_Reset 		WWDG复位
```



## 开发板购买链接

![](luatos_shop.jpg)
