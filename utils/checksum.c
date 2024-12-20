#include <stdint.h>
#include <stdio.h>

uint8_t checksum(uint8_t *data, size_t len)
{
    uint8_t checksum = 0;

    printf("Payload:");
    for (int i = 0; i < len; i++) {
        printf(" %02x", data[i]);
        checksum += data[i];
    }
    printf("\n");

    return 0x55 - checksum;
}

uint8_t example1[] = {0xaa, 0x03, 0x0c, 0x00, 0x00};

uint8_t expected1 = 0x9c;

uint8_t example2[] = {0xAA, 0x1D, 0x08, 0x00, 0x0B, 0x41, 0x69, 0x72, 0x36, 0x30, 0x20, 0x42, 0x54, 0x35, 0x2E, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

uint8_t expected2 = 0xb0;

int main()
{
    uint8_t checksum1 = checksum(example1, sizeof(example1) / sizeof(example1[0]));
    printf("Checksum: %02x %s\n", checksum1, checksum1 == expected1 ? "OK" : "FAIL");
    uint8_t checksum2 = checksum(example2, sizeof(example2) / sizeof(example2[0]));
    printf("Checksum: %02x %s\n", checksum2, checksum2 == expected2 ? "OK" : "FAIL");
}
