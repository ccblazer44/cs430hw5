#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

#include "linmath.h"

#include <assert.h>

#define RGB_NUMBER 255

float trans_x = 0, trans_y = 0, scale = 1, rotation = 0, sheer = 0;



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

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS){
      trans_y += 0.1;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
      trans_y -= 0.1;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
      trans_x -= 0.1;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
      trans_x += 0.1;
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS){
      rotation += 0.1;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS){
      rotation -= 0.1;
    }


}

static void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char *argv[]){

  if (argc != 2){
      printf("usage: ezview input.ppm");
      return(1);
  }

  char* inFile = argv[1];
  read_ppm(inFile);

  GLFWwindow* window;
  GLuint vertex_buffer, vertex_shader, fragment_shader, program;
  GLint mvp_location, vpos_location, vcol_location;

  glfwSetErrorCallback(error_callback);

  if (!glfwInit()){
    exit(EXIT_FAILURE);
  }


      glfwDefaultWindowHints();
      glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
      glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  window = glfwCreateWindow(800, 600, "ezview", NULL, NULL);
  if (!window){
      glfwTerminate();
      exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);
  // gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);

  // NOTE: OpenGL error checks have been omitted for brevity

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);


  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShaderOrDie(vertex_shader);

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShaderOrDie(fragment_shader);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  // more error checking! glLinkProgramOrDie!

  mvp_location = glGetUniformLocation(program, "MVP");
  assert(mvp_location != -1);

  vpos_location = glGetAttribLocation(program, "vPos");
  assert(vpos_location != -1);

  GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
  assert(texcoord_location != -1);

  GLint tex_location = glGetUniformLocation(program, "Texture");
  assert(tex_location != -1);

  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location,
      2,
      GL_FLOAT,
      GL_FALSE,
                        sizeof(Vertex),
      (void*) 0);

  glEnableVertexAttribArray(texcoord_location);
  glVertexAttribPointer(texcoord_location,
      2,
      GL_FLOAT,
      GL_FALSE,
                        sizeof(Vertex),
      (void*) (sizeof(float) * 2));


  // int image_width = 4;
  // int image_height = 4;

  GLuint texID;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB,
   GL_UNSIGNED_BYTE, image->data);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texID);
  glUniform1i(tex_location, 0);

  while (!glfwWindowShouldClose(window))
  {
      float ratio;
      int width, height;
      mat4x4 m, p, mvp, trans, rotat;

      glfwGetFramebufferSize(window, &width, &height);
      ratio = width / (float) height;

      glViewport(0, 0, width, height);
      glClear(GL_COLOR_BUFFER_BIT);

      mat4x4_identity(m); // main matrix

      mat4x4_identity(trans); // translation matrix
      mat4x4_translate(trans,trans_x,trans_y,0);  // translate
      mat4x4_add(m,trans,m); // combine translation and main matrix

      mat4x4_identity(rotat); // rotation matrix
      mat4x4_rotate_Z(rotat, rotat, rotation); // rotate
      mat4x4_mul(m,rotat,m); // combine rotation and main matrix







      mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      mat4x4_mul(mvp, p, m);

      glUseProgram(program);
      glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      glfwSwapBuffers(window);
      glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);

}
