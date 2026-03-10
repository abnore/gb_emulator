#ifndef OPCODES_H_
#define OPCODES_H_

/* Popluating this list with all 240+ opcodes, is going to take a while
 * i will start with the ones needed, as shown in the running of the ROM,
 * and organize later */
enum{
    NOP = 0x00,
    JMP = 0xc3,
    XRA = 0xaf,
    LHL = 0x21,
    LDC = 0x0e,
    LDB = 0x06,
    LAH = 0x32,
};

#endif // OPCODES_H_
