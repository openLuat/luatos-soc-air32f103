本DAPLink固件已升级至V2版本，烧录速度为V1的数倍（100KB/s以上）
源码：https://gitee.com/openLuat/daplink
此处v1与v2固件均已提供，可随意取用

升级方法：
1.  走自带的BootLoader升级：
     上电前，短接对外输出Reset的引脚与GND，插入USB，即可进入BootLoader升级模式；
     断开短接的连线；
     将【air32_daplink_vx_iap.hex】复制到虚拟的U盘设备中；
2.  使用ISP烧录，刷入【air32_daplink_vx.hex】
3.  使用SWD/JTAG烧录，刷入【air32_daplink_vx.hex】

【注意】
2023年上半年前的出厂固件均为DAPLink V1版本
若想升级到V2版本，需要连带BootLoader一起升级，否则无法正常启动，不能使用方法1升级

灯功能描述：
PB2：dap运行指示灯，常亮
PB10：串口CDC指示，串口有数据时闪烁
PB11：SWD使用指示，烧录或调试时亮
PA0（开发板未使用）：MSC U盘烧录功能指示，有数据时闪烁