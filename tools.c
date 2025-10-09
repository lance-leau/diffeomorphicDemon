#include "tools.h"

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
    img->width = (int)cols;
    img->height = (int)rows;
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
        dst[i] = (double)df->x[i]; // Ux
        dst[i + N] = (double)df->y[i]; // Uy
    }

    return out;
}

DispField *upsampleDispFieldToFull(const DispField *coarse, int W, int H)
{
    if (!coarse || !coarse->x || !coarse->y)
    {
        fprintf(stderr, "upsampleDispFieldToFull: null coarse field.\n");
        return NULL;
    }

    // Optional sanity check: your pipeline assumes scale=3 exactly.
    // This ensures the mapping x->x/3, y->y/3 used inside
    // interpolateDispField().
    if (W < 2 || H < 2)
    {
        fprintf(stderr, "upsampleDispFieldToFull: invalid target size.\n");
        return NULL;
    }

    // Allocate full-resolution displacement field
    DispField *full = (DispField *)calloc(1, sizeof(DispField));
    if (!full)
        return NULL;

    full->width = W;
    full->height = H;
    size_t N = (size_t)W * (size_t)H;

    full->x = (float *)calloc(N, sizeof(float));
    full->y = (float *)calloc(N, sizeof(float));
    if (!full->x || !full->y)
    {
        free(full->x);
        free(full->y);
        free(full);
        return NULL;
    }

    // IMPORTANT: reuse your exact sampling rule via interpolateDispField(),
    // which assumes the coarse grid is at 1/3 resolution and uses:
    //   gx = x/3, gy = y/3, fx = (x%3)/3, fy = (y%3)/3  (+ clamping)
    // This guarantees identical values to what warpImage() would fetch.
    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            // interpolateDispField signature is (DispField*, int, int)
            // It doesn't modify the field, so casting away const is safe here.
            DispVect v = interpolateDispField((DispField *)coarse, x, y);
            size_t idx = (size_t)y * (size_t)W + (size_t)x;
            full->x[idx] = v.x;
            full->y[idx] = v.y;
        }
    }

    return full;
}