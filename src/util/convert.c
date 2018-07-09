//Using SDL and standard IO
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Set up STB_IMAGE #define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

int main(int argc, char* args[])
{
  // font notes
  // Characters are in ascii order
  // 5 wide by 7 tall
  // one row blank above
  // one row below
  // one column either size (treat it as a size of 8 advanced by 1 to give kerning)
  // ascii character starts with SPACE, dec 32.

  // Basic usage (see HDR discussion below for HDR usage):
//    int x,y,n;
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
//    // ... process data if not NULL ...
//    // ... x = width, y = height, n = # 8-bit components per pixel ...
//    // ... replace '0' with '1'..'4' to force that many components per pixel
//    // ... but 'n' will always be the number that it would have been if you said 0
//    stbi_image_free(data)
//
// Standard parameters:
//    int *x                 -- outputs image width in pixels
//    int *y                 -- outputs image height in pixels
//    int *channels_in_file  -- outputs # of image components in image file
//    int desired_channels   -- if non-zero, # of image components requested in result

  int32_t imageWidth, imageHeight, channels;
  uint8_t* image = stbi_load("charmap2.png", &imageWidth, &imageHeight, &channels, STBI_grey);
  if (image == NULL) {
    printf("error");
    return EXIT_FAILURE;
  }


  /*
  uint8_t* ptr = image;
  // advance by a row
  ptr += imageWidth;
  int imageRow = 0;
  int x;

  // 18 by 6 glyphs

  while (imageRow < imageHeight) {
    x = 0;
    while (x < imageWidth) {
      uint8_t pixel = *ptr;
      char* c = (pixel == 0) ? " ": "#";
      printf("%c", *c);

      ptr = ptr + 1;
      x += 1;

    }
    imageRow += 1;
    printf("\n");

  }
  */
  int gW = 18;
  int gH = 6;

  int gY = 0;
  int gX = 1;
  /*
   for (int gY = 0; gY < gH; gY++) {
     for (int gX = 0; gX < gW; gX++) {
      for (int j = 0; j < 7; j++) {
        // traverse a row
        for (int i = 0; i < 5; i++) {
          uint8_t pixel = image[1+ imageWidth + gX * 7 + j * imageWidth + i + (imageWidth * (7+2) * gY)];
          char* c = (pixel == 0) ? " ": "#";
          printf("%s", c);
        }
        printf("\n");
      }
      printf("---%d,%d\n", gX, gY);
    }
  }
   */
  printf("char font[%d][%d] = {\n", gW*gH, 5*7);
   for (int gY = 0; gY < gH; gY++) {
     for (int gX = 0; gX < gW; gX++) {
       printf("  {\n");
      for (int j = 0; j < 7; j++) {
        printf("    ");
        // traverse a row
        for (int i = 0; i < 5; i++) {
          uint8_t pixel = image[1+ imageWidth + gX * 7 + j * imageWidth + i + (imageWidth * (7+2) * gY)];
          char* c = (pixel == 0) ? "0x00": "0xFF";
          printf("%s, ", c);
        }
        printf("\n");
      }
      printf("  },\n");
    }
  }
   printf("};\n");
  stbi_image_free(image);
  return EXIT_SUCCESS;
}



