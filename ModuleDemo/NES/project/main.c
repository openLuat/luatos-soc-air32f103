#include <board.h>
#include "st7789.h"

#include "nes.h"

main(void){
    //不提供游戏本体
	// nes_t* nes = nes_load_rom(super_mario_rom);
	// if (!nes){
	// 	return -1;
	// }
	// nes_run(nes);

	while (1){
		rt_thread_mdelay(1000);
		rt_kprintf("rt-thread nano\n");
	}
}




