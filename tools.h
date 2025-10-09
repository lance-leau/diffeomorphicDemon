#ifndef TOOLS_H
#define TOOLS_H

#include "demon.h"
#include "mex.h"

Image *parseMatlabToImage(const mxArray *arr);
mxArray *parseImageToMatlab(const Image *img);
mxArray *parseDispFieldToMatlab(const DispField *df);
DispField *upsampleDispFieldToFull(const DispField *coarse, int W, int H);

#endif // TOOLS_H