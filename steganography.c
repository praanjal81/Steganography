#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1) // Avoid structure padding
typedef struct {
    unsigned short type;        // Signature: 0x4D42
    unsigned int size;          // File size in bytes
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;        // Offset to image data in bytes
} BMPHeader;

typedef struct {
    unsigned int size;          // Header size in bytes
    int width;
    int height;
    unsigned short planes;
    unsigned short bitsPerPixel;
    unsigned int compression;
    unsigned int imageSize;
    int xPixelsPerMeter;
    int yPixelsPerMeter;
    unsigned int colorsUsed;
    unsigned int importantColors;
} DIBHeader;
#pragma pack(pop)

void encode(char *inputFile, char *outputFile, char *message) {
    FILE *in = fopen(inputFile, "rb");
    if (!in) { printf("Error: Cannot open %s\n", inputFile); return; }

    FILE *out = fopen(outputFile, "wb");
    if (!out) { printf("Error: Cannot create %s\n", outputFile); fclose(in); return; }

    BMPHeader bmp;
    DIBHeader dib;

    fread(&bmp, sizeof(BMPHeader), 1, in);
    fread(&dib, sizeof(DIBHeader), 1, in);

    if (bmp.type != 0x4D42 || dib.bitsPerPixel != 24) {
        printf("Error: Only 24-bit BMP files are supported.\n");
        fclose(in); fclose(out);
        return;
    }

    // Copy headers to output
    fwrite(&bmp, sizeof(BMPHeader), 1, out);
    fwrite(&dib, sizeof(DIBHeader), 1, out);

    // Copy pixel data into a buffer
    int dataSize = dib.imageSize;
    if (dataSize == 0) dataSize = bmp.size - bmp.offset;
    unsigned char *data = (unsigned char *)malloc(dataSize);
    fread(data, 1, dataSize, in);

    // Prepare message: store length first
    int msgLen = strlen(message);
    int totalBits = (msgLen + 4) * 8; // +4 bytes to store length

    if (totalBits > dataSize) {
        printf("Error: Image not large enough for message.\n");
        free(data); fclose(in); fclose(out);
        return;
    }

    // Store message length in first 32 bits
    for (int i = 0; i < 32; i++) {
        int bit = (msgLen >> i) & 1;
        data[i] = (data[i] & 0xFE) | bit;
    }

    // Store message bits
    for (int i = 0; i < msgLen; i++) {
        unsigned char ch = message[i];
        for (int b = 0; b < 8; b++) {
            int bit = (ch >> b) & 1;
            data[32 + i * 8 + b] = (data[32 + i * 8 + b] & 0xFE) | bit;
        }
    }

    // Write modified pixel data
    fwrite(data, 1, dataSize, out);

    free(data);
    fclose(in);
    fclose(out);

    printf("Message successfully encoded into %s\n", outputFile);
}

void decode(char *inputFile) {
    FILE *in = fopen(inputFile, "rb");
    if (!in) { printf("Error: Cannot open %s\n", inputFile); return; }

    BMPHeader bmp;
    DIBHeader dib;

    fread(&bmp, sizeof(BMPHeader), 1, in);
    fread(&dib, sizeof(DIBHeader), 1, in);

    if (bmp.type != 0x4D42 || dib.bitsPerPixel != 24) {
        printf("Error: Only 24-bit BMP files are supported.\n");
        fclose(in);
        return;
    }

    int dataSize = dib.imageSize;
    if (dataSize == 0) dataSize = bmp.size - bmp.offset;
    unsigned char *data = (unsigned char *)malloc(dataSize);
    fread(data, 1, dataSize, in);

    // Retrieve message length
    int msgLen = 0;
    for (int i = 0; i < 32; i++) {
        int bit = data[i] & 1;
        msgLen |= (bit << i);
    }

    // Retrieve message
    char *message = (char *)malloc(msgLen + 1);
    for (int i = 0; i < msgLen; i++) {
        unsigned char ch = 0;
        for (int b = 0; b < 8; b++) {
            int bit = data[32 + i * 8 + b] & 1;
            ch |= (bit << b);
        }
        message[i] = ch;
    }
    message[msgLen] = '\0';

    printf("Decoded message: %s\n", message);

    free(message);
    free(data);
    fclose(in);
}

int main() {
    int choice;
    char inputFile[100], outputFile[100], message[256];
    printf("---------steganography--------------\n");
    printf("1. Encode\n2. Decode: ");
    printf("\nEnter your choice: ");
    scanf("%d", &choice);
    getchar(); // clear newline

    if (choice == 1) {
        printf("Enter input BMP filename: ");
        gets(inputFile);
        printf("Enter output BMP filename: ");
        gets(outputFile);
        printf("Enter message to hide: ");
        gets(message);
        encode(inputFile, outputFile, message);
    }
    else if (choice == 2) {
             printf("Enter BMP filename to decode: ");
        gets(inputFile);
        decode(inputFile);
        
    }
    else {
        printf("Invalid choice.\n");
    }

    return 0;
}

