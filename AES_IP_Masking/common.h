#ifndef COMMON_H
#define COMMON_H

typedef unsigned char uint8;

typedef struct {
   uint8	L[n];
   uint8	R[n];
   uint8 L2[2*n];
} mask;

typedef struct {
   mask 	st[16];
} maskState;

#endif
