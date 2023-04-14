本DAPLink固件已升级至V2高速版本
源码：https://gitee.com/openLuat/daplink

升级方法：
1. 走自带的BootLoader升级：
上电前，短接对外输出Reset的引脚与GND，插入USB，即可进入BootLoader升级模式
断开短接的连线
将air32_daplink_iap.hex复制到虚拟的U盘设备中
2. 使用ISP烧录：此方法不再赘述，刷入air32_daplink.hex文件即可
3. 使用SWD/JTAG烧录：同上，刷入air32_daplink.hex文件即可

注意！
2023年上半年前的出厂固件均为DAPLink V1版本
若想升级到V2版本，需要连带BootLoader一起升级，否则无法正常启动
