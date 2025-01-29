#if !defined(BASECODE_H)
#define BASECODE_H (1)

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    void (*pfCopy)(uint64_t *dest, const uint64_t *src, uint32_t wlen);
    void (*pfZero)(uint64_t *dest, uint32_t wlen);
} RAMCodeEntry;

extern const RAMCodeEntry g_ramcode;

typedef enum {
	Boot_Application = (uint8_t)0,
	Boot_Bootloader = (uint8_t)1,

	Boot_Max
} Boot_Type;

typedef struct {
	uint8_t resetReason;
	uint8_t bootType;
} EData;

#if defined(__cplusplus)
}
#endif

#endif
