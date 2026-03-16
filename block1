# Organized after 3 last bits, not hex values
### z=0 LD into B, r8,r8 ordered on y

LD B, B      0x40    01 000 000
LD C, B      0x48    01 001 000
LD D, B      0x50    01 010 000
LD E, B      0x58    01 011 000
LD H, B      0x60    01 100 000
LD L, B      0x68    01 101 000
LD [HL], B   0x70    01 110 000
LD A, B      0x78    01 111 000

LD B, C      0x41    01 000 001
LD C, C      0x49    01 001 001
LD D, C      0x51    01 010 001
LD E, C      0x59    01 011 001
LD H, C      0x61    01 100 001
LD L, C      0x69    01 101 001
LD [HL], C   0x71    01 110 001
LD A, C      0x79    01 111 001

LD B, D      0x42    01 000 010
LD C, D      0x4a    01 001 010
LD D, D      0x52    01 010 010
LD E, D      0x5a    01 011 010
LD H, D      0x62    01 100 010
LD L, D      0x6a    01 101 010
LD [HL], D   0x72    01 110 010
LD A, E      0x7a    01 111 010

HLT          0x76    01 110 110
Same pattern for everyone, except 0x76 - "LD [HL],[HL]",  which is HLT!

Storing a register into itself is a no-op; however, some Game Boy emulators
interpret LD B,B as a breakpoint, or LD D,D as a debug message (such as BGB).


