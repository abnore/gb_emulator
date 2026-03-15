# Organized after 3 last bits, not hex values
### xx yyy zzz
### z=0 Specials, sorted after y
### STOP: The opcode of this instruction is $10, but it has to be followed by an
### additional byte that is ignored by the CPU (any value works, but normally $00
### is used).
NOP          0x00    00 000 000     4
LD [a16], SP 0x08    00 001 000     20
STOP n8      0x10    00 010 000     4 - Has to be followed by a byte, ignored
JR e8        0x18    00 011 000     12
JR NZ, e8    0x20    00 100 000     8/12 depends on cc
JR Z e8      0x28    00 101 000     8/12 depends on cc
JR NC, e8    0x30    00 110 000     8/12 depends on cc
JR C, e8     0x38    00 111 000     8/12 depends on cc

## 4th bit is internally dividing
### z=1 load/add n16      v
LD BC, n16   0x01    00 000 001     12
LD DE, n16   0x11    00 010 001     12
LD HL, n16   0x21    00 100 001     12
LD SP, n16   0x31    00 110 001     12
ADD HL, BC   0x09    00 001 001     8
ADD HL, DE   0x19    00 011 001     8
ADD HL, HL   0x29    00 101 001     8
ADD HL, SP   0x39    00 111 001     8

### z=2 load A to/from pointers
LD [BC], A   0x02    00 000 010     8
LD [DE], A   0x12    00 010 010     8
LD [HL+], A  0x22    00 100 010     8
LD [HL-], A  0x32    00 110 010     8
LD A, [BC]   0x0a    00 001 010     8
LD A, [DE]   0x1a    00 011 010     8
LD A, [HL+]  0x2a    00 101 010     8
LD A, [HL-]  0x3a    00 111 010     8

### z=3 inc/dec r16
INC BC       0x03    00 000 011     8
INC DE       0x13    00 010 011     8
INC HL       0x23    00 100 011     8
INC SP       0x33    00 110 011     8
DEC BC       0x0b    00 001 011     8
DEC DE       0x1b    00 011 011     8
DEC HL       0x2b    00 101 011     8
DEC SP       0x3b    00 111 011     8

### z=4 inc r8 sorted on y
INC B        0x04    00 000 100     4
INC C        0x0c    00 001 100     4
INC D        0x14    00 010 100     4
INC E        0x1c    00 011 100     4
INC H        0x24    00 100 100     4
INC L        0x2c    00 101 100     4
INC [HL]     0x34    00 110 100     12
INC A        0x3c    00 111 100     4

### z=5 dec r8 sorted on y
DEC B        0x05    00 000 101     4
DEC C        0x0d    00 001 101     4
DEC D        0x15    00 010 101     4
DEC E        0x1d    00 011 101     4
DEC H        0x25    00 100 101     4
DEC L        0x2d    00 101 101     4
DEC [HL]     0x35    00 110 101     12
DEC A        0x3d    00 111 101     4

### z=6 ld r8, n8 sorted on y
LD B, n8     0x06    00 000 110     8
LD C, n8     0x0e    00 001 110     8
LD D, n8     0x16    00 010 110     8
LD E, n8     0x1e    00 011 110     8
LD H, n8     0x26    00 100 110     8
LD L, n8     0x2e    00 101 110     8
LD [HL], n8  0x36    00 110 110     12
LD A, n8     0x3e    00 111 110     8

### z=7 rotate/flag, sorted on y
RLCA         0x07    00 000 111     4
RRCA         0x0f    00 001 111     4
RLA          0x17    00 010 111     4
RRA          0x1f    00 011 111     4
DAA          0x27    00 100 111     4
CPL          0x2f    00 101 111     4
SCF          0x37    00 110 111     4
CCF          0x3f    00 111 111     4
