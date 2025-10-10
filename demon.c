
#include "demon.h"

#include "tools.h"

void normalizeImage(Image *img)
{
    float minVal = img->data[0];
    float maxVal = img->data[0];

    for (int i = 1; i < img->width * img->height; i++)
    {
        if (img->data[i] < minVal)
            minVal = img->data[i];
        if (img->data[i] > maxVal)
            maxVal = img->data[i];
    }

    for (int i = 0; i < img->width * img->height; i++)
    {
        img->data[i] = (img->data[i] - minVal) / (maxVal - minVal);
    }
}

DispField *initDispField(int width, int height, int stepSize)
{
    DispField *df = calloc(1, sizeof(DispField));
    df->width = width;
    df->height = height;
    df->x = calloc(df->width * df->height, sizeof(float));
    df->y = calloc(df->width * df->height, sizeof(float));
    df->stepSize = stepSize;
    return df;
}

void interpolateDispField(DispField *df)
{
    int W = df->width;
    int H = df->height;
    int s = df->stepSize;

    for (int y = 0; y < H; ++y)
    {
        int y0 = (y / s) * s;
        int y1 = y0 + s;
        if (y1 >= H)
            y1 = H - 1;

        float fy0 = (float)(y - y0) / (float)(y1 - y0);
        float fy1 = 1.0f - fy0;

        for (int x = 0; x < W; ++x)
        {
            int x0 = (x / s) * s;
            int x1 = x0 + s;
            if (x1 >= W)
                x1 = W - 1;
            float fx0 = (float)(x - x0) / (float)(x1 - x0);
            float fx1 = 1.0f - fx0;

            float u00 = df->x[y0 * W + x0];
            float v00 = df->y[y0 * W + x0];

            float u10 = df->x[y0 * W + x1];
            float v10 = df->y[y0 * W + x1];

            float u01 = df->x[y1 * W + x0];
            float v01 = df->y[y1 * W + x0];

            float u11 = df->x[y1 * W + x1];
            float v11 = df->y[y1 * W + x1];

            float u = (fx1 * fy1 * u00) + (fx0 * fy1 * u10) + (fx1 * fy0 * u01)
                + (fx0 * fy0 * u11);

            float v = (fx1 * fy1 * v00) + (fx0 * fy1 * v10) + (fx1 * fy0 * v01)
                + (fx0 * fy0 * v11);

            df->x[y * W + x] = u;
            df->y[y * W + x] = v;
        }
    }
}

void freeImage(Image *img)
{
    if (img)
        free(img->data);
    free(img);
}

void freeDispField(DispField *df)
{
    if (df)
    {
        free(df->x);
        free(df->y);
    }
    free(df);
}

void warpImage(Image *moving, DispField *df)
{
    Image *warped = calloc(1, sizeof(Image));
    warped->width = moving->width;
    warped->height = moving->height;
    warped->data = calloc(warped->width * warped->height, sizeof(float));

    for (int y = 0; y < moving->height; y++)
    {
        for (int x = 0; x < moving->width; x++)
        {
            int srcX = x + (int)vect.x;
            int srcY = y + (int)vect.y;

            if (srcX < 0)
                srcX = 0;
            if (srcX >= moving->width)
                srcX = moving->width - 1;
            if (srcY < 0)
                srcY = 0;
            if (srcY >= moving->height)
                srcY = moving->height - 1;

            warped->data[y * warped->width + x] =
                moving->data[srcY * moving->width + srcX];
        }
    }

    free(moving->data);
    moving->data = warped->data;
    free(warped);
}

// TODO maybe switch from square kernel to circular kernel (maybe gaussian ???)
float compareBlockSSD(Image *fixed, Image *moving, int fixedX, int fixedY,
                      int movingX, int movingY, int blockSize)
{
    int rad = blockSize / 2;
    float ssd = 0.0f;

    for (int j = -rad; j <= rad; j++)
    {
        for (int i = -rad; i <= rad; i++)
        {
            int fx = fixedX + i;
            int fy = fixedY + j;
            int mx = movingX + i;
            int my = movingY + j;

            // skip if out of bounds
            if (fx < 0 || fy < 0 || fx >= fixed->width || fy >= fixed->height)
                continue;
            if (mx < 0 || my < 0 || mx >= moving->width || my >= moving->height)
                continue;

            float diff = fixed->data[fy * fixed->width + fx]
                - moving->data[my * moving->width + mx];
            ssd += diff * diff;
        }
    }

    return ssd;
}

void estimateBlockDisps(Image *fixed, Image *moving, DispField *df,
                        int searchRadius)
{
    int step = df->stepSize;

    for (int gy = 0; gy < df->height; gy += step)
    {
        for (int gx = 0; gx < df->width; gx += step)
        {
            int fixedX = gx;
            int fixedY = gy;

            float minSSD = FLT_MAX; // const max flaot in std lib
            int bestDx = 0;
            int bestDy = 0;

            for (int dy = -searchRadius; dy <= searchRadius; dy++)
            {
                for (int dx = -searchRadius; dx <= searchRadius; dx++)
                {
                    int movingX = fixedX + dx;
                    int movingY = fixedY + dy;

                    float ssd = compareBlockSSD(fixed, moving, fixedX, fixedY,
                                                movingX, movingY, df->stepSize);
                    if (ssd < minSSD)
                    {
                        minSSD = ssd;
                        bestDx = dx;
                        bestDy = dy;
                    }
                }
            }

            df->x[gy * df->width + gx] += bestDx;
            df->y[gy * df->width + gx] += bestDy;
        }
    }
}

void demonsRegistration(Image *fixed, Image *moving, DispField *df,
                        int numLevels, int numIters, float sigmaI, float sigmaX)
{
    // TODO blur and normalize
    for (int iter = 0; iter < numIters; iter++)
    {
        printf("Starting iteration %d/%d\n", iter + 1, numIters);
        fflush(stdout);
        // TODO maybe add a pyramid lvl step size like in matlab
        estimateBlockDisps(fixed, moving, df, df->stepSize, 5);
        warpImage(moving, df);
        // TODO free warped
    }
}

/*
D_total = zeros(W, H);
M_warped = copy(M);

for iter in range(numIters):

    // (a) Compute incremental field
    dD = estimateDisplacement(F, M_warped);

    // (b) Smooth for regularization
    gaussianSmooth(dD, sigmaX);

    // (c) Accumulate
    addFields(D_total, dD);

    // (d) Warp the original image (or incrementally)
    M_warped = warp(M, D_total);
end
*/