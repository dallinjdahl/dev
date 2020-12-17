#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

char cbuf[256] = {0};
FILE *inf, *of;

uint32_t word = 0;
uint8_t breakout[6] = {0};
uint16_t ip = 0;
uint16_t line = 1;

char *ops = \
".." "\x00" "@p" "\x08" "**" "\x10" "du" "\x18"\
"ex" "\x01" "@+" "\x09" "2*" "\x11" "po" "\x19"\
"ju" "\x02" "@@" "\x0a" "2/" "\x12" "ov" "\x1a"\
"ca" "\x03" "!p" "\x0b" "--" "\x13" "aa" "\x1b"\
"un" "\x04" "!+" "\x0c" "++" "\x14" ";;" "\x1c"\
"nx" "\x05" "!!" "\x0d" "an" "\x15" "pu" "\x1d"\
"if" "\x06" "in" "\x0e" "or" "\x16" "xx" "\x1e"\
"-i" "\x07" "ot" "\x0f" "dr" "\x17" "a!" "\x1f";

uint8_t fill() {
	return !!fgets(cbuf, 256, inf);
}

void usage() {
	fputs("Usage: fas <input file>\n", stderr);
	exit(0);
}

void err(char *s) {
	fprintf(stderr, "Error [%s] on line %d: %s\n", s, line, cbuf);
	exit(1);
}

uint8_t getop(char *s) {
	char *t = ops;
	while(*t) {
		if(*s == *t && s[1] == t[1]) {
			return t[2];
		}
		t += 3;
	}
	err("op");
	return 0xff;
}

unsigned char lbuf[2048] = {0};
uint16_t lbp = 0;

void printref() {
	unsigned char *s = lbuf;
	while(s < lbuf+lbp) {
		while(*s && *s != '\n') {
			printf("%c", *(s++));
		}
		s++;
		printf("\t%d\n", *s);
		s++;
	}
}

void label() {
	uint16_t lstart = lbp;
	for(char *s = cbuf+2; *s && *s ^ '\n'; s++) {
		lbuf[lbp++] = *s;
	}
	lbuf[lbp++] = '\0';
	lbuf[lbp++] = ip&0xff;
	lbuf[lbp++] = ip >> 8;
	printf("label %s at %d:%d\n", lbuf+lstart, line, ip);
}

uint32_t getref(char *t) {
//printref();
	char *s = t;
	unsigned char *ls = lbuf;
	uint16_t res = 0;
	while(ls < lbuf+lbp) {
		for(s = t; *s && *s ^ '\n' && *s == *ls; s++, ls++); //compare as far as possible
		if(!*ls  && (!*s || *s == '\n')) { 
			ls++;
//			printf("ref %d at %d\n", *ls , line);
			res += *(ls++);
			res += (*ls) << 8;
			return res;
		}
		while(*(ls++)); //skip to end of word
		ls++; //skip reference
		ls++;
	}
}

void inst() {
	word = 0;
	uint32_t addr = 0;
	char *s = cbuf+2;
	for(int i = 0; i < 6 && *s && *s ^ '\n'; i++) {
		uint8_t o = getop(s);
		fprintf(stderr, "op: %x\n", o);
		word += o <<(i*5);
		s += 2;

		if(o != 2 && o != 3 ) continue;
		s++;
		i++;
		addr = getref(s);
		if(addr != (addr << (i*5)) >> (i*5)) {
			printf("E%d: too long jump: %s:%d\n", line, s, addr);
			exit(1);
		}
		word += addr <<(i*5);
		break;
	}
	fwrite(&word, 4, 1, of);
	fprintf(stderr, "w: %x\n", word);
}

void hex() {
	word = 0;
	uint8_t b = 0;
	for(char *s = cbuf+2; *s && *s ^ '\n'; s++) {
		if(*s < '0' || *s > 'f' ||
			(*s > '9' && *s < 'a')) err("hex");

		word <<= 4;
		b = (*s)-'0';
		if(b > 10) {
			b -= ('a'-'9')-1;
		}
		word += b;
	}
	fwrite(&word, 4, 1, of);
}

uint16_t getskip() {
	uint16_t num = 0;
	char *s = cbuf+2;
	for(; *s && *s ^ '\n'; s++) {
		if(*s < '0' || *s > '9') err("skip");
		num *= 10;
		num += *s-'0';
	}
	return num;
}

void dec() {
	word = 0;
	int8_t mult = 1;
	char *s = cbuf+2;
	if (cbuf[2] == '-') {
		mult = -1;
		s++;
	}
	for(; *s && *s ^ '\n'; s++) {
		if(*s < '0' || *s > '9') err("dec");
		word *= 10;
		word += *s-'0';
	}
	word *= mult;
	fwrite(&word, 4, 1, of);
}

void ref() {
	word = getref(cbuf+2);
	fwrite(&word, 4, 1, of);
//	printf("ref %d at %d\n", word, line);
	return;
}

void labelize() {
	while(fill()) {
//printf("labelc: %c\n", cbuf[0]);
		switch(cbuf[0]) {
		case '+':
			ip += getskip()-1;
			break;
		case ':':
			label();
		case ']':
		case ' ':
		case '\n':
		case '\t':
			ip--;
			break;
		}
		line++;
		ip++;
	}
}

void chars() {
	char *s = cbuf+2;
	uint8_t cslot = 0;
	char c = 0;
	word = 0;
	while(*s && *s^'\n' && cslot < 4) {
		c = *s;
		if(c == '\\') {
			s++;
			switch(*s) {
			case 'n':
				c = '\n';
				break;
			case 't':
				c = '\t';
				break;
			case 's':
				c = ' ';
				break;
			default:
				c = *s;
			}
		}
		s++;
		word += ((uint32_t)c) << (8*cslot++);
	}
	fwrite(&word, 4, 1, of);
}

void str() {
	char s[64] = {0};
	char *t = 0;
	uint32_t size = 0;
	char c = 0;
	for(char *i = s, *t = cbuf+3; i < s+64 && *t != '"'; i++, t++) {
		char c = *t;
		if(c == '\\') {
			t++;
			switch(*t) {
			case 'n':
				c = '\n';
				break;
			case 't':
				c = '\t';
				break;
			case 's':
				c = ' ';
				break;
			default:
				c = *t;
			}
		}
		*i = c;
		size++;
	}
	size += 3;
	size >>= 2;
	size--;
	word = size;
	fwrite(&word, 4, 1, of);
	t = s;
	while(*t) {
		word = 0;
		for(uint8_t i = 0; i < 4 && *t; i++) {
			c = *(t++);
			word += ((uint32_t) c) << (8*i);
		}
		fwrite(&word, 4, 1, of);
	}
}

void skip() {
	word = 0;
	uint16_t num = 0;
	char *s = cbuf+2;
	for(; *s && *s ^ '\n'; s++) {
		if(*s < '0' || *s > '9') err("skip");
		num *= 10;
		num += *s-'0';
	}
	while(num) {
		fwrite(&word, 4, 1, of);
		num--;
	}
}
	
int main(int argc, char **argv) {
	if(argc < 2) usage();
	inf = fopen(argv[1], "r");
	labelize();
	rewind(inf);
	line = 1;
	ip = 0;
	of = fopen("block.img", "wb");
	while(fill()) {
//printf("mainc: %c\n", cbuf[0]);
		switch(cbuf[0]) {
		case 'i':
			inst();
			break;
		case 'x':
			hex();
			break;
		case 'd':
			dec();
			break;
		case 'c':
			chars();
			break;
		case 's':
			str();
			break;
		case 'r':
			ref();
			break;
		case '+':
			skip();
			break;
		case ':':
		case ']':
		case ' ':
		case '\n':
		case '\t':
			ip--;
			break;
		default:
			err("p0");
		}
		line++;
		ip++;
	}
	return 0;
}
