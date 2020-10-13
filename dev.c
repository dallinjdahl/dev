#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "vm.h"


int main(int argc, char **argv) {
	vm_init();
	while(1) {
		cycle();
	}
	return 0;
}
