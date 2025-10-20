#ifndef DEMON_H
#define DEMON_H

#include <stddef.h>


#define STEP_SIZE 15
#define BLOCK_SIZE 15
#define SEARCH_RADIUS 10
#define NUM_ITERS 10

typedef struct
{
    size_t width;
    size_t height;
    float *data; // in [0,1]
} Image;

typedef struct
{
    // displacement field is computed at 1/stepSize resolution and interpolated
    // to scale
    size_t width;
    size_t height;
    float *x;
    float *y;
    size_t stepSize; // in pixels
} DispField;

typedef struct
{
    float x;
    float y;
} DispVect; // in pixels

// Initialization
DispField *initDispField(size_t width, size_t height, size_t stepSize);

// Preprocessing
void normalizeImage(Image *img);

// Block comparison
float compareBlockSSD(Image *fixed, Image *moving, size_t fixedX, size_t fixedY,
                      size_t movingX, size_t movingY, size_t blockSize);

// Disp estimation
void estimateBlockDisps(Image *fixed, Image *moving, DispField *df,
                        int searchRadius);

// Warping
Image *warpImage(Image *moving, DispField *df);

// Iteration
void demonsRegistration(Image *fixed, Image *moving, DispField *df,
                        int numIters);

void interpolateDispField(DispField *df);

void saveImagePGM(const Image *img, const char *filename);

void freeImage(Image *img);
void freeDispField(DispField *df);

#endif // DEMON_H