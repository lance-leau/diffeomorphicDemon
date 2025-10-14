#include "tools.h"

#include <stddef.h>

Image *parseMatlabToImage(const mxArray *arr)
{
    if (!mxIsNumeric(arr) || mxIsComplex(arr))
        mexErrMsgTxt("Input must be a real numeric array.");

    size_t ndims = mxGetNumberOfDimensions(arr);
    if (ndims != 2)
        mexErrMsgTxt("Only 2D grayscale images are supported.");

    mwSize rows = mxGetM(arr);
    mwSize cols = mxGetN(arr);
    mwSize numel = rows * cols;

    /* Create Image struct */
    Image *img = calloc(1, sizeof(Image));
    img->width = (size_t)cols;
    img->height = (size_t)rows;
    img->data = (float *)calloc(numel, sizeof(float));

    /* Copy data depending on MATLAB input type */
    if (mxIsDouble(arr))
    {
        const double *src = (const double *)mxGetData(arr);
        for (mwSize i = 0; i < numel; i++)
            img->data[i] = (float)src[i];
    }
    else if (mxIsSingle(arr))
    {
        const float *src = (const float *)mxGetData(arr);
        for (mwSize i = 0; i < numel; i++)
            img->data[i] = src[i];
    }
    else if (mxIsUint8(arr))
    {
        const unsigned char *src = (const unsigned char *)mxGetData(arr);
        for (mwSize i = 0; i < numel; i++)
            img->data[i] = src[i] / 255.0f;
    }
    else
    {
        mexErrMsgTxt("Unsupported input type. Use double, single, or uint8.");
    }

    return img;
}

mxArray *parseImageToMatlab(const Image *img)
{
    if (!img || !img->data)
        mexErrMsgTxt("Invalid Image pointer.");

    mwSize rows = img->height;
    mwSize cols = img->width;
    mxArray *out = mxCreateDoubleMatrix(rows, cols, mxREAL);
    double *dst = mxGetPr(out);

    mwSize numel = rows * cols;
    for (mwSize i = 0; i < numel; i++)
        dst[i] = (double)img->data[i];

    return out;
}

mxArray *parseDispFieldToMatlab(const DispField *df)
{
    if (!df || !df->x || !df->y)
        mexErrMsgTxt("Invalid DispField pointer.");

    mwSize dims[3] = { df->height, df->width, 2 };
    mxArray *out = mxCreateNumericArray(3, dims, mxDOUBLE_CLASS, mxREAL);
    double *dst = mxGetPr(out);

    mwSize N = df->width * df->height;
    for (mwSize i = 0; i < N; i++)
    {
        dst[i] = -(double)df->y[i];
        dst[i + N] = -(double)df->x[i];
    }
    return out;
}

// Performs a - b
// @return size_t   0       if a - b < 0
//                  a - b   else
size_t clampSizetDiff(size_t a, size_t b)
{
    if (a < b)
        return 0;

    return a - b;
}

size_t safeSizetIntAddition(size_t a, int b)
{
    if (b >= 0)
    {
        if ((size_t)b > SIZE_MAX - a)
            return SIZE_MAX; // Overflow
    }
    else
    {
        if ((size_t)-b > a)
            return 0; // Underflow
    }

    return a + (size_t)b;
}
