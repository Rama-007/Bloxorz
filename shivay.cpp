#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <ctime>
#include <unistd.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <FTGL/ftgl.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;
int times;

typedef struct blockfet
{
  int x=0,z=0;float y=0,rotangle=0,xtranslate=0,ztranslate=0;int xrot=-1,zrot=0;
  int orientation=0;
}blockfet;

float zoomy=0;
GLfloat fov;

blockfet *block=(blockfet*)malloc(sizeof(blockfet));

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int upflag=0,angle=90,downflag=0,leftflag=0,rightflag=0;
int completeflag=1;
int blockflag=0,topflag=0,towerflag=0,followflag=0,helicopter=0,actualhelicopter=0,circularhelicopter=0;
float lx=0,lz=0,deltaangle=0;
int steps=0,level=1,camflag=0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
      if(completeflag==1)
      {
        switch (key) {
            case GLFW_KEY_UP:
            camflag=0;
             if(downflag==0 && upflag==0&&rightflag==0&&leftflag==0)
             {
                upflag=1;
                steps++;
                
              }
                break;
            case GLFW_KEY_DOWN:
            camflag=1;
             if(downflag==0 && upflag==0&&rightflag==0&&leftflag==0)
             {
                
                downflag=1;
                steps++;
              }  
                break;
            case GLFW_KEY_LEFT:
             if(downflag==0 && upflag==0&&rightflag==0&&leftflag==0)
                {
                  
                  leftflag=1;
                  steps++;
                }
                break;
            case GLFW_KEY_RIGHT:
             if(downflag==0 && upflag==0&&rightflag==0&&leftflag==0)
             {  
              
              rightflag=1;
              steps++;
             }
                break;
            case GLFW_KEY_0:
              blockflag=0;
              topflag=0;
              towerflag=0;
              followflag=0;
              helicopter=0;
              actualhelicopter=0;
              circularhelicopter=0;
              break;        

            case GLFW_KEY_1:
              blockflag=1;
              topflag=0;
              towerflag=0;
              followflag=0;
              helicopter=0;
              actualhelicopter=0;
              circularhelicopter=0;
              break;
            case GLFW_KEY_2:
              blockflag=0;
              topflag=1;
              towerflag=0;
              followflag=0;
              helicopter=0;
              actualhelicopter=0;
              circularhelicopter=0;
              break;
            case GLFW_KEY_3:
              blockflag=0;
              topflag=0;
              towerflag=1;
              followflag=0;
              helicopter=0;
              actualhelicopter=0;
              circularhelicopter=0;
              break;
            case GLFW_KEY_4:
              blockflag=0;
              topflag=0;
              towerflag=0;
              followflag=1;
              helicopter=0;
              actualhelicopter=0;
              circularhelicopter=0;
              break;
            case GLFW_KEY_5:
              blockflag=0;
              topflag=0;
              towerflag=0;
              followflag=0;
              helicopter=1;
              actualhelicopter=0;
              lx=0;lz=0;zoomy=0;
              circularhelicopter=0;
              break;
            case GLFW_KEY_6:
              blockflag=0;
              topflag=0;
              towerflag=0;
              followflag=0;
              helicopter=0;
              actualhelicopter=1;
              lx=0;lz=0;zoomy=0;
              circularhelicopter=0;
              break;
            case GLFW_KEY_7:
              blockflag=0;
              topflag=0;
              towerflag=0;
              followflag=0;
              helicopter=0;
              actualhelicopter=0;
              lx=0;lz=0;zoomy=0;
              circularhelicopter=1;
              break;
            default:
                break;
        }
      }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            system("pkill mpg123");
            quit(window);
            break;
		default:
			break;
	}
}

double totalanglex=0,totalanglez=0,totalangle=0, xorigin,deltaAngle=0,prevxpos=0,prevypos,otherx=0,otherz=0;


double checkpos(GLFWwindow* window)
{
  double xpos,ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
    if(xorigin==0)
    {
      if(prevxpos-xpos<0)
       {
        totalangle+=0.1;
        totalanglex+=(xpos-prevxpos)*0.001;
       }
      else
      {
        totalangle-=0.1;
        totalanglex-=(xpos-prevxpos)*0.001;
      }
      lx = 5*cos(totalanglex);
      if(prevypos-ypos<0)
       {
        totalanglez+=(ypos-prevypos)*0.001;
       }
      else
      {
        totalanglez-=(ypos-prevypos)*0.001;
      }
      lz = 10*sin(totalanglez);
      otherx=5*cos(totalangle);
      otherz=10*sin(totalangle);
    }
  return xpos;
}

/* Executed when a mouse button is pressed/released */
int scrollflag=0;
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
          if(helicopter==1||actualhelicopter==1)
          {
            if(action==GLFW_PRESS)
            {
              totalanglex+=deltaAngle;
              if(scrollflag==0)
              {
                glfwGetCursorPos(window, &prevxpos, &prevypos);
                scrollflag=1;
              }
              xorigin=0;
            }
            else
            {
              xorigin=checkpos(window);
              scrollflag=0;
            }
          }
          break;
        default:
            break;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if(helicopter==1||actualhelicopter==1||circularhelicopter==1)
  {
    if(yoffset==-1)
    {
     zoomy-=0.2;
    }
    else
    {
      zoomy+=0.2;
    }
    if(zoomy<-6)
      zoomy=-6;
    else if(zoomy>5)
      zoomy=5;
  }
}


VAO *segment;

void drawtext(glm::mat4 MVP,glm::mat4 VP,int a,int b,int c,int d, int e, int f ,int g,float trans)
{
  
  glm::mat4 translateRectangle1 = glm::translate (glm::vec3(-1*trans, 0, 0));   
  if(a==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectanglea = glm::translate (glm::vec3(-1*trans+3.5, 3, 0));        // glTranslatef
  glm::mat4 rotateRectanglea = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectanglea * rotateRectanglea);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
  }
  if(f==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectanglef = glm::translate (glm::vec3(0.7-trans+3.5, 3.45, 0));        // glTranslatef
  glm::mat4 rotateRectanglef = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectanglef * rotateRectanglef );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
}
  if(b==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangleb = glm::translate (glm::vec3(-0.45-trans+3.5, 3.7, 0));        // glTranslatef
  glm::mat4 rotateRectangleb = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangleb * rotateRectangleb );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
  }

  if(g==1)
  {

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangleg = glm::translate (glm::vec3(-1*trans+3.5, 2.65, 0));        // glTranslatef
  glm::mat4 rotateRectangleg = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangleg * rotateRectangleg );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
  }

  if(c==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectanglec = glm::translate (glm::vec3(-0.45-trans+3.5, 3.35, 0));        // glTranslatef
  glm::mat4 rotateRectanglec = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectanglec * rotateRectanglec );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
  }

  if(e==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectanglee = glm::translate (glm::vec3(0.7-trans+3.5, 3.1, 0));        // glTranslatef
  glm::mat4 rotateRectanglee = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectanglee * rotateRectanglee );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
  }


  if(d==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangled = glm::translate (glm::vec3(-1*trans+3.5, 2.3, 0));        // glTranslatef
  glm::mat4 rotateRectangled = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangled * rotateRectangled);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(segment);
  }

}




void check()
{
  if(upflag==1)
  {
    
    if(block->orientation==0)
    {
      block->xrot=-1;
      block->xtranslate=block->ztranslate=0;
     if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->z+=-1;
        block->rotangle=0;
        upflag=0;
      }
    }
    if(block->orientation==1)
    {
      block->xtranslate=block->ztranslate=0;
      if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->z+=-2;
        block->rotangle=0;
        block->orientation=2;
        upflag=0;
      }
    }
    if(block->orientation==2)
    {
      block->xtranslate=block->ztranslate=0;
      if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->z+=-1;
        block->rotangle=0;
        block->orientation=1;
        upflag=0;
      }
    }
  }
  else if(downflag==1)
  {
    if(block->orientation==0)
    {
      block->xrot=-1;
      block->xtranslate=0;
      block->ztranslate=1;
     if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->z+=1;
        block->rotangle=0;
        downflag=0;
      }
    }
    if(block->orientation==1)
    {
      block->xtranslate=0;block->ztranslate=1;
      if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->z+=1;
        block->rotangle=0;
        block->orientation=2;
        downflag=0;
      }
    }
    if(block->orientation==2)
    {
      block->xtranslate=0;block->ztranslate=2;
      if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->z+=2;
        block->rotangle=0;
        block->orientation=1;
        downflag=0;
      }
    }
  }
  else if(leftflag==1)
  {
    
    if(block->orientation==0)
    {
      block->xtranslate=block->ztranslate=0;
      block->xrot=-1;
     if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->x-=1;
        block->rotangle=0;
        block->orientation=1;
        leftflag=0;
      }
    }
    if(block->orientation==1)
    {
      block->xtranslate=block->ztranslate=0;
      if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->x+=-2;
        block->rotangle=0;
        block->orientation=0;
        leftflag=0;
      }
    }
    if(block->orientation==2)
    {
      block->xtranslate=block->ztranslate=0;
      if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->x+=-1;
        block->rotangle=0;
        block->orientation=2;
        leftflag=0;
      }
    }
  }
  else if(rightflag==1)
  {
    if(block->orientation==0)
    {
      block->xtranslate=2;
      block->ztranslate=1;
      block->xrot=-1;
     if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->x+=2;
        block->rotangle=0;
        block->orientation=1;
        rightflag=0;
      }
    }
    if(block->orientation==1)
    {
      block->xtranslate=1;block->ztranslate=0;
      if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->x+=1;
        block->rotangle=0;
        block->orientation=0;
        rightflag=0;
      }
    }
    if(block->orientation==2)
    {
      block->xtranslate=1;block->ztranslate=0;
      if(block->rotangle<1)
      {
       block->rotangle+=0.04;
      }
      else
      {
        system("mpg123  -n 10 fall.mp3 &");
        block->x+=1;
        block->rotangle=0;
        block->orientation=2;
        rightflag=0;
      }
    }
  }
}


VAO *cube[3], *rectangle,*board,*board1,*board2,*board3,*teleport,*oreint;
float bl[108];

void makecubes( float vertex_buffer_data[],float l, float b , float h)
{
  for(int i=0;i<36;i++)
  {
    bl[3*i]=l*vertex_buffer_data[3*i];
    bl[3*i+1]=b*vertex_buffer_data[3*i+1];
    bl[3*i+2]=h*vertex_buffer_data[3*i+2];
  }
}


void createsegment()
{
  static const GLfloat vertex_buffer_data [] = {
    0,0.8,0,
    0,0.7,0,
    0.25,0.8,0,
    0.25,0.8,0,
    0,0.7,0,
    0.25,0.7,0

  };

  static const GLfloat color_buffer_data [] = {
   0,0,1,
   0,0,1,
   0,0,1,
   0,0,1,
   0,0,1,
   0,0,1
  };


  // create3DObject creates and returns a handle to a VAO that can be used later
  segment = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}


void createcube()
{
  float a[108],c[108],cc[108],ccc[108],cccc[108],tel[108],orie[108];
  float  vertex_buffer_data [] = {
        0,0,0,
        1,0,0,
        1,0,1,
        1,0,1,
        0,0,1,
        0,0,0,

        0,1,0,
        1,1,0,
        1,1,1,
        1,1,1,
        0,1,1,
        0,1,0,

        0,0,1,
        1,0,1,
        1,1,1,
        1,1,1,
        0,1,1,
        0,0,1,

        0,0,0,
        1,0,0,
        1,1,0,
        1,1,0,
        0,1,0,
        0,0,0,

        0,0,1,
        0,1,1,
        0,1,0,
        0,1,0,
        0,0,0,
        0,0,1,
        
        1,0,1,
        1,1,1,
        1,1,0,
        1,1,0,
        1,0,0,
        1,0,1,  
    };
  for(int i=0;i<108;i++)
  {
    a[i]=0.6;
  }
  makecubes(vertex_buffer_data,2,1,1);
  cube[0]=create3DObject(GL_TRIANGLES, 36, bl, a, GL_FILL);
  makecubes(vertex_buffer_data,1,2,1);
  cube[1]=create3DObject(GL_TRIANGLES, 36, bl, a, GL_FILL);
  makecubes(vertex_buffer_data,1,1,2);
  cube[2]=create3DObject(GL_TRIANGLES, 36, bl, a, GL_FILL); 


  for(int i=0;i<108;i++)
  {
      c[i]=0;
  }
  for(int i=1*18;i<2*18;i++)
  {
    if(i%3==0)
      c[i]=1;
    if(i%15==0)
      c[i]=0.3;
  }

  makecubes(vertex_buffer_data,1,0.3,1);    
  board=create3DObject(GL_TRIANGLES, 36, bl, c, GL_FILL); 

  for(int i=0;i<108;i++)
  {
      cc[i]=0;
  }
  for(int i=1*18;i<2*18;i++)
  {
    if(i%3==1)
      cc[i]=1;
    if(i%15==1)
      cc[i]=0.3;
  }
  makecubes(vertex_buffer_data,1,0.3,1);    
  board1=create3DObject(GL_TRIANGLES, 36, bl, cc, GL_FILL);

  for(int i=0;i<108;i++)
  {
      ccc[i]=0;
  }
  for(int i=1*18;i<2*18;i++)
  {
    if(i%3==2)
      ccc[i]=1;
    if(i%15==2)
      ccc[i]=0.3;
  }
  makecubes(vertex_buffer_data,1,0.3,1);    
  board2=create3DObject(GL_TRIANGLES, 36, bl, ccc, GL_FILL);

  for(int i=0;i<108;i++)
  {
      cccc[i]=0;
  }
  for(int i=1*18;i<2*18;i++)
  {
    if(i%3==0)
      cccc[i]=0.5;
    if(i%3==1)
      cccc[i]=0.5;
    if(i%15==0)
      cccc[i]=0.3;
  }
  makecubes(vertex_buffer_data,1,0.3,1);    
  board3=create3DObject(GL_TRIANGLES, 36, bl, cccc, GL_FILL);

  for(int i=0;i<108;i++)
  {
    tel[i]=1;
  }
  makecubes(vertex_buffer_data,1,0.3,1);    
  teleport=create3DObject(GL_TRIANGLES, 36, bl, tel, GL_FILL);

  for(int i=0;i<108;i++)
  {
    orie[i]=(float) rand()/RAND_MAX;
  }
  makecubes(vertex_buffer_data,1,0.3,1);    
  oreint=create3DObject(GL_TRIANGLES, 36, bl, orie, GL_FILL);

}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	 fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
     Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    //Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}


void drawgameover(glm::mat4 MVP,glm::mat4 VP,int a,int b,int c,int d, int e, int f ,int g,float trans)
{
  
  glm::mat4 translateRectangle1 = glm::translate (glm::vec3(-1*trans, 0, 2));   
  if(a==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectanglea = glm::translate (glm::vec3(-1*trans, 3-3, 2));        // glTranslatef
  glm::mat4 rotateRectanglea = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectanglea * rotateRectanglea);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
  }
  if(f==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectanglef = glm::translate (glm::vec3(0.7-trans, 3.45-3, 2));        // glTranslatef
  glm::mat4 rotateRectanglef = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectanglef * rotateRectanglef );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
}
  if(b==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangleb = glm::translate (glm::vec3(-0.45-trans, 3.7-3, 2));        // glTranslatef
  glm::mat4 rotateRectangleb = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangleb * rotateRectangleb );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
  }

  if(g==1)
  {

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangleg = glm::translate (glm::vec3(-1*trans, 2.65-3, 2));        // glTranslatef
  glm::mat4 rotateRectangleg = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangleg * rotateRectangleg );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
  }

  if(c==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectanglec = glm::translate (glm::vec3(-0.45-trans, 3.35-3, 2));        // glTranslatef
  glm::mat4 rotateRectanglec = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectanglec * rotateRectanglec );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
  }

  if(e==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectanglee = glm::translate (glm::vec3(0.7-trans, 3.1-3, 2));        // glTranslatef
  glm::mat4 rotateRectanglee = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectanglee * rotateRectanglee );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(segment);
  }


  if(d==1)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangled = glm::translate (glm::vec3(-1*trans, 2.3-3, 2));        // glTranslatef
  glm::mat4 rotateRectangled = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangled * rotateRectangled);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(segment);
  }

}

void drawover(glm::mat4 MVP,glm::mat4 VP)
{
  float transt2=-2.5;

  drawgameover(MVP,VP,0,0,0,0,1,0,1,transt2);transt2+=0.5;
  drawgameover(MVP,VP,1,0,0,1,1,1,1,transt2);transt2+=0.5;
  drawgameover(MVP,VP,0,1,1,1,1,1,0,transt2);transt2+=0.5;
  drawgameover(MVP,VP,1,1,1,1,1,1,0,transt2);transt2+=0.85;
  drawgameover(MVP,VP,1,0,0,1,1,1,1,transt2);transt2+=0.5;
  drawgameover(MVP,VP,1,1,1,0,0,1,0,transt2);transt2+=0.25;
  drawgameover(MVP,VP,1,1,0,0,1,1,0,transt2);transt2+=0.5;
  drawgameover(MVP,VP,1,1,1,0,1,1,1,transt2);transt2+=0.5;
  drawgameover(MVP,VP,0,0,1,1,0,0,1,transt2);transt2+=0.15;
  drawgameover(MVP,VP,1,0,0,1,1,1,0,transt2);transt2+=0.5;
  
}








float camera_rotation_angle = 90;
float adjustment=0;
int strongbridgeflag=0,weakbridgeflag=0,oreintflag=0,switchflag1=0,switchflag2=0;
float brick[10][10][10];
int y[10],teleported[10];
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (int s[10][10],int bridge1[5],int bridge2[5])
{
  float camx=5*cos(camera_rotation_angle*M_PI/180.0f),camy=7,camz=5*sin(camera_rotation_angle*M_PI/180.0f),targetx=0,targety=0,targetz=0,upx=0,upy=1,upz=0;
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);
  if(blockflag==0&&topflag==0&&towerflag==0&&followflag==0&&helicopter==0&&actualhelicopter==0&&circularhelicopter==0)
  {
     camx=5*cos(camera_rotation_angle*M_PI/180.0f);camy=7;camz=5*sin(camera_rotation_angle*M_PI/180.0f);targetx=0;targety=0;targetz=0;upx=0;upy=1;upz=0;  
  }
  else if(blockflag==1)
  {
    
    if(camflag==0)
    {
      targetz=10;
      targetx=0;
      targety=0;
      camx=block->x;
    camy=block->y+2;
    camz=block->z+2;
    }
    else if(camflag==1)
    {
      targetz=-10;
      targetx=0;
      targety=0;
      camx=block->x;
    camy=block->y+2;
    camz=block->z-0.7;
    }
    
    
    upx=0;upy=1;upz=0;
  }
  else if(topflag==1)
  {
    camx=0;
    camy=block->y+5;
    camz=0;
    targetz=0;
    upx=1;upy=0;upz=0;
  }
  else if(towerflag==1)
  {
    camx=-3;
    camy=6;
    camz=6;
    targetx=targety=0;
    targetz=0;
    upx=0;upy=1;upz=0;
  }
  else if(followflag==1)
  {
      
    if(camflag==1)
    {
      camx=block->x;
    camy=block->y+4;
    camz=(float) block->z-0.7;
    targetz=10;
    upx=0;upy=1;upz=0;
    }
    else if(camflag==0)
    {
      camx=block->x;
    camy=block->y+4;
    camz=(float) block->z+2;
    targetz=-10;
    upx=0;upy=1;upz=0;
    }  
  }
  else if(helicopter==1)
  {
    camx=5*cos(camera_rotation_angle*M_PI/180.0f)+otherx;
         camy=7+zoomy;camz=5*sin(camera_rotation_angle*M_PI/180.0f)+otherz  ;
         targetx=0;targety=0;targetz=0;upx=0;upy=1;upz=0; 
  }
  if(actualhelicopter==1)
  {
    camx=5*cos(camera_rotation_angle*M_PI/180.0f);camy=7+zoomy;
    camz=5*sin(camera_rotation_angle*M_PI/180.0f);
    targetx=0+lx;targety=0;targetz=0+lz;upx=0;upy=1;upz=0;
  }
  if(circularhelicopter==1)
  {
    camx=5*cos(camera_rotation_angle*M_PI/180.0f);camy=7+zoomy;
    camz=5*sin(camera_rotation_angle*M_PI/180.0f);
    targetx=0+otherx;targety=0;targetz=0+otherz;upx=0;upy=1;upz=0;
  }
  
  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( camx, camy,camz );

  glm::vec3 target (targetx,targety,targetz);
  
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (upx, upy, upz);

  // Compute Camera matrix (view)
   Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  //Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  check();
  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 rotatecube;
  glm::mat4 translatecube = glm::translate (glm::vec3(block->x+adjustment,block->y,block->z));
  glm::mat4 translatecube1 = glm::translate (glm::vec3(block->xtranslate,0,block->ztranslate));

  if(downflag==1)
   rotatecube = glm::rotate((float)(block->rotangle*M_PI/2.0f), glm::vec3(1,0,0));
  else if(leftflag==1)
   rotatecube = glm::rotate((float)(block->rotangle*M_PI/2.0f), glm::vec3(0,0,1));
  else if(rightflag==1)
   rotatecube = glm::rotate((float)(block->rotangle*M_PI/2.0f), glm::vec3(0,0,-1));
  else
   rotatecube = glm::rotate((float)(block->rotangle*M_PI/2.0f), glm::vec3(-1,0,0));
  glm::mat4 translatecube2 = glm::translate (glm::vec3(-1*block->xtranslate,0,-1*block->ztranslate)); 
  Matrices.model *= (translatecube *translatecube1* rotatecube*translatecube2);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cube[block->orientation]);

 // cout<<block->x<<" "<<block->z<<endl;
  //cout<<s[5-block->x][5-block->z]<<endl;
  if(completeflag==0 && block->y<=-10.0f)
  {       
          block->z=y[level-1];
          block->x=block->y=0;
          block->orientation=0;
          block->rotangle=0;
          for(int i=0;i<10;i++)
          {
            for(int j=0;j<10;j++)
            {
              brick[level-1][i][j]=0;
            }
          }
          adjustment=0;
          switchflag1=0;
          s[bridge1[1]][bridge1[2]]=s[bridge1[3]][bridge1[4]]=0;
          s[bridge2[1]][bridge2[2]]=s[bridge2[3]][bridge2[4]]=0;
          completeflag=1;
          steps=0;
  }
  if(block->orientation==1 && s[5-block->x][5-block->z]==2)
  {
    block->y-=0.1;
    if(block->y>-0.2)
    {
      system("mpg123 -k 10  -n 80 lost.mp3 &");
    }
    brick[level-1][5-block->x][5-block->z]--;
    completeflag=0;   
  }
  if(block->orientation==0&&(s[5-block->x][5-block->z]==0))
  {
    adjustment=-1;
  }
  if(block->orientation==0&&(s[5-block->x-1][5-block->z]==0))
  {
    adjustment=1;
  }

  if(s[5-block->x][5-block->z]==3)
  {
    if(switchflag2==0)
      switchflag1++;
    if(switchflag1%2==1)
      s[bridge2[1]][bridge2[2]]=s[bridge2[3]][bridge2[4]]=9;
    else
      s[bridge2[1]][bridge2[2]]=s[bridge2[3]][bridge2[4]]=0;
    switchflag2=1;
  }
  else if(s[5-block->x][5-block->z]!=3)
    switchflag2=0;

  if(s[5-block->x][5-block->z]==6)
  {
    block->z+=teleported[level-1];
    system("mpg123 Beep3.mp3 &");
  }

  if(s[5-block->x][5-block->z]==7 && oreintflag==0)
  {
    block->orientation=(block->orientation+1)%3;
    oreintflag=1;
  }
  else if(s[5-block->x][5-block->z]!=7)
  {
    oreintflag=0;
  }

  if(s[5-block->x][5-block->z]==5 && block->orientation==1)
  {
    s[bridge1[1]][bridge1[2]]=s[bridge1[3]][bridge1[4]]=5;
  }

  if(s[5-block->x][5-block->z]==0||block->z>5||block->z<=-5||block->x>5||block->x<=-5||(block->orientation==2&&block->z==5)||(block->orientation==0&&s[5-block->x-1][5-block->z]==0)||(block->orientation==2&&s[5-block->x][5-block->z-1]==0))
  {
    block->y-=0.1;
    if(block->y>-0.2)
    {
      system("mpg123 -k 10  -n 80 lost.mp3 &");
    }
    if(block->orientation!=1)
      block->rotangle+=0.1;
    completeflag=0;
  }
  for(int i=0;i<10;i++)
  {
    for(int j=0;j<10;j++)
    {
      if(s[5-block->x][5-block->z]==4 && block->orientation==1)
      {
        block->y=-100;
        
        //completeflag=0;
        if(level==1)
        {
          system("mpg123 win.mp3 &");
          level=2;
          block->z=y[level-1];
          block->x=block->y=0;
          block->orientation=0;
        }
        else if(level==2)
        {
          system("mpg123 win.mp3 &");
          level=3;
          block->z=y[level-1];
          block->x=block->y=0;
          block->orientation=0;
        }
        else
          completeflag=2;
        break;
      }
    }
  }



 for(int i=0;i<10;i++)
  {
    for(int j=0;j<10;j++)
    {
      Matrices.model = glm::mat4(1.0f);
     glm::mat4 translateRectangle = glm::translate (glm::vec3(1*(5-i),0+brick[level-1][i][j], 1*(5-j)));        // glTranslatef
     glm::mat4 rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,1,0)); // rotate about vector (-1,1,1)
     Matrices.model *= (translateRectangle * rotateRectangle);// *rotateRectangle1);
     MVP = VP * Matrices.model;
     glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
     if(s[i][j]==1)
     {
      draw3DObject(board);
      }
      else if(s[i][j]==2)
      {
        draw3DObject(board2);
      }
      else if(s[i][j]==3||s[i][j]==9)
      {
        draw3DObject(board1);
      }
      else if(s[i][j]==5)
      {
        draw3DObject(board3);
      }
      else if(s[i][j]==6)
      {
        draw3DObject(teleport);
      }
      else if(s[i][j]==7)
      {
        draw3DObject(oreint);
      }
    }
  }

  int temptext=times,templife=level;float transt=0,transt1=2;
  while(temptext)
  {
    int digit=temptext%10;
    temptext/=10;
    if(digit==0)
      drawtext(MVP,VP,1,1,1,1,1,1,0,transt);
    else if(digit==1)
      drawtext(MVP,VP,0,1,1,0,0,0,0,transt);
    else if(digit==2)
      drawtext(MVP,VP,1,1,0,1,1,0,1,transt);
    else if(digit==3)
      drawtext(MVP,VP,1,1,1,1,0,0,1,transt);
    else if(digit==4)
      drawtext(MVP,VP,0,1,1,0,0,1,1,transt);
    else if(digit==5)
      drawtext(MVP,VP,1,0,1,1,0,1,1,transt);
    else if(digit==6)
      drawtext(MVP,VP,1,0,1,1,1,1,1,transt);
    else if(digit==7)
      drawtext(MVP,VP,1,1,1,0,0,0,0,transt);
    else if(digit==8)
      drawtext(MVP,VP,1,1,1,1,1,1,1,transt);
    else if(digit==9)
      drawtext(MVP,VP,1,1,1,1,0,1,1,transt);
    transt+=0.5;
  }
  while(templife)
  {
    int digit=templife%10;
    templife/=10;
    if(digit==0)
      drawtext(MVP,VP,1,1,1,1,1,1,0,transt1);
    else if(digit==1)
      drawtext(MVP,VP,0,1,1,0,0,0,0,transt1);
    else if(digit==2)
      drawtext(MVP,VP,1,1,0,1,1,0,1,transt1);
    else if(digit==3)
      drawtext(MVP,VP,1,1,1,1,0,0,1,transt1);
    else if(digit==4)
      drawtext(MVP,VP,0,1,1,0,0,1,1,transt1);
    else if(digit==5)
      drawtext(MVP,VP,1,0,1,1,0,1,1,transt1);
    else if(digit==6)
      drawtext(MVP,VP,1,0,1,1,1,1,1,transt1);
    else if(digit==7)
      drawtext(MVP,VP,1,1,1,0,0,0,0,transt1);
    else if(digit==8)
      drawtext(MVP,VP,1,1,1,1,1,1,1,transt1);
    else if(digit==9)
      drawtext(MVP,VP,1,1,1,1,0,1,1,transt1);
    transt1+=0.5;
  }
  if(completeflag==2)
     drawover(MVP,VP);


  //camera_rotation_angle++; // Simulating camera rotation
  
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "BloxOrz", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, scroll_callback);

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createcube ();
  createsegment();
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);


}

int main (int argc, char** argv)
{
	int width = 1000;
	int height = 2000;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;
    int s[10][10][10],bridge1[10][5],bridge2[10][5];
    for(int i=0;i<10;i++)
    {
      for(int j=0;j<10;j++)
        s[0][i][j]=0;
    }
    for(int i=4;i<8;i++)
    {
      for(int j=0;j<10;j++)
      {
        s[0][i][j]=1;
      }
    }
    //level1
    s[0][7][7]=3;
    s[0][6][5]=5;
    s[0][6][2]=2; 
    s[0][4][2]=2;
    s[0][6][1]=4;
    s[0][6][3]=s[0][6][4]=0;
    s[0][4][3]=s[0][4][4]=0;
    s[0][5][8]=6;
    s[0][5][6]=7;

    bridge1[0][1]=4;
    bridge1[0][2]=3;
    bridge1[0][3]=4;
    bridge1[0][4]=4;

    bridge2[0][1]=6;
    bridge2[0][2]=3;
    bridge2[0][3]=6;
    bridge2[0][4]=4;
    y[0]=0;
    block->z=y[level-1];
    teleported[0]=5;



    //level2
    for(int i=0;i<10;i++)
    {
      for(int j=0;j<10;j++)
        s[1][i][j]=0;
    }
    y[1]=4;
    for(int i=4;i<7;i++)
    {
      for(int j=0;j<3;j++)
      {
        s[1][i][j]=1;
      }
    }  
    s[1][5][3]=s[1][5][4]=0; 
    s[1][6][5]=s[1][5][5]=s[1][4][5]=1;
    s[1][5][6]=0;
    s[1][6][7]=s[1][5][7]=s[1][4][7]=1;
    s[1][6][8]=s[1][5][8]=s[1][4][8]=1;
    s[1][6][9]=s[1][5][9]=s[1][4][9]=1;
    s[1][6][0]=2;
    s[1][5][8]=4;
    s[1][5][2]=5;
    s[1][5][5]=3;

    bridge1[1][1]=5;
    bridge1[1][2]=3;
    bridge1[1][3]=5;
    bridge1[1][4]=4;

    bridge2[1][1]=5;
    bridge2[1][2]=6;
    bridge2[1][3]=5;
    bridge2[1][4]=6;


    //level 3
    for(int i=0;i<10;i++)
    {
      for(int j=0;j<10;j++)
        s[2][i][j]=0;
    }
    y[2]=5; 
    for(int i=0;i<10;i++)
    {
      s[2][i][0]=1;
    }
    for(int i=0;i<10;i++)
    {
      s[2][i][2]=1;
    }
    teleported[2]=-2;

    bridge2[2][1]=0;
    bridge2[2][2]=3;
    bridge2[2][3]=0;
    bridge2[2][4]=4;

    bridge1[2][1]=1;
    bridge1[2][2]=5;
    bridge1[2][3]=2;
    bridge1[2][4]=5;


    s[2][9][0]=6;
    s[2][0][2]=3;
    s[2][0][3]=0;//
    s[2][0][4]=0;//
    s[2][0][5]=5;
    s[2][1][5]=0;//
    s[2][2][5]=0;//
    for(int i=3;i<10;i++)
      s[2][i][5]=1;
    for(int i=6;i<10;i++)
      s[2][9][i]=1;
    for(int i=7;i<10;i++)
    {
      for(int j=7;j<10;j++)
      {
        s[2][i][j]=1;
      }
    }
    s[2][8][8]=7;
    for(int i=5;i<7;i++)
    {
      for(int j=8;j<10;j++)
        s[2][i][j]=1;
    }

    for(int i=2;i<5;i++)
    {
      for(int j=7;j<10;j++)
        s[2][i][j]=1;
    }
    s[2][3][8]=4;
    s[2][5][8]=2;
    s[2][3][7]=2;

    for(int k=0;k<10;k++)
    {
      for(int i=0;i<10;i++)
      {
       for(int j=0;j<10;j++)
       {
         brick[k][i][j]=0;
       }
      }
    }

    /* Draw in loop */
    
    times = glfwGetTime();

    system("mpg123 game.mp3 &");
    while (!glfwWindowShouldClose(window)) {

        times=glfwGetTime()-times;
        // OpenGL Draw commands
        draw(s[level-1],bridge1[level-1],bridge2[level-1]);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}
