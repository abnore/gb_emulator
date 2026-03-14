# Organized after 3 last bits, not hex values
### z=0 LD into B, r8,r8
## 4th bit is internally dividing with 0 first row, 1 second:
### 0=B, 1=D, 2=H, 3=[HL]
### 0=C, 1=E, 2=L, 3=A
LD B, B      0x40    01 000 000
LD D, B      0x50    01 010 000
LD H, B      0x60    01 100 000
LD [HL], B   0x70    01 110 000
LD C, B      0x48    01 001 000
LD E, B      0x58    01 011 000
LD L, B      0x68    01 101 000
LD A, B      0x78    01 111 000

Same pattern for everyone, except 0x76 - "LD [HL],[HL]",  which is HLT!
