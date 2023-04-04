#include <board.h>

int main(void){
	while (1){
		rt_thread_mdelay(1000);
		rt_kprintf("rt-thread nano\n");
	}
}

