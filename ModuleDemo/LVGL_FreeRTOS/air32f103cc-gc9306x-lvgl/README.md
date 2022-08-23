# LVGL+FreeRTOS示例工程

本工程针对`Air32F103CCT6`芯片，使用硬件SPI+DMA的方式驱动合宙官方`GC9306X`控制器的`320x240`LCD屏幕，并支持双缓冲模式。由于使用了`FreeRTOS`，保证了DMA传输过程中CPU能够进入休眠，降低系统功耗。

*请使用较新版本的Keil，较旧版本的Keil可能会出现编译出的程序运行错误的问题*

## 工程组件

- FreeRTOS来自[FreeRTOS官网](https://www.freertos.org/a00104.html)中最新的`LTS 202012.04`版本；
- LVGL来自官网[LVGL官方GitHub仓库](https://github.com/lvgl/lvgl/releases)中最新的`LVGL 8.3.1`版本；
- 固件库标准库来自本仓库的[STM32F10x, STM32L1xx and STM32F3xx USB Full Speed Device Library](https://img.yuanze.wang/posts/air32-lvgl-freertos/stsw-stm32121.zip)中的`3.6.1`版本（该版本没有独立发布过，单独发布的最新版本是`3.5.0`）。

## 组件库的裁剪与优化

由于芯片的RAM空间有限，因此需要对芯片的RAM空间进行一定的规划与优化。程序中占用RAM较大的部分与相应的规划如下：

- 系统栈：由于使用了`FreeRTOS`，各个Task有其自己的任务栈，因此系统栈只有ISR与`main`函数使用。因此，在`startup_air32f10x.s`中将`Stack_Size`改为`0x00000100`，即256字节。
- 任务栈：目前的代码中只有三个Task，分别是`LVGL Task` `LED Task`与`IDLE Task`。其中，`LED Task`与`IDLE Task`都非常简单，为它们设置`128`字节的任务栈；`LVGL Task`较为复杂，根据官方推荐的2-8k范围，设置为`4k`。
- LVGL堆：LVGL的所有的句柄都是动态内存，因此其自己维护了一个堆空间。堆空间的大小可以在`lv_conf.h`中的`LV_MEM_SIZE`中修改，您可以根据自己使用的UI复杂度对其进行修改。对于`benchmark` demo，`12k`即可满足要求。
- LVGL缓冲区：LVGL需要将画面渲染到缓冲区中，之后再刷新到屏幕上。本工程支持单缓冲与双缓冲模式（可以在Keil的Target中选择），单缓冲模式使用1个`240x40`像素的缓冲区，双缓冲模式则使用2个`240x40`像素的缓冲区。使用双缓冲模式可以在DMA控制器向屏幕写入一个缓冲区的数据时，CPU继续渲染到另一个缓冲区中，提升渲染效率，但会占用双倍的缓冲区。

同时，由于芯片的ROM空间也有限，因此我裁剪了一些`LVGL`与`FreeRTOS`的功能。您可以在`lv_conf.h`与`FreeRTOSConfig.h`中自行开关它们。`benchmark`demo中包含了大量的字体与图像，因此导致最终编译生成的bin文件较大。只使用`FreeRTOS`与`LVGL`内核时，ROM占用约`120k`。使用常用的空间后，还能剩余约`100k`空间给用户开发自己的应用。

## 存储占用与运行结果

单缓冲模式的存储空间占用情况如下：

```
Total RO  Size (Code + RO Data)               230904 ( 225.49kB)
Total RW  Size (RW Data + ZI Data)             40768 (  39.81kB)
Total ROM Size (Code + RO Data + RW Data)     231472 ( 226.05kB)
```

跑分结果约`90`fps。

双缓冲模式的存储空间占用情况如下：

```
Total RO  Size (Code + RO Data)               230928 ( 225.52kB)
Total RW  Size (RW Data + ZI Data)             59968 (  58.56kB)
Total ROM Size (Code + RO Data + RW Data)     231496 ( 226.07kB)
```

跑分结果约`120`fps。

## 联系作者

若您在使用本工程时遇到了任何问题，欢迎联系作者。

@wangyz1997

https://yuanze.wang
