# dev
This is my personal development environment, based on chuck Moore's
colorforth and the retroforth language.

## Design
dev is designed to run on a portable virtual machine, allowing for either native or hosted platforms.  To this end, it has a minimal instruction set and port-mapped io, allowing for platform specific extensions.  each opcode is 5 bits long and 6 opcodes are packed into a single 32 bit word.

## ops
