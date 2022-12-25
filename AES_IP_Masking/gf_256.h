#ifndef GF_256_H
#define GF_256_H

void gf256Multiply(uint8 * res, uint8 a, uint8 b);
void gf256Square(uint8 * res, uint8 a);
void gf256Inverse(uint8 * res, uint8 a);

#endif
