
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

DispField *initDispField(int width, int height)
{
    DispField *df = calloc(1, sizeof(DispField));
    df->width = (width + 2) / 3;
    df->height = (height + 2) / 3;
    df->x = calloc(df->width * df->height, sizeof(float));
    df->y = calloc(df->width * df->height, sizeof(float));
    return df;
}

DispVect interpolateDispField(DispField *df, int x, int y)
{
    int gx = x / 3;
    int gy = y / 3;
    if (gx >= df->width - 1)
        gx = df->width - 2;
    if (gy >= df->height - 1)
        gy = df->height - 2;

    float fx = (x % 3) / 3.0f;
    float fy = (y % 3) / 3.0f;

    float u00 = df->x[gy * df->width + gx];
    float v00 = df->y[gy * df->width + gx];
    float u01 = df->x[gy * df->width + (gx + 1)];
    float v01 = df->y[gy * df->width + (gx + 1)];
    float u10 = df->x[(gy + 1) * df->width + gx];
    float v10 = df->y[(gy + 1) * df->width + gx];
    float u11 = df->x[(gy + 1) * df->width + (gx + 1)];
    float v11 = df->y[(gy + 1) * df->width + (gx + 1)];

    float u = (1 - fx) * (1 - fy) * u00 + fx * (1 - fy) * u10
        + (1 - fx) * fy * u01 + fx * fy * u11;
    float v = (1 - fx) * (1 - fy) * v00 + fx * (1 - fy) * v10
        + (1 - fx) * fy * v01 + fx * fy * v11;
    return (DispVect){ .x = u, .y = v };
}

void freeImage(Image *img)
{
    if (img)
    {
        free(img->data);
        free(img);
    }
}

void freeDispField(DispField *df)
{
    if (df)
    {
        free(df->x);
        free(df->y);
        free(df);
    }
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
            DispVect vect = interpolateDispField(df, x, y);

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
                        int blockSize, int searchRadius)
{
    for (int gy = 0; gy < df->height; ++gy)
    {
        int mY = gy * 3;
        for (int gx = 0; gx < df->width; ++gx)
        {
            int mX = gx * 3;
            float currBestSSD = -1.0f;
            for (int dy = -searchRadius; dy <= searchRadius; dy++)
            {
                for (int dx = -searchRadius; dx <= searchRadius; dx++)
                {
                    int fX = mX + dx;
                    int fY = mY + dy;

                    if (fX < 0 || fY < 0 || fX >= fixed->width
                        || fY >= fixed->height)
                        continue;

                    float ssd = compareBlockSSD(fixed, moving, fX, fY, mX, mY,
                                                blockSize);
                    if (currBestSSD == -1 || ssd < currBestSSD)
                    {
                        currBestSSD = ssd;
                        int idx = gy * df->width + gx;
                        df->x[idx] = (float)dx;
                        df->y[idx] = (float)dy;
                    }
                }
            }
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
        estimateBlockDisps(fixed, moving, df, 32, 5);
        warpImage(moving, df);
        // TODO free warped
    }
}

/*
function demonsRegistration(fixed, moving, df, numLevels, numIters, sigmaI,
sigmaX): for iteration in [0 .. numIters-1]: # 1. Estimate local block
displacements for each grid cell (gx, gy): bestDisp =
findDisplacementBySSD(fixed, moving, gx, gy, searchRadius) df.x[gx, gy] =
bestDisp.x df.y[gx, gy] = bestDisp.y

        # 2. Smooth / regularize the displacement field
        smoothDispField(df, sigmaX)

        # 3. Warp moving image
        moving = warpImage(originalMoving, df)
    end

    return df
*/