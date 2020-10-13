
uint32_t data[32] = {0};
uint8_t dp = 0;

uint32_t rstack[32] = {0};
uint8_t rp = 0;

uint32_t disk[1024<<8] = {0};

#include "io.h"

uint32_t a = 0;
uint32_t ip = 0;

uint32_t word = 0;
uint8_t slot = 0;
uint16_t breakout[6] = {0};

#include "dbg.h"

#define ADDR(i) (word >> (((i)+1)*5))
void vm_init() {
	io_init();
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
	while(slot < 6) {
//printf("ip[%d]sl[%d]op[%d]ts[%d]\n", ip-1, slot, breakout[slot], data[dp&0x1f]);
		switch(breakout[slot]) {
		case 0x00:
			slot++;
			break;
		case 0x01:
			slot = 6;
			w = ip;
			ip = rstack[rp];
			rstack[rp] = w;
			break;
		case 0x02:
//printf("ju[%d]w[%x]sh[%d]\n",ADDR(slot), word, (((slot)+1)*5));
			ip = ADDR(slot);
			slot = 6;
			break;
		case 0x03:
//printf("ca[%d]w[%x]sh[%d]\n",ADDR(slot), word, (((slot)+1)*5));
			rstack[(++rp)&0x1f] = ip;
			ip = ADDR(slot);
			slot = 6;
			break;
		case 0x04:
			if(rstack[rp&0x1f]) {
				rstack[rp&0x1f]--;
				slot = 0;
				break;
			}
			slot++;
			rp--;
			break;
		case 0x05:
			if(rstack[rp&0x1f]) {
				rstack[rp&0x1f]--;
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
			if(!data[dp&0x1f]) {
				ip = disk[ip];
				slot = 6;
				break;
			}
			slot++;
			ip++;
			break;
		case 0x07:
			if(data[dp&0x1f] >= 0) {
				ip = disk[ip];
				slot = 6;
				break;
			}
			slot++;
			ip++;
			break;
		case 0x08:
			data[(++dp)&0x1f] = disk[ip++];
			slot++;
			break;
		case 0x09:
			data[(++dp)&0x1f] = disk[a++];
			slot++;
			break;
		case 0x0a:
			data[(++dp)&0x1f] = disk[a];
			slot++;
			break;
		case 0x0b:
			disk[ip++] = data[(dp--)&0x1f];
			slot++;
			break;
		case 0x0c:
			disk[a++] = data[(dp--)&0x1f];
			slot++;
			break;
		case 0x0d:
			disk[a] = data[(dp--)&0x1f];
			slot++;
			break;
		case 0x0e:
			data[(++dp)&0x1f] = io_in[a]();
			slot++;
			break;
		case 0x0f:
			io_out[a](data[(dp--)&0x1f]);
			slot++;
			break;
		case 0x10:
			data[(dp-1)&0x1f] *= data[dp&0x1f];
			dp--;
			slot++;
			break;
		case 0x11:
			data[dp&0x1f] <<= 1;
			slot++;
			break;
		case 0x12:
			data[dp&0x1f] = ((int32_t)data[dp&0x1f]) >> 1;
			slot++;
			break;
		case 0x13:
			data[dp&0x1f] = ~data[dp&0x1f];
			slot++;
			break;
		case 0x14:
			data[(dp-1)&0x1f] += data[dp&0x1f];
			dp--;
			slot++;
			break;
		case 0x15:
			data[(dp-1)&0x1f] &= data[dp&0x1f];
			dp--;
			slot++;
			break;
		case 0x16:
			data[(dp-1)&0x1f] ^= data[dp&0x1f];
			dp--;
			slot++;
			break;
		case 0x17:
			dp--;
			slot++;
			break;
		case 0x18:
			data[(dp+1)&0x1f] = data[dp&0x1f];
			dp++;
			slot++;
			break;
		case 0x19:
			data[(++dp)&0x1f] = rstack[(rp--)&0x1f];
			slot++;
			break;
		case 0x1a:
			data[(dp+1)&0x1f] = data[(dp-1)&0x1f];
			dp++;
			slot++;
			break;
		case 0x1b:
			data[(++dp)&0x1f] = a;
			slot++;
			break;
		case 0x1c:
			ip=rstack[(rp--)&0x1f];
			slot = 6;
			break;
		case 0x1d:
			rstack[(++rp)&0x1f] = data[(dp--)&0x1f];
			slot++;
			break;
		case 0x1e:
			io_exit();
			exit(0);
		case 0x1f:
			a = data[(dp--)&0x1f];
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
