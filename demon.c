
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
    df->width = width;
    df->height = height;
    df->x = calloc(width * height, sizeof(float));
    df->y = calloc(width * height, sizeof(float));
    return df;
}

// TODO maybe switch from square kernel to circular kernel (maybe gaussian ???)
DispVect *compareBlockSSD(Image *fixed, Image *moving, int fixedX, int fixedY,
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

            if (fx < 0 || fy < 0 || fx >= fixed->width || fy >= fixed->height)
                continue;
            if (mx < 0 || my < 0 || mx >= moving->width || my >= moving->height)
                continue;

            float diff = fixed->data[fy * fixed->width + fx] - moving->data[my * moving->width + mx];
            ssd += (diff * diff);
        }
    }
    DispVect *vect = calloc(1, sizeof(DispVect));
    vect->x = (float)(movingX - fixedX);
    vect->y = (float)(movingY - fixedY);

    return vect;
}

DispVect interpolateDispField(DispField *df, int x, int y)
{
    int gx = x / 3;
    int gy = y / 3;
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

    float u = (1 - fx) * (1 - fy) * u00 + fx * (1 - fy) * u10 + (1 - fx) * fy * u01 + fx * fy * u11;
    float v = (1 - fx) * (1 - fy) * v00 + fx * (1 - fy) * v10 + (1 - fx) * fy * v01 + fx * fy * v11;
    return (DispVect){.x = u, .y = v};
}

Image *warpImage(Image *moving, DispField *df)
{
    Image *warped = calloc(1, sizeof(Image));
    warped->width = moving->width;
    warped->height = moving->height;
    warped->data = calloc(warped->width * warped->height, sizeof(float));

    for (size_t y = 0; y < moving->height; y++)
    {
        for (size_t x = 0; x < moving->width; x++)
        {
            DispVect vect = interpolateDispField(df, x, y);

            if (x + (int)vect.x >= moving->width)
                vect.x = 0;
            if (y + (int)vect.y >= moving->height)
                vect.y = 0;

            warped->data[y * warped->width + x] = moving->data[(y + (int)vect.y) * warped->width + (x + (int)vect.x)];
        }
    }

    // TODO free warped
    return warped;
}

void demonsRegistration(Image *fixed, Image *moving, DispField *df,
                        int numLevels, int numIters, float sigmaI, float sigmaX)
{
}