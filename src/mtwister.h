#ifndef __MTWISTER_H
#define __MTWISTER_H

#include <stdio.h>

#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397 /* changes to STATE_VECTOR_LENGTH also require changes to this */

typedef struct tagMTRand {
  unsigned long mt[STATE_VECTOR_LENGTH];
  int index;
} MTRand;

MTRand seedRand(unsigned long seed);
unsigned long genRandLong(MTRand* rand);
double genRand(MTRand* rand);

/* fixed-point representation */
unsigned long genRandFixed(MTRand *rand);
unsigned long fpoint(double x);

void serializeRand(FILE *file, const MTRand *rand);
void deserializeRand(FILE *file, MTRand *rand);

#endif /* #ifndef __MTWISTER_H */
