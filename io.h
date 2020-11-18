#define IONUM 4
 
uint32_t io_mem[IONUM] = {0};

FILE *block_file;

void io_init() {
	block_file = fopen("block.img", "r+b");
	fseek(block_file, 0, SEEK_SET);
	fread(disk, 255, 1024, block_file);
	fflush(block_file);
}

void io_exit() {
	fclose(block_file);
}

#define IOSTORE(n, x) \
void n ## _out(uint32_t n) {\
	io_mem[x] = n;\
}\
uint32_t n ## _in() {\
	return io_mem[x];\
}

uint32_t console_in() {
	uint32_t res = 0;
	res += getc(stdin); 
	return res;
}

void console_out(uint32_t c) {
	putc(c&0xff,stdout);
}

IOSTORE(block, 1)
IOSTORE(disk_addr, 2)

void disk_out(uint32_t addr) {
	fseek(block_file, io_mem[1]<<10, SEEK_SET);
	fwrite(disk+addr, 4, 256, block_file);
	fflush(block_file);
}

uint32_t disk_in() {
	uint32_t res = 0;
	fseek(block_file, io_mem[1]<<10, SEEK_SET);
	fread(disk+io_mem[2], 4, 256, block_file);
	fflush(block_file);
	for(int i = io_mem[2]; i < io_mem[2]+256; i++) {
		res += disk[i];
	}
	return res;
}

uint32_t dbgstack() {
printf("ip[%d]sl[%d]op[%x]ts[%d][%d %d %d]rs[%d][%d %d %d %d]a[%d]\n", ip-1, slot, breakout[slot], dp, data[(dp-1)&0x1f], data[(dp-2)&0x1f], data[(dp-3)&0x1f], rp, rstack[(rp-1)&0x1f], rstack[(rp-2)&0x1f], rstack[(rp-3)&0x1f], rstack[(rp-4)&0x1f], a);
	return dp;
}

uint32_t (*io_in[IONUM])()= {console_in, block_in, disk_addr_in, dbgstack};
void (*io_out[IONUM])(uint32_t) = {console_out, block_out, disk_addr_out};
