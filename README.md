# dev
This is my personal development environment, based on chuck Moore's
colorforth and the retroforth language.

## Design
dev is designed to run on a portable virtual machine, allowing for either native or hosted platforms.  To this end, it has a minimal instruction set and port-mapped io, allowing for platform specific extensions.  each opcode is 5 bits long and 6 opcodes are packed into a single 32 bit word.

## ops

| opcode | mnemonic | description |
|--------|----------|-------------|
| 00	 | ..		| nop |
| 01	 | ex		| exchange ip and top of return stack |
| 02	 | ju		| jump to address in remaining bits of word |
| 03	 | ca		| call address in remaining bits of word |
| 04	 | un		| (unext) loop over single instruction word as long as TORS != 0, decrementing TORS |
| 05	 | nx		| (next) jump to address in next word and decrement TORS if TORS != 0 |
| 06	 | if		| jump to address in next word if TOS != 0 |
| 07	 | -i		| (-if) jump to address in next word if TOS >= 0 |
| 08	 | @p		| (fetch-pc) push next word to TOS |
| 09	 | @+		| (fetch-plus) push word addressed by A register, and increment A register |
| 0a	 | @@		| (fetch) push word addressed by A register |
| 0b	 | !p		| (store-pc) store TOS in the following word |
| 0c	 | !+		| (store-plus) store TOS at address in A register, increment A register |
| 0d	 | !!		| (store) store TOS at address in A register |
| 0e	 | in		| read port addressed by A register |
| 0f	 | ot		| (out) write TOS to port addressed by A register |
| 10	 | \*\*		| multiply top 2 items on the stack and push the result |
| 11	 | 2\*		| left shift TOS |
| 12	 | 2/		| arithmetic right shift TOS |
| 13	 | --		| (invert) bitwise invert TOS |
| 14	 | ++		| add top 2 items on the stack and push the result |
| 15	 | an		| (and) push the bitwise and of the top 2 items of the stack |
| 16	 | or		| push the bitwise or of the top 2 items of the stack |
| 17	 | dr		| drop the TOS |
| 18	 | du		| duplicate the TOS |
| 19	 | po		| pop TORS to TOS |
| 1a	 | ov		| (over) push a copy second element on the stack to the top |
| 1b	 | aa		| push value of A register |
| 1c	 | ;;		| return from call |
| 1d	 | pu		| push TOS to return stack |
| 1e	 | xx		| exit |
| 1f	 | a!		| store TOS to A register |
