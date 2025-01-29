/* Including necessary configuration files. */
#include "Mcal.h"

#pragma GCC section text   ".basecode"
#pragma GCC section rodata ".basecodeentry"

void __attribute__((section(".basecode"))) quadcopy(uint64_t *dest, const uint64_t *src,
                                                           uint32_t wlen) {

    register uint64_t       *pDest = dest;
    register const uint64_t *pSrc  = src;
    register uint32_t        count = wlen;

    while (count > 0) {

        *pDest = *pSrc;

        ++pDest;
        ++pSrc;

        --count;
    }
}

void __attribute__((section(".basecode"))) quadzero(uint64_t *dest, uint32_t wlen) {

    register uint64_t *pDest = dest;
    register uint64_t  filldata =
#if defined(NDEBUG)
        0x0UL
#else
        0xFECAEDFEEFBEADDEUL
#endif
        ;
    register uint32_t count = wlen;

    while (count > 0) {
        *pDest = filldata;

        ++pDest;

        --count;
    }
}

typedef struct {
    void (*pfCopy)(uint64_t *dest, const uint64_t *src, uint32_t wlen);
    void (*pfZero)(uint64_t *dest, uint32_t wlen);
} RAMCodeEntry;

const RAMCodeEntry g_ramcode = {quadcopy, quadzero};

#pragma GCC section text
#pragma GCC section rodata
