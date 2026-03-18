### Block 3 seems like a random mess actually
# Organized after 3 last bits
### xx yyy zzz

The following opcodes are invalid, and hard-lock the CPU until the console is
powered off: $D3, $DB, $DD, $E3, $E4, $EB, $EC, $ED, $F4, $FC, and $FD.

### z=0 ret / ldh / sp-relative, sorted after y
RET NZ        0xC0    11 000 000
RET Z         0xC8    11 001 000
RET NC        0xD0    11 010 000
RET C         0xD8    11 011 000
LDH [a8], A   0xE0    11 100 000
ADD SP, e8    0xE8    11 101 000
LDH A, [a8]   0xF0    11 110 000
LD HL, SP+e8  0xF8    11 111 000

### z=1 pop / ret / jp hl / ld sp,hl, sorted after p and q
POP BC        0xC1    11 000 001
POP DE        0xD1    11 010 001
POP HL        0xE1    11 100 001
POP AF        0xF1    11 110 001
RET           0xC9    11 001 001
RETI          0xD9    11 011 001
JP HL         0xE9    11 101 001
LD SP, HL     0xF9    11 111 001

### z=2 jp cond / ldh / a16 loads, sorted after y
JP NZ, a16    0xC2    11 000 010
JP Z, a16     0xCA    11 001 010
JP NC, a16    0xD2    11 010 010
JP C, a16     0xDA    11 011 010
LDH [C], A    0xE2    11 100 010
LD [a16], A   0xEA    11 101 010
LDH A, [C]    0xF2    11 110 010
LD A, [a16]   0xFA    11 111 010

### z=3 misc / control, sorted after y
JP a16        0xC3    11 000 011
PREFIX        0xCB    11 001 011
—             0xD3    11 010 011  HARDLOCK
—             0xDB    11 011 011  HARDLOCK
—             0xE3    11 100 011  HARDLOCK
—             0xEB    11 101 011  HARDLOCK
DI            0xF3    11 110 011
EI            0xFB    11 111 011

### z=4 call cond, sorted after y
CALL NZ, a16  0xC4    11 000 100
CALL Z, a16   0xCC    11 001 100
CALL NC, a16  0xD4    11 010 100
CALL C, a16   0xDC    11 011 100
—             0xE4    11 100 100  HARDLOCK
—             0xEC    11 101 100  HARDLOCK
—             0xF4    11 110 100  HARDLOCK
—             0xFC    11 111 100  HARDLOCK

### z=5 push / call, divided by q and p
PUSH BC       0xC5    11 000 101
PUSH DE       0xD5    11 010 101
PUSH HL       0xE5    11 100 101
PUSH AF       0xF5    11 110 101
CALL a16      0xCD    11 001 101
—             0xDD    11 011 101  HARDLOCK
—             0xED    11 101 101  HARDLOCK
—             0xFD    11 111 101  HARDLOCK

### z=6 alu A, n8 sorted after y
ADD A, n8     0xC6    11 000 110
ADC A, n8     0xCE    11 001 110
SUB A, n8     0xD6    11 010 110
SBC A, n8     0xDE    11 011 110
AND A, n8     0xE6    11 100 110
XOR A, n8     0xEE    11 101 110
OR A, n8      0xF6    11 110 110
CP A, n8      0xFE    11 111 110

### z=7 rst vectors, sorted after y
RST $00       0xC7    11 000 111
RST $08       0xCF    11 001 111
RST $10       0xD7    11 010 111
RST $18       0xDF    11 011 111
RST $20       0xE7    11 100 111
RST $28       0xEF    11 101 111
RST $30       0xF7    11 110 111
RST $38       0xFF    11 111 111
