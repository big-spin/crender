#include "custom-types.h"

int LoadTexture(const char *path, Texture *tex);

#ifdef TEX_LOADER_IMPL

void AddPixelToTexture(uint8_t pixel[3], Texture *tex, int pixelIdx) {
        uint32_t pixel32 = (
                (pixel[0]) << 16 | (pixel[1]) << 8 | (pixel[2])
        );

        tex->data[pixelIdx] = pixel32;
}

int LoadTexture(const char *path, Texture *tex) {
        FILE *ptr = fopen(path, "rb");

        if (ptr == NULL) {
                printf("Error loading texture: (can't find '%s')\n", path);
                return 1;
        }

        char buffer[100];

        int isFileP6 = 0;
        int width, height;
        int maxColorValue;

        while (fgets(buffer, sizeof(buffer), ptr)) {
                if (strcmp(buffer, "P6\n") == 0) {
                        isFileP6 = 1;
                } else if (sscanf(buffer, " %d %d ", &width, &height) == 2) {
                        tex->width = width;
                        tex->height = height;
                } else if (sscanf(buffer, " %d ", &maxColorValue) == 1) {
                        tex->numOfColors = maxColorValue;
                        break;
                } 
        }

        if (isFileP6 == 0) {
                printf(
                        "Error loading texture: file '%s' isn't in the .ppm/P6 format",
                        path
                );
                return 1;
        }

        tex->data = (uint32_t*)(malloc(tex->width * tex->height * sizeof(uint32_t)));

        uint8_t pixel[3];
        size_t bytesRead;

        int expectedPixelCount = tex->width * tex->height;
        int actualPixelCount = 0;

        do {
                bytesRead = fread(&pixel, sizeof(uint8_t) * 3, 1, ptr);
                if (bytesRead != 0) {
                        //printf("Red: %u | Green: %u | Blue: %u\n", pixel[0], pixel[1], pixel[2]);
                        AddPixelToTexture(pixel, tex, actualPixelCount);
                        actualPixelCount++;
                }
        } while (bytesRead != 0);

        if (expectedPixelCount != actualPixelCount) {
                printf("Error loading texture: file '%s' has less pixels then expected", path);
                return 1;
        }

        /*
        printf(
                "Texture width:height: %d:%d\nTexture Max Color Value: %d\nExpected pixels vs received pixels: %d vs %d\n",
                tex->width, tex->height, tex->numOfColors, expectedPixelCount, actualPixelCount
        );
        */

        fclose(ptr);

        return 0;
}

#endif