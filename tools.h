#ifndef TOOLS_H
#define TOOLS_H

#include "demon.h"
#include "mex.h"

Image* parseMatlabToImage(const mxArray* arr);
mxArray* parseImageToMatlab(const mxArray* arr);
mxArray* parseDispFieldToMatlab(const mxArray* arr);

#endif // TOOLS_H