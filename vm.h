
uint32_t data[32] = {0};
uint8_t dp = 0;

uint32_t rstack[32] = {0};
uint8_t rp = 0;

uint32_t disk[1024<<8] = {0};


uint32_t a = 0;
uint32_t ip = 0;

uint32_t word = 0;
uint8_t slot = 0;
uint16_t breakout[6] = {0};

#include "dbg.h"
#include "io.h"

#define ADDR(i) (word >> (((i)+1)*5))
#define TOS data[(dp-1)&0x1f]
#define NOS data[(dp-2)&0x1f]
#define PUSH(x) data[dp&0x1f] = (x);dp++
#define POP() TOS;dp--

#define TORS rstack[(rp-1)&0x1f]
#define NORS rstack[(rp-2)&0x1f]
#define RPUSH(x) rstack[rp&0x1f] = (x);rp++
#define RPOP() TORS;rp--

void vm_init() {
	io_init();
}

void memdump(uint32_t addr, uint32_t len) {
	for(uint8_t i = 0; i < len; i++) {
		printf("%016x ", disk[addr+i]);
	}
	printf("\n");
}

void fetch() {
	word = disk[ip++];
}

void decode() {
	uint32_t x = word;
	breakout[0] = x & 0x1f; x >>= 5;
	breakout[1] = x & 0x1f; x >>= 5;
	breakout[2] = x & 0x1f; x >>= 5;
	breakout[3] = x & 0x1f; x >>= 5;
	breakout[4] = x & 0x1f; x >>= 5;
	breakout[5] = x & 0x1f;
}

void execute() {
	uint32_t w;
//printf("w[%x]: %x %x %x %x %x %X\n", word, breakout[0], breakout[1], breakout[2], breakout[3], breakout[4], breakout[5]);
	while(slot < 6) {
//printf("ip[%d]sl[%d]op[%x]ts[%d][%d %d %d]rs[%d][%d %d %d %d]a[%d]\n", ip-1, slot, breakout[slot], dp, data[(dp-1)&0x1f], data[(dp-2)&0x1f], data[(dp-3)&0x1f], rp, rstack[(rp-1)&0x1f], rstack[(rp-2)&0x1f], rstack[(rp-3)&0x1f], rstack[(rp-4)&0x1f], a);
		switch(breakout[slot]) {
		case 0x00:
			slot++;
			break;
		case 0x01:
			slot = 6;
			w = ip;
			ip = TORS;
			TORS = w;
			break;
		case 0x02:
//printf("ju[%d]w[%x]sh[%d]\n",ADDR(slot), word, (((slot)+1)*5));
			ip = ADDR(slot);
			slot = 6;
			break;
		case 0x03:
//printf("ca[%d]w[%x]sh[%d]\n",ADDR(slot), word, (((slot)+1)*5));
			RPUSH(ip);
			ip = ADDR(slot);
			slot = 6;
			break;
		case 0x04:
			if(TORS) {
				TORS--;
				slot = 0;
				break;
			}
			slot++;
			rp--;
			break;
		case 0x05:
			if(TORS) {
				TORS--;
				ip = disk[ip];
				slot = 6;
				break;
			}
			slot++;
			rp--;
			ip++;
			break;
		case 0x06:
//printf("rs[%d]ds[%x]\n", rstack[rp&0x1f], data[dp&0x1f]);
			if(!TOS) {
				ip = disk[ip];
				slot = 6;
				break;
			}
			slot++;
			ip++;
			break;
		case 0x07:
			if((int32_t)(TOS) >= 0) {
				ip = disk[ip];
				slot = 6;
				break;
			}
			slot++;
			ip++;
			break;
		case 0x08:
			PUSH(disk[ip++]);
			slot++;
			break;
		case 0x09:
			PUSH(disk[a++]);
			slot++;
			break;
		case 0x0a:
			PUSH(disk[a]);
			slot++;
			break;
		case 0x0b:
			disk[ip++] = POP();
			slot++;
			break;
		case 0x0c:
			disk[a++] = POP();
			slot++;
			break;
		case 0x0d:
			disk[a] = POP();
			slot++;
			break;
		case 0x0e:
			PUSH(io_in[a]());
			slot++;
			break;
		case 0x0f:
			io_out[a](TOS);
			dp--;
			slot++;
			break;
		case 0x10:
			NOS *= POP();
			slot++;
			break;
		case 0x11:
			TOS <<= 1;
			slot++;
			break;
		case 0x12:
			TOS = (int32_t)(TOS) >> 1;
			slot++;
			break;
		case 0x13:
			TOS = ~TOS;
			slot++;
			break;
		case 0x14:
			NOS += POP();
			slot++;
			break;
		case 0x15:
			NOS &= POP();
			slot++;
			break;
		case 0x16:
			NOS ^= POP();
			slot++;
			break;
		case 0x17:
			dp--;
			slot++;
			break;
		case 0x18:
			PUSH(TOS);
			slot++;
			break;
		case 0x19:
			PUSH(TORS);
			rp--;
			slot++;
			break;
		case 0x1a:
			PUSH(NOS);
			slot++;
			break;
		case 0x1b:
			PUSH(a);
			slot++;
			break;
		case 0x1c:
			ip=RPOP();
			slot = 6;
			break;
		case 0x1d:
			RPUSH(TOS);
			dp--;
			slot++;
			break;
		case 0x1e:
//dmpstack();
//memdump(39, 3);
			io_exit();
			exit(0);
		case 0x1f:
			a = POP();
			slot++;
			break;
		}
	}
	slot = 0;
}

void cycle() {
	fetch();
	decode();
	execute();
}
