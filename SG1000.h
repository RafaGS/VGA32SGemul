#ifndef SG1000_H
#define SG1000_H

#include "Z80.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SG1000_CART_SIZE  0xC000
#define SG1000_RAM_SIZE   0x0400

extern Z80 CPU;
extern byte CartROM[SG1000_CART_SIZE];
extern byte WorkRAM[SG1000_RAM_SIZE];
extern int CartSize;

int InitMachine(void);
void TrashMachine(void);
int LoadCartridge(const char *fileName);
void ResetSG1000(void);
int StartSG1000(void);

#ifdef __cplusplus
}
#endif
#endif