# Organized after 3 last bits, not hex values
## orderer after y internal, z for operands. ALU always uses Accumulator A:
0/8 1/9 2/a 3/b 4/c 5/d  6/e  7/f
 B,  C,  D,  E,  H,  L,  [HL], A

### xx yyy zzz
### z=0 is B
ADD A, B    0x80    10 000 000
ADC A, B    0x88    10 001 000
SUB A, B    0x90    10 010 000
SBC A, B    0x98    10 011 000
AND A, B    0xa0    10 100 000
XOR A, B    0xa8    10 101 000
OR  A, B    0xb0    10 110 000
CP  A, B    0xb8    10 111 000
