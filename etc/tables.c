#include <stdint.h>
#include <stdio.h>

typedef uint8_t byte;

static uint8_t parity[256];
static uint8_t sz53[256];
static uint8_t sz53p[256];

static const byte BIT_F_CARRY = 0x01;
static const byte BIT_F_NEG = 0x02;
static const byte BIT_F_PARITY = 0x04;
static const byte BIT_F_3 = 0x08;
static const byte BIT_F_HALF = 0x10;
static const byte BIT_F_5 = 0x20;
static const byte BIT_F_ZERO = 0x40;
static const byte BIT_F_SIGN = 0x80;

int main() {
    int i, j, k;
    byte p;
    FILE* f;

    for (i = 0; i < 256; i++) {
        sz53[i] = (byte)(i & (BIT_F_3 | BIT_F_5 | BIT_F_SIGN));
        j = i; p = 0;
        for (k = 0; k < 8; k++) { p ^= (byte)(j & 1); j >>= 1; }
        parity[i] = (byte)(p > 0 ? 0 : BIT_F_PARITY);
        sz53p[i] = (byte)(sz53[i] | parity[i]);
    }

    sz53[0] |= (byte)BIT_F_ZERO;
    sz53p[0] |= (byte)BIT_F_ZERO;

    f = fopen("parity.bin", "wb");
    fwrite(parity, 1, 256, f);
    fclose(f);

    f = fopen("sz53.bin", "wb");
    fwrite(sz53, 1, 256, f);
    fclose(f);

    f = fopen("sz53p.bin", "wb");
    fwrite(sz53p, 1, 256, f);
    fclose(f);

    return 0;
}
