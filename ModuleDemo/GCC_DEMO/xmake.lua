set_project("air32f103")
set_xmakever("2.6.3")

set_version("0.0.2", {build = "%Y%m%d%H%M"})
add_rules("mode.debug", "mode.release")

-- 在线下载GCC并配置
package("gnu_rm")
    set_kind("toolchain")
    set_homepage("https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm")
    set_description("GNU Arm Embedded Toolchain")
    local version_map = {
        ["2021.10"] = "10.3-2021.10"
    }
    if is_host("windows") then
        set_urls("http://cdndownload.openluat.com/xmake/toolchains/gcc-arm/gcc-arm-none-eabi-$(version)-win32.zip", {version = function (version)
            return version_map[tostring(version)]
        end})
        add_versions("2021.10", "d287439b3090843f3f4e29c7c41f81d958a5323aecefcf705c203bfd8ae3f2e7")
    elseif is_host("linux") then
        set_urls("http://cdndownload.openluat.com/xmake/toolchains/gcc-arm/gcc-arm-none-eabi-$(version)-x86_64-linux.tar.bz2", {version = function (version)
            return version_map[tostring(version)]
        end})
        add_versions("2021.10", "97dbb4f019ad1650b732faffcc881689cedc14e2b7ee863d390e0a41ef16c9a3")
    end
    on_install("@windows", "@linux", function (package)
        os.vcp("*", package:installdir())
    end)
package_end()

add_requires("gnu_rm 2021.10")
set_toolchains("gnu-rm@gnu_rm")

set_warnings("allextra")
set_languages("gnu11")
set_optimize("fastest")

local device_flags = "-mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections"

target("air32f103.elf")
    set_kind("binary") 
    set_plat("cross")
    set_arch("m3")
    set_targetdir("$(buildir)/out")

    add_defines("AIR32F10X_MD")

    add_cflags(
        device_flags,
        {force = true}
    )

    add_asflags(
        device_flags,
        {force = true}
    )

    add_ldflags(
        device_flags,
        "--specs=nosys.specs"," -lc",
        '-Wl,--gc-sections,-Map=$(buildir)/out/air32f103.map,-cref,-u,Reset_Handler',
        '-T $(projectdir)/../../Libraries/STARTUP/gcc/linker/AIR32F103XB_FLASH.ld',
        {force = true}
    )

    add_includedirs(
        "$(projectdir)/../../Libraries/AIR32F10xLib/inc/",
        "$(projectdir)/../../Libraries/CMSIS/Include/",
        "$(projectdir)",
        "$(projectdir)/SYSTEM/"
    )
    add_files(
        "$(projectdir)/*.c",
        "$(projectdir)/SYSTEM/*.c",
        "$(projectdir)/../../Libraries/STARTUP/gcc/startup_air32f10x.s",
        "$(projectdir)/../../Libraries/AIR32F10xLib/src/air32f10x_gpio.c",
        "$(projectdir)/../../Libraries/AIR32F10xLib/src/air32f10x_rcc.c",
        "$(projectdir)/../../Libraries/AIR32F10xLib/src/air32f10x_rcc_ex.c",
        "$(projectdir)/../../Libraries/AIR32F10xLib/src/air32f10x_usart.c",
        "$(projectdir)/../../Libraries/AIR32F10xLib/src/misc.c",
        "$(projectdir)/../../Libraries/AIR32F10xLib/src/system_air32f10x.c"
    )

    after_build(
        function(target)
            sdk_dir = target:toolchains()[1]:sdkdir().."/"
            os.exec(sdk_dir.."bin/arm-none-eabi-objcopy -O ihex $(buildir)/out/air32f103.elf $(buildir)/out/air32f103.hex")
            os.exec(sdk_dir.."bin/arm-none-eabi-objcopy -O binary $(buildir)/out/air32f103.elf $(buildir)/out/air32f103.bin")
            os.exec(sdk_dir.."bin/arm-none-eabi-size $(buildir)/out/air32f103.elf")
        end
    )





