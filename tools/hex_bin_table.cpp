#include <cstdio>
#include <cstdint>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '-'), \
  ((byte) & 0x40 ? '1' : '-'), \
  ((byte) & 0x20 ? '1' : '-'), \
  ((byte) & 0x10 ? '1' : '-'), \
  ((byte) & 0x08 ? '1' : '-'), \
  ((byte) & 0x04 ? '1' : '-'), \
  ((byte) & 0x02 ? '1' : '-'), \
  ((byte) & 0x01 ? '1' : '-')

int main(int argc, char *argv[]) {
    for (uint16_t low = 0; low <= 0x1F; ++low) {
        for (uint16_t hi = 0; hi <= 0xE0; hi += 0x20) {
            uint8_t val = hi + low;
            std::printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(val));
            std::putchar(' ');
        }
        std::putchar('\n');
    }
    return 0;
}
