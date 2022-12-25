#ifndef AES_H
#define AES_H

void sboxAES(mask * mval);
void shiftRowsAES(maskState * mval);
void mixColumnsAES(mask * st0, mask * st1, mask * st2, mask * st3);
void nextRoundKeyAES(maskState * mval, uint8 rcon);

#endif
