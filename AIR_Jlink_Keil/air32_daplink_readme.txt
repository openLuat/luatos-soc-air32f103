本DAPLink固件已升级至V2版本，烧录速度为V1的数倍（100KB/s以上）
源码：https://gitee.com/openLuat/daplink
此处v1与v2固件均已提供，可随意取用

【注意】
Air32 Core开发板出厂固件均为DAPLink V1版本（小蓝板无固件）
若想升级到V2版本，需要连带BootLoader一起升级，否则无法正常启动，不能使用方法1升级

【升级方法】
1.  走自带的BootLoader升级（开发板不能，原因见上面）：
     上电前，短接对外输出Reset的引脚与GND，插入USB，即可进入BootLoader升级模式；
     断开短接的连线；
     将【air32_daplink_vx_iap.hex】复制到虚拟的U盘设备中；
2.  使用ISP烧录，刷入【air32_daplink_vx.hex】
3.  使用SWD/JTAG烧录，刷入【air32_daplink_vx.hex】

【支持JTAG的固件】
air32_daplink_v2_JTAG_iap.hex为支持JTAG的版本，对应官方DAPLink烧录器引脚变更如下：
串口RTS（PA9）引脚，功能变为TDI
串口DTR（PA10）引脚，功能变为TDO
按需刷入JTAG功能的固件

【灯功能描述】
PB2：dap运行指示灯，常亮
PB10：串口CDC指示，串口有数据时闪烁
PB11：SWD使用指示，烧录或调试时亮
PA0（开发板未使用）：MSC U盘烧录功能指示，有数据时闪烁