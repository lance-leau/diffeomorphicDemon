#ifndef TOOLS_H
#define TOOLS_H

#include "demon.h"
#include "mex.h"


#define DEBUG_PRINT(var) \
    do { \
        _Generic((var), \
            int:                printf("[DEBUG] %s:%d | int %s = %d\n", __FILE__, __LINE__, #var, var), \
            unsigned int:       printf("[DEBUG] %s:%d | unsigned int %s = %u\n", __FILE__, __LINE__, #var, var), \
            long:               printf("[DEBUG] %s:%d | long %s = %ld\n", __FILE__, __LINE__, #var, var), \
            unsigned long:      printf("[DEBUG] %s:%d | unsigned long %s = %lu\n", __FILE__, __LINE__, #var, var), \
            \
            float:              printf("[DEBUG] %s:%d | float %s = %f\n", __FILE__, __LINE__, #var, var), \
            double:             printf("[DEBUG] %s:%d | double %s = %f\n", __FILE__, __LINE__, #var, var), \
            long double:        printf("[DEBUG] %s:%d | long double %s = %Lf\n", __FILE__, __LINE__, #var, var), \
            \
            char:               printf("[DEBUG] %s:%d | char %s = '%c'\n", __FILE__, __LINE__, #var, var), \
            char*:              printf("[DEBUG] %s:%d | char* %s = \"%s\"\n", __FILE__, __LINE__, #var, var), \
            const char*:        printf("[DEBUG] %s:%d | const char* %s = \"%s\"\n", __FILE__, __LINE__, #var, var), \
            \
            void*:              printf("[DEBUG] %s:%d | void* %s = %p\n", __FILE__, __LINE__, #var, var), \
            \
            default:            printf("[DEBUG] %s:%d | pointer %s = %p\n", __FILE__, __LINE__, #var, (void*)var) \
        ); \
    } while (0)

Image *parseMatlabToImage(const mxArray *arr);
mxArray *parseImageToMatlab(const Image *img);
mxArray *parseDispFieldToMatlab(const DispField *df);
DispField *upsampleDispFieldToFull(const DispField *coarse, int W, int H);

size_t clampSizetDiff(size_t a, size_t b);
size_t safeSizetIntAddition(size_t a, int b);

#endif // TOOLS_H