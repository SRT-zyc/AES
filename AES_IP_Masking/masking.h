#ifndef MASKING_H
#define MASKING_H

void initMasks(mask * mval, uint8 * L, uint8 * L2);
void copyMasks(mask * mout, mask * min);

void refreshMask(mask * mout, mask * min);
void refreshArrays(uint8 * Lout, uint8 * Rout, uint8 * Lin, uint8 * Rin, int size);

void halfEncode(mask * mval, uint8 val);
void encodeByte(mask * mval, uint8 val);
void decodeByte(uint8 * val, mask * mval);

void maskAddition(mask * out, mask * op1, mask * op2);
void maskSquaring(mask * out, mask * op);
void maskSquaring2(mask * out, mask * op);
void maskSquaring4(mask * out, mask * op);
void maskMultiplication(mask * out, mask * op1, mask * op2);
void maskMultiplication2(mask * out, mask * op1, mask * op2);
void maskMultConst(mask * out, mask * op1, uint8 op2);
void maskAddConst(mask * mval, mask * op1, uint8 val);

#endif
