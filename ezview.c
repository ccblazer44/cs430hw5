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
  int c;

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
  int width = image->width;
  int height = image->height;

  // get RGB_NUMBER from image and comapre to defined RGB_NUMBER
  int rgb_num;
  fscanf(fh, "%d", &rgb_num);
  if(rgb_num != RGB_NUMBER){
      fprintf(stderr, "Error with RGB_NUMBER in input image, it must be %d\n", RGB_NUMBER);
      exit(1);
  }

  // allocate memory
  image->data = (RGBpixel*)malloc((RGB_NUMBER + 1) * width * height);
  if(!image->data){
    fprintf(stderr, "Error allocating memory\n");
    exit(1);
  }

  fgetc(fh);

  //check if p3 or p6
  if(image->magicNum == '3'){
    int temp;
    for (int i = 0; i < height*width; i++){
        fscanf(fh, "%d", &temp);
        image->data[i].r = temp;
        fscanf(fh, "%d", &temp);
        image->data[i].g = temp;
        fscanf(fh, "%d", &temp);
        image->data[i].b = temp;
    }
  }

  if(image->magicNum == '6'){
    fread(image->data, (sizeof(&image->data)), height * width, fh);
  }

  fclose(fh);

} // end read function

typedef struct {
  float Position[2];
  float TexCoord[2];
} Vertex;

// (-1, 1)  (1, 1)
// (-1, -1) (1, -1)

Vertex vertexes[] = {
  {{-1, 1}, {0, 0}}, //Bottom Left
  {{1, 1},  {1, 0}},  //Bottom Right
  {{-1, -1},  {0, 1}},  //Top Left

  {{1, 1}, {1, 0}}, //Bottom Right
  {{1, -1},  {1, 1}}, //Top Right
  {{-1, -1},  {0, 1}} //Top Left

};

const GLubyte Indices[] = {
  0, 1, 2,
  2, 3, 0
};

static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec2 TexCoordIn;\n"
"attribute vec2 vPos;\n"
"varying vec2 TexCoordOut;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    TexCoordOut = TexCoordIn;\n"
"}\n";

static const char* fragment_shader_text =
"varying lowp vec2 TexCoordOut;\n"
"uniform sampler2D Texture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
"}\n";

void glCompileShaderOrDie(GLuint shader) {
  GLint compiled;
  glCompileShader(shader);
  glGetShaderiv(shader,
		GL_COMPILE_STATUS,
		&compiled);
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader,
		  GL_INFO_LOG_LENGTH,
		  &infoLen);
    char* info = malloc(infoLen+1);
    GLint done;
    glGetShaderInfoLog(shader, infoLen, &done, info);
    printf("Unable to compile shader: %s\n", info);
    exit(1);
  }
}




int main(int argc, char *argv[]){

  if (argc != 2){
      printf("usage: ezview input.ppm");
      return(1);
  }

  char* inFile = argv[1];
  read_ppm(inFile);

  return(1);

}
