#ifndef DEMON_H
#define DEMON_H

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    int width;
    int height;
    float *data; // in [0,1]
} Image;

typedef struct
{
    // displacement field is computed at 1/stepSize resolution and interpolated
    // to scale
    int width;
    int height;
    float *x;
    float *y;
    int stepSize; // in pixels
} DispField;

typedef struct
{
    float x;
    float y;
} DispVect; // in pixels

// Initialization
DispField *initDispField(int width, int height, int stepSize);

// Preprocessing
void normalizeImage(Image *img);

// Block comparison
float compareBlockSSD(Image *fixed, Image *moving, int fixedX, int fixedY,
                      int movingX, int movingY, int blockSize);

// Disp estimation
void estimateBlockDisps(Image *fixed, Image *moving, DispField *df,
                        int searchRadius);

// Warping
Image *warpImage(Image *moving, DispField *df);

// Iteration
void demonsRegistration(Image *fixed, Image *moving, DispField *df,
                        int numIters);

void interpolateDispField(DispField *df);

void freeImage(Image *img);
void freeDispField(DispField *df);

#endif // DEMON_H