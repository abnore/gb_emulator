# Organized after 3 last bits, not hex values
### xx yyy zzz
### z=0 Specials, sorted after y
NOP          0x00    00 000 000
LD [a16], SP 0x08    00 001 000
STOP n8      0x10    00 010 000
JR e8        0x18    00 011 000
JR NZ, e8    0x20    00 100 000
JR Z e8      0x28    00 101 000
JR NC, e8    0x30    00 110 000
JR C, e8     0x38    00 111 000

## 4th bit is internally dividing
### z=1 load/add n16      v
LD BC, n16   0x01    00 000 001
LD DE, n16   0x11    00 010 001
LD HL, n16   0x21    00 100 001
LD SP, n16   0x31    00 110 001
ADD HL, BC   0x09    00 001 001
ADD HL, DE   0x19    00 011 001
ADD HL, HL   0x29    00 101 001
ADD HL, SP   0x39    00 111 001

### z=2 load A to/from pointers
LD [BC], A   0x02    00 000 010
LD [DE], A   0x12    00 010 010
LD [HL+], A  0x22    00 100 010
LD [HL-], A  0x32    00 110 010
LD A, [BC]   0x0a    00 001 010
LD A, [DE]   0x1a    00 011 010
LD A, [HL+]  0x2a    00 101 010
LD A, [HL-]  0x3a    00 111 010

### z=3 inc/dec r16
INC BC       0x03    00 000 011
INC DE       0x13    00 010 011
INC HL       0x23    00 100 011
INC SP       0x33    00 110 011
DEC BC       0x0b    00 001 011
DEC DE       0x1b    00 011 011
DEC HL       0x2b    00 101 011
DEC SP       0x3b    00 111 011

### z=4 inc r8 sorted on y
INC B        0x04    00 000 100
INC C        0x0c    00 001 100
INC D        0x14    00 010 100
INC E        0x1c    00 011 100
INC H        0x24    00 100 100
INC L        0x2c    00 101 100
INC [HL]     0x34    00 110 100
INC A        0x3c    00 111 100

### z=5 dec r8 sorted on y
DEC B        0x05    00 000 101
DEC D        0x15    00 010 101
DEC H        0x25    00 100 101
DEC [HL]     0x35    00 110 101
DEC C        0x0d    00 001 101
DEC E        0x1d    00 011 101
DEC L        0x2d    00 101 101
DEC A        0x3d    00 111 101

### z=6 ld r8, n8 sorted on y
LD B, n8     0x06    00 000 110
LD C, n8     0x0e    00 001 110
LD D, n8     0x16    00 010 110
LD E, n8     0x1e    00 011 110
LD H, n8     0x26    00 100 110
LD L, n8     0x2e    00 101 110
LD [HL], n8  0x36    00 110 110
LD A, n8     0x3e    00 111 110

### z=7 rotate/flag, sorted on y
RLCA         0x07    00 000 111
RRCA         0x0f    00 001 111
RLA          0x17    00 010 111
RRA          0x1f    00 011 111
DAA          0x27    00 100 111
CPL          0x2f    00 101 111
SCF          0x37    00 110 111
CCF          0x3f    00 111 111
