#ifndef _DIFFUSIVE_H_
#define _DIFFUSIVE_H_
#include <math.h>
#include "libVVHD/core.h"

int InitDiffusive(Space *sS, double sReD);
int CalcVortexDiffusive();
int CalcHeatDiffusive();

#endif

