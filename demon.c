#include "demon.h"

#include <float.h>
#include <math.h>
#include <string.h>

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

Image *warpImage(Image *moving, DispField *df)
{
    int W = moving->width;
    int H = moving->height;

    Image *warped = calloc(1, sizeof(Image));
    warped->width = W;
    warped->height = H;
    warped->data = calloc(W * H, sizeof(float));

    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            // Read displacement at this pixel
            float dx = df->x[y * W + x];
            float dy = df->y[y * W + x];

            // Backward mapping: find source position
            float srcX = x - dx;
            float srcY = y - dy;

            // Clamp coordinates
            if (srcX < 0)
                srcX = 0;
            if (srcY < 0)
                srcY = 0;
            if (srcX > W - 2)
                srcX = W - 2;
            if (srcY > H - 2)
                srcY = H - 2;

            // Bilinear interpolation
            int x0 = (int)srcX;
            int y0 = (int)srcY;
            float wx = srcX - x0;
            float wy = srcY - y0;

            float I00 = moving->data[y0 * W + x0];
            float I10 = moving->data[y0 * W + (x0 + 1)];
            float I01 = moving->data[(y0 + 1) * W + x0];
            float I11 = moving->data[(y0 + 1) * W + (x0 + 1)];

            float val = (1 - wx) * (1 - wy) * I00 + wx * (1 - wy) * I10
                + (1 - wx) * wy * I01 + wx * wy * I11;

            warped->data[y * W + x] = val;
        }
    }

    return warped;
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
            // TODO if fixed[x, y] and moving[x, y] are equal, stop warping
            if (compareBlockSSD(fixed, moving, gx, gy, gx, gy, BLOCK_SIZE)
                <= 0.0f)
                continue;

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
                                                movingX, movingY, BLOCK_SIZE);
                    if (ssd < minSSD)
                    {
                        minSSD = ssd;
                        bestDx = dx;
                        bestDy = dy;
                    }
                }
            }

            df->x[gy * df->width + gx] -= bestDx;
            df->y[gy * df->width + gx] -= bestDy;
        }
    }
    interpolateDispField(df);
}

Image *copyImage(Image *src)
{
    Image *ret = calloc(1, sizeof(Image));
    ret->width = src->width;
    ret->height = src->height;
    ret->data = calloc(src->height * src->width, sizeof(float));

    for (int i = 0; i < src->height * src->width; i++)
        ret->data[i] = src->data[i];

    return ret;
}

void sumDispFields(DispField *D_tot, DispField *D_iter)
{
    int W = D_tot->width;
    int H = D_tot->height;

    float *newX = calloc(W * H, sizeof(float));
    float *newY = calloc(W * H, sizeof(float));

    for (int y = 0; y < H; y++)
    {
        for (int x = 0; x < W; x++)
        {
            int cur = y * W + x;

            int a = x - (int)(D_iter->x[cur]);
            int b = y - (int)(D_iter->y[cur]);

            if (a < 0)
                a = 0;
            if (a >= W)
                a = W - 1;
            if (b < 0)
                b = 0;
            if (b >= H)
                b = H - 1;

            newX[cur] = D_iter->x[cur] + D_tot->x[b * W + a];
            newY[cur] = D_iter->y[cur] + D_tot->y[b * W + a];
        }
    }

    memcpy(D_tot->x, newX, W * H * sizeof(float));
    memcpy(D_tot->y, newY, W * H * sizeof(float));

    free(newX);
    free(newY);
}

void sumDispFields_legacy(DispField *D_tot, DispField *D_iter)
{
    for (int i = 0; i < D_tot->height * D_tot->width; i++)
    {
        D_tot->x[i] -= D_iter->x[i];
        D_tot->y[i] -= D_iter->y[i];
    }
}

void matrixMultiplication(DispField *D_tot, DispField *D_iter)
{
    int H = D_tot->height;
    int W = D_iter->width;
    int N = D_tot->width; // shared dimension

    // Result buffers
    float *newX = calloc(H * W, sizeof(float));
    float *newY = calloc(H * W, sizeof(float));

    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            float sumX = 0.0f;
            float sumY = 0.0f;
            for (int k = 0; k < N; k++)
            {
                sumX += D_tot->x[i * N + k] * D_iter->x[k * W + j];
                sumY += D_tot->y[i * N + k] * D_iter->y[k * W + j];
            }
            newX[i * W + j] = sumX;
            newY[i * W + j] = sumY;
        }
    }

    // Store result back in D_tot
    free(D_tot->x);
    free(D_tot->y);
    D_tot->x = newX;
    D_tot->y = newY;

    // Update new width/height
    D_tot->width = W;
    D_tot->height = H;
}

void demonsRegistration(Image *fixed, Image *moving, DispField *D_tot,
                        int numIters)
{
    // TODO blur and normalize
    Image *moving_i = copyImage(moving);
    for (int iter = 0; iter < numIters; iter++)
    {
        printf("Starting iteration %d/%d\n", iter + 1, numIters);
        fflush(stdout);

        printf("estimating displacements...");
        fflush(stdout);

        DispField *D_iter =
            initDispField(fixed->width, fixed->height, D_tot->stepSize);
        estimateBlockDisps(fixed, moving_i, D_iter, SEARCH_RADIUS);

        printf("done!\n");
        fflush(stdout);

        Image *newMoving = warpImage(moving_i, D_iter);
        freeImage(moving_i);
        moving_i = newMoving;

        sumDispFields(D_tot, D_iter);
        freeDispField(D_iter);
    }
    Image *test = copyImage(moving);
    saveImagePGM(warpImage(test, D_tot), "MORPHED.pgm");
    free(moving->data);
    moving->data = moving_i->data;
    free(moving_i);
}

void saveImagePGM(const Image *img, const char *filename)
{
    FILE *fp = fopen(filename, "wb"); // 'wb' for "write binary"
    if (!fp)
    {
        printf("Error: Could not open file %s for writing.\n", filename);
        return;
    }

    // Write PGM header
    // P5 is the magic number for binary grayscale
    // Width Height
    // Maxval
    fprintf(fp, "P5\n%d %d\n255\n", img->width, img->height);

    int numPixels = img->width * img->height;
    unsigned char *buffer =
        (unsigned char *)malloc(numPixels * sizeof(unsigned char));

    if (!buffer)
    {
        printf("Error: Could not allocate memory for image buffer.\n");
        fclose(fp);
        return;
    }

    // Convert float data [0, 1] to unsigned char [0, 255]
    for (int i = 0; i < numPixels; i++)
    {
        float val = img->data[i];
        if (val < 0.0f)
            val = 0.0f;
        if (val > 1.0f)
            val = 1.0f;
        buffer[i] = (unsigned char)(val * 255.0f);
    }

    // Write pixel data to file
    fwrite(buffer, sizeof(unsigned char), numPixels, fp);

    // Cleanup
    fclose(fp);
    free(buffer);
}