void dmpstack() {
	printf("dp: %d\n",dp);
	for(uint8_t i = 0; i < 8; i++) {
		for(uint8_t j = 0; j < 4; j++) {
			printf("[%02d][0x%08x] ",(i<<3)+j,data[(i<<3)+j]);
		}
		printf("\n");
	}
}

void optrace() {
	printf("ip[%02d] sl[%02d] op[0x%02x]\n",ip,slot, breakout[slot]);
}
