#include "demon.h"
#include "mex.h"
#include "tools.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    mexPrintf("1\n");
    mexEvalString("drawnow;");

    if (nrhs < 2)
        mexErrMsgTxt("Usage: [warped, field] = diffeoDemon(fixed, moving)");
    if (nlhs > 2)
        mexErrMsgTxt("Too many output arguments.");

    mexPrintf("2\n");
    mexEvalString("drawnow;");

    Image *fixed = parseMatlabToImage(prhs[0]);
    Image *moving = parseMatlabToImage(prhs[1]);

    mexPrintf("3\n");
    mexEvalString("drawnow;");

    normalizeImage(fixed);
    normalizeImage(moving);

    mexPrintf("4\n");
    mexEvalString("drawnow;");

    DispField *df = initDispField(fixed->width, fixed->height, 5);

    mexPrintf("5\n");
    mexEvalString("drawnow;");

    mexPrintf("Fixed: %dx%d | Moving: %dx%d\n", fixed->width, fixed->height,
              moving->width, moving->height);
    mexEvalString("drawnow;");
    demonsRegistration(fixed, moving, df, 10);

    mexPrintf("Registration done, warping image...\n");
    mexEvalString("drawnow;");
    warpImage(moving, df);

    mexPrintf("6\n");
    mexEvalString("drawnow;");

    // TODO free the everything
    // TODO parse fixed and moving to matlab compatible structs and return

    plhs[0] = parseDispFieldToMatlab(df);
    plhs[1] = parseImageToMatlab(moving);

    mexPrintf("7\n");
    mexEvalString("drawnow;");

    return;
}