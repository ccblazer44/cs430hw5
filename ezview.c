#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

#include "linmath.h"

#include <assert.h>

#define RGB_NUMBER 255


typedef struct RGBpixel{
    unsigned char r,g,b;
} RGBpixel;

typedef struct Image {
    int width, height, magicNum;
    RGBpixel *data;
} Image;

Image *image;

static void read_ppm(char *filename){
  FILE *fh;

  // open file
  fh = fopen(filename, "rb");
  if (!fh) {
      fprintf(stderr, "Error opening file\n");
      exit(1);
  }

  // check if ppm
  c = fgetc(fh);
  if(c != 'P'){
    fprintf(stderr, "Not a ppm image file\n");
    exit(1);
  }

  ungetc(c, fh);

  // allocate memory
  image = (Image*)malloc(sizeof(Image));
  if (!image) {
      fprintf(stderr, "Error allocating memory\n");
      exit(1);
  }

  // get magic number
  c = fgetc(fh);
  c = fgetc(fh);
  image->magicNum = c;

  // skip magic number
  while((c = fgetc(fh)) != '\n'){
  }

  // skip comments
  c = fgetc(fh);
  while (c=='#'){
      while (fgetc(fh) != '\n');
          c=fgetc(fh);
  }
  ungetc(c, fh);

  // get height and width
  if(fscanf(fh, "%d %d", &image->width, &image->height) !=2){
      fprintf(stderr, "Error reading height and width\n");
      exit(1);
  }
  width = image->width;
  height = image->height;

  // get RGB_NUMBER from image and comapre to defined RGB_NUMBER
  int rgb_num;
  fscanf(fh, "%d", &rgb_num);
  if(rgb_num != RGB_NUMBER){
      fprintf(stderr, "Error with RGB_NUMBER in input image, it must be %d\n", RGB_NUMBER);
      exit(1);
  }


  fgetc(fh);

  //check if p3 or p6

} // end read function








int main(int argc, char *argv[]){

}
