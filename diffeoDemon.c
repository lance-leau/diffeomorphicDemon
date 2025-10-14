#include "demon.h"
#include "mex.h"
#include "tools.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nrhs < 2)
        mexErrMsgTxt("Usage: [warped, field] = diffeoDemon(fixed, moving)");
    if (nlhs > 2)
        mexErrMsgTxt("Too many output arguments.");

    Image *fixed = parseMatlabToImage(prhs[0]);
    Image *moving = parseMatlabToImage(prhs[1]);

    normalizeImage(fixed);
    normalizeImage(moving);

    DispField *df = initDispField(fixed->width, fixed->height, STEP_SIZE);

    mexPrintf("Fixed: %dx%d | Moving: %dx%d\n", fixed->width, fixed->height,
              moving->width, moving->height);
    mexEvalString("drawnow;");
    demonsRegistration(fixed, moving, df, NUM_ITERS);

    warpImage(moving, df);

    // TODO free the everything
    // TODO parse fixed and moving to matlab compatible structs and return

    plhs[0] = parseDispFieldToMatlab(df);
    plhs[1] = parseImageToMatlab(moving);

    return;
}