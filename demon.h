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
    // displacement field one third the size of the image
    int width;
    int height;
    float *x;
    float *y;
} DispField;

typedef struct
{
    float x;
    float y;
} DispVect; // in pixels

// Initialization
DispField *initDispField(int width, int height);

// Preprocessing
void normalizeImage(Image *img);
void smoothImage(Image *img, float sigma);

// Block comparison
float compareBlockSSD(Image *fixed, Image *moving, int fixedX, int fixedY,
                      int movingX, int movingY, int blockSize);
float computeBlockNCC(Image *fixed, Image *moving, int x, int y, int dx, int dy,
                      int blockSize);

// Disp estimation
void estimateBlockDisps(Image *fixed, Image *moving, DispField *df,
                        int blockSize, int searchRadius);

// Regularization
void smoothDispField(DispField *df, float sigma);

// Warping
Image *warpImage(Image *moving, DispField *df);

// Iteration
void demonsRegistration(Image *fixed, Image *moving, DispField *df,
                        int blockSize, int searchRadius, int nIterations,
                        float smoothSigma);

#endif // DEMON_H