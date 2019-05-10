/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code

  Student username: joeyuhoc
*/

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"
#include <vector>
#include <iostream>
#include <cstring>

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// POINTS/POINT defined in windef.h
typedef enum { DISPLAY_POINTS, DISPLAY_TRIANGLES, DISPLAY_WIREFRAME,
DISPLAY_WIREFRAME_ON_TRIANGLE, DISPLAY_IMAGE_ON_TRIANGLE, DISPLAY_MILESTONE } DISPLAY_TYPE;
DISPLAY_TYPE displayType = DISPLAY_POINTS;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I Joe 8800444224";

ImageIO * colorImage;
ImageIO * heightmapImage;
OpenGLMatrix matrix;
BasicPipelineProgram pipelineProgram, pipelineProgramColor;
GLuint program, programColor;
GLint h_modelViewMatrix, h_projectionMatrix;

int fovy = 45;  // Field of Viww y
float aspect = 1280.0f / 720.0f; // Aspect Ratio
float zStudent = 3.8800444224; // Camera Z Positon for Milestone
int shaderNumber = 1;
int screenshotCounter = 0;

GLuint loc;

GLuint VAO;
GLuint VBO_POSITION;
GLuint VBO_COLORS;

GLuint VAO_POINT;
GLuint VBO_POSITION_POINT;
GLuint VBO_COLORS_POINT;
vector<GLfloat> POINT_VERTICES;
vector<GLfloat> POINT_COLORS;

GLuint VAO_TRIANGLE;
GLuint VBO_POSITION_TRIANGLE;
GLuint VBO_COLORS_TRIANGLE;
vector<GLfloat> TRIANGLE_VERTICES;
vector<GLfloat> TRIANGLE_COLORS;
vector<GLuint> TRIANGLE_INDICES;
GLuint IBO_TRIANGLE;

GLuint VAO_WIREFRAME;
GLuint VBO_POSITION_WIREFRAME;
GLuint VBO_COLORS_WIREFRAME;
vector<GLfloat> WIREFRAME_VERTICES;
vector<GLfloat> WIREFRAME_COLORS;

GLuint VAO_HYBRID;
GLuint VBO_POSITION_HYBRID;
GLuint VBO_COLORS_HYBRID;
vector<GLfloat> HYBRID_VERTICES;
vector<GLfloat> HYBRID_COLORS;

GLuint VAO_COLOR;
GLuint VBO_POSITION_COLOR;
GLuint VBO_COLORS_COLOR;
vector<GLfloat> COLOR_VERTICES;
vector<GLfloat> COLOR_COLORS;
GLuint IBO_COLOR;

GLuint VAO_COLORHEIGHT;
GLuint VBO_POSITION_COLORHEIGHT;
GLuint VBO_COLORS_COLORHEIGHT;
vector<GLfloat> COLORHEIGHT_VERTICES;
vector<GLfloat> COLORHEIGHT_COLORS;

// Write a Screenshot to the Specified Filename
void saveScreenshot(const char * filename){
	unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;

	delete [] screenshotData;
}

// Initalizing Modelview Matrix and Projection Matrix
void initializeMatrix(){

	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.LoadIdentity(); // Load Identity Matrix

	// Set Camera Pointing Direction and Position
	if (displayType == DISPLAY_MILESTONE) {
		matrix.LookAt(0, 0, zStudent, 0, 0, -1, 0, 1, 0); 
	}
	else {
		matrix.LookAt(250, 252, 256, 0, 0, 0, 0, 1, 0);
	}

	// Translate, Rotate, and Scale the Matrix
	matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
	matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
	matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
	matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
	matrix.Scale(landScale[0], landScale[1], landScale[2]);
	
	float m[16]; 
	matrix.GetMatrix(m); // Obtatin Current Matrix
	//pipelineProgram.SetModelViewMatrix(m); // Upload Modelveiw Matrix to GPU
	glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);

	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.LoadIdentity(); // Load Identity Matrix
	matrix.Perspective(fovy, aspect, 0.01, 1000.0); // Set Perspective

	matrix.GetMatrix(m); // Obtatin Current Matrix
	//pipelineProgram.SetProjectionMatrix(m); // Upload Projection Matrix to GPU
	glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, m);
}

// Initialize heightmapImage Vertices if 3 Channels (RGB)
void initializeVerticesRGB() {

	// Scale Down to Look Nice
	float scale = 5 * (float)(heightmapImage->getHeight() / 100);

	// Obtain Image Width and Height
	int IMAGE_WIDTH = heightmapImage->getWidth();
	int IMAGE_HEIGHT = heightmapImage->getHeight();
	GLuint counter = 0;

	// For Every Pixel (Move Vertices Negetive Left and Down Half Size to Center the Image)
	for (int i = -IMAGE_HEIGHT / 2; i < IMAGE_HEIGHT / 2 - 1; i++) {
		for (int j = -IMAGE_WIDTH / 2; j < IMAGE_WIDTH / 2 - 1; j++) {

			// Correct Pixel 
			int x = i + IMAGE_HEIGHT / 2;
			int y = j + IMAGE_WIDTH / 2;

			// Get 4 Vertices and Colors (For Traingle and WireFrame), 
			//   VERTEX_TOP_LEFT  --------  VERTEX_TOP_RIGHT
			//                    |      |
			//                    |      |
			// VERTEX_BOTTOM_LEFT --------  VERTEX_BOTTOM_RIGHT

			GLfloat r = (float)colorImage->getPixel(x, y, 0) / 255.0f;
			GLfloat g = (float)colorImage->getPixel(x, y, 1) / 255.0f;
			GLfloat b = (float)colorImage->getPixel(x, y, 2) / 255.0f;
			GLfloat grayscale = 0.3 * r + 0.59 * g + 0.11 * b;
			GLfloat height = scale * grayscale;
			GLfloat VERTEX_BOTTOM_LEFT[3] = { (float)i , height, (float)-j };
			GLfloat COLOR_BOTTOM_LEFT[4] = { r, g, b, 1.0f };

			r = (float)colorImage->getPixel(x + 1, y, 0) / 255.0f;
			g = (float)colorImage->getPixel(x + 1, y, 1) / 255.0f;
			b = (float)colorImage->getPixel(x + 1, y, 2) / 255.0f;
			grayscale = 0.3 * r + 0.59 * g + 0.11 * b;
			height = scale * grayscale;
			GLfloat VERTEX_BOTTOM_RIGHT[3] = { (float)i + 1 , height, (float)-j };
			GLfloat COLOR_BOTTOM_RIGHT[4] = { r, g, b, 1.0f };

			r = (float)colorImage->getPixel(x, y + 1, 0) / 255.0f;
			g = (float)colorImage->getPixel(x, y + 1, 1) / 255.0f;
			b = (float)colorImage->getPixel(x, y + 1, 2) / 255.0f;
			grayscale = 0.3 * r + 0.59 * g + 0.11 * b;
			height = scale * grayscale;
			GLfloat VERTEX_TOP_LEFT[3] = { (float)i , height, (float)-(j + 1) };
			GLfloat COLOR_TOP_LEFT[4] = { r, g, b, 1.0f };

			r = (float)colorImage->getPixel(x + 1, y + 1, 0) / 255.0f;
			g = (float)colorImage->getPixel(x + 1, y + 1, 1) / 255.0f;
			b = (float)colorImage->getPixel(x + 1, y + 1, 2) / 255.0f;
			grayscale = 0.3 * r + 0.59 * g + 0.11 * b;
			height = scale * grayscale;
			GLfloat VERTEX_TOP_RIGHT[3] = { (float)i + 1 , height, (float)-(j + 1) };
			GLfloat COLOR_TOP_RIGHT[4] = { r, g, b, 1.0f };

			// Insert Point
			POINT_VERTICES.insert(POINT_VERTICES.end(), VERTEX_BOTTOM_LEFT, VERTEX_BOTTOM_LEFT + 3);

			// Insert Triangles (6 or 4 Points for 2 Triangles)
			// If glDrawArrays => Insert 6 Vertices' Colors; If glDrawElement => Insert 4 Only
			TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_BOTTOM_LEFT, VERTEX_BOTTOM_LEFT + 3);
			TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_BOTTOM_RIGHT, VERTEX_BOTTOM_RIGHT + 3);
			TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_TOP_RIGHT, VERTEX_TOP_RIGHT + 3);
			//TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_TOP_RIGHT, VERTEX_TOP_RIGHT + 3);
			TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_TOP_LEFT, VERTEX_TOP_LEFT + 3);
			//TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_BOTTOM_LEFT, VERTEX_BOTTOM_LEFT + 3);

			GLuint index[6] = { counter, counter + 1, counter + 2, counter + 2, counter + 3, counter };
			counter += 4;
			TRIANGLE_INDICES.insert(TRIANGLE_INDICES.end(), index, index + 6);

			// Insert Wireframe (4 Lines to for a Box)
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_BOTTOM_LEFT, VERTEX_BOTTOM_LEFT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_BOTTOM_RIGHT, VERTEX_BOTTOM_RIGHT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_BOTTOM_RIGHT, VERTEX_BOTTOM_RIGHT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_TOP_RIGHT, VERTEX_TOP_RIGHT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_TOP_RIGHT, VERTEX_TOP_RIGHT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_TOP_LEFT, VERTEX_TOP_LEFT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_TOP_LEFT, VERTEX_TOP_LEFT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_BOTTOM_LEFT, VERTEX_BOTTOM_LEFT + 3);

			// Insert Point Color
			POINT_COLORS.insert(POINT_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);

			// Insert Triangle Colors (6 or 4 Points for 2 Triangles)
			TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);
			TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_BOTTOM_RIGHT, COLOR_BOTTOM_RIGHT + 4);
			TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_TOP_RIGHT, COLOR_TOP_RIGHT + 4);
			//TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_TOP_RIGHT, COLOR_TOP_RIGHT + 4);
			TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_TOP_LEFT, COLOR_TOP_LEFT + 4);
			//TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);

			// Insert Wireframe Colors (4 Lines to for a Box)
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_BOTTOM_RIGHT, COLOR_BOTTOM_RIGHT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_BOTTOM_RIGHT, COLOR_BOTTOM_RIGHT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_TOP_RIGHT, COLOR_TOP_RIGHT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_TOP_RIGHT, COLOR_TOP_RIGHT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_TOP_LEFT, COLOR_TOP_LEFT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_TOP_LEFT, COLOR_TOP_LEFT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);

			// Insert Wireframe Color for Overlaying (Single Orange Color)
			GLfloat singleColor[4] = { 1.0f, 0.72f, 0.3f, 1.0f };
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
		}
	}
}

// Initialize heightmapImage Vertices if 1 Channel (Grayscale)
void initializeVertices() {

	// Scale Down to Look Nice
	float scale = 0.05 * (float)(heightmapImage->getHeight() / 100);

	// Obtain Image Width and Height
	int IMAGE_WIDTH = heightmapImage->getWidth();
	int IMAGE_HEIGHT = heightmapImage->getHeight();
	GLuint counter = 0;

	// For Every Pixel (Move Vertices Negetive Left and Down Half Size to Center the Image)
	for (int i = -IMAGE_HEIGHT / 2 ; i < IMAGE_HEIGHT / 2 - 1 ; i++) {
		for (int j = -IMAGE_WIDTH / 2; j < IMAGE_WIDTH / 2 - 1; j++) {

			// Correct Pixel 
			int x = i + IMAGE_HEIGHT / 2;
			int y = j + IMAGE_WIDTH / 2;

			// Get 4 Vertices (For Traingle and WireFrame), 
			//   VERTEX_TOP_LEFT  --------  VERTEX_TOP_RIGHT
			//                    |      |
			//                    |      |
			// VERTEX_BOTTOM_LEFT --------  VERTEX_BOTTOM_RIGHT

			GLfloat height = scale * heightmapImage->getPixel(x, y, 0);
			GLfloat VERTEX_BOTTOM_LEFT[3] = { (float)i , height, (float)-j };

			height = scale * heightmapImage->getPixel(x + 1, y, 0);
			GLfloat VERTEX_BOTTOM_RIGHT[3] = { (float)i + 1 , height, (float)-j };

			height = scale * heightmapImage->getPixel(x, y + 1, 0);
			GLfloat VERTEX_TOP_LEFT[3] = { (float)i , height, (float)-(j + 1) };

			height = scale * heightmapImage->getPixel(x + 1, y + 1, 0);
			GLfloat VERTEX_TOP_RIGHT[3] = { (float)i + 1 , height, (float)-(j + 1) };

			// Insert Point
			POINT_VERTICES.insert(POINT_VERTICES.end(), VERTEX_BOTTOM_LEFT, VERTEX_BOTTOM_LEFT + 3);

			// Insert Triangles (6 or 4 Points for 2 Triangles)
			// If glDrawArrays => Insert 6 Vertices; If glDrawElement => Insert 4 Only
			TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_BOTTOM_LEFT, VERTEX_BOTTOM_LEFT + 3);
			TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_BOTTOM_RIGHT, VERTEX_BOTTOM_RIGHT + 3);
			TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_TOP_RIGHT, VERTEX_TOP_RIGHT + 3);
			//TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_TOP_RIGHT, VERTEX_TOP_RIGHT + 3);
			TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_TOP_LEFT, VERTEX_TOP_LEFT + 3);
			//TRIANGLE_VERTICES.insert(TRIANGLE_VERTICES.end(), VERTEX_BOTTOM_LEFT, VERTEX_BOTTOM_LEFT + 3);

			GLuint index[6] = { counter, counter + 1, counter + 2, counter + 2, counter + 3, counter };
			counter += 4;
			TRIANGLE_INDICES.insert(TRIANGLE_INDICES.end(), index, index + 6);

			// Insert Wireframe (4 Lines to for a Box)
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_BOTTOM_LEFT, VERTEX_BOTTOM_LEFT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_BOTTOM_RIGHT, VERTEX_BOTTOM_RIGHT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_BOTTOM_RIGHT, VERTEX_BOTTOM_RIGHT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_TOP_RIGHT, VERTEX_TOP_RIGHT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_TOP_RIGHT, VERTEX_TOP_RIGHT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_TOP_LEFT, VERTEX_TOP_LEFT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_TOP_LEFT, VERTEX_TOP_LEFT + 3);
			WIREFRAME_VERTICES.insert(WIREFRAME_VERTICES.end(), VERTEX_BOTTOM_LEFT, VERTEX_BOTTOM_LEFT + 3);

			// Get 4 Vertices' Colors (For Traingle and WireFrame), 
			GLfloat color = (float)heightmapImage->getPixel(x , y , 0) / 255.0f; // Reduce Value to Range 0 - 1
			GLfloat COLOR_BOTTOM_LEFT[4] = { color, color, color, 1.0f };

			color = (float)heightmapImage->getPixel(x + 1, y, 0) / 255.0f; // Reduce Value to Range 0 - 1
			GLfloat COLOR_BOTTOM_RIGHT[4] = { color, color, color, 1.0f };

			color = (float)heightmapImage->getPixel(x, y + 1, 0) / 255.0f; // Reduce Value to Range 0 - 1
			GLfloat COLOR_TOP_LEFT[4] = { color, color, color, 1.0f };

			color = (float)heightmapImage->getPixel(x + 1, y + 1, 0) / 255.0f; // Reduce Value to Range 0 - 1
			GLfloat COLOR_TOP_RIGHT[4] = { color, color, color, 1.0f };

			// Insert Point Color
			POINT_COLORS.insert(POINT_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);

			// Insert Triangle Colors (6 or 4 Points for 2 Triangles)
			// If glDrawArrays => Insert 6 Vertices' Colors; If glDrawElement => Insert 4 Only
			TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);
			TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_BOTTOM_RIGHT, COLOR_BOTTOM_RIGHT + 4);
			TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_TOP_RIGHT, COLOR_TOP_RIGHT + 4);
			//TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_TOP_RIGHT, COLOR_TOP_RIGHT + 4);
			TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_TOP_LEFT, COLOR_TOP_LEFT + 4);
			//TRIANGLE_COLORS.insert(TRIANGLE_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);

			// Insert Wireframe Colors (4 Lines to for a Box)
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_BOTTOM_RIGHT, COLOR_BOTTOM_RIGHT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_BOTTOM_RIGHT, COLOR_BOTTOM_RIGHT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_TOP_RIGHT, COLOR_TOP_RIGHT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_TOP_RIGHT, COLOR_TOP_RIGHT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_TOP_LEFT, COLOR_TOP_LEFT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_TOP_LEFT, COLOR_TOP_LEFT + 4);
			WIREFRAME_COLORS.insert(WIREFRAME_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);

			// Insert Wireframe Color for Overlaying (Single Orange Color)
			GLfloat singleColor[4] = { 1.0f, 0.72f, 0.3f, 1.0f };
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
			HYBRID_COLORS.insert(HYBRID_COLORS.end(), singleColor, singleColor + 4);
		}
	}	
}

// Initiazize Colors from Other Image to Paint Triangle
void initializeColorImage() {

	// Obtain Image Width and Height
	int IMAGE_WIDTH = colorImage->getWidth();
	int IMAGE_HEIGHT = colorImage->getHeight();

	// For Every Pixel Obtain RGB Values in Range 0 - 1, Set Alpha to 1.0f and Save to Vector
	for (int i = 0; i < IMAGE_HEIGHT - 1; i++) {
		for (int j = 0; j < IMAGE_WIDTH - 1; j++) {
			GLfloat r = (float)colorImage->getPixel(i, j, 0) / 255.0f;
			GLfloat g = (float)colorImage->getPixel(i, j, 1) / 255.0f;
			GLfloat b = (float)colorImage->getPixel(i, j, 2) / 255.0f;
			GLfloat COLOR_BOTTOM_LEFT[4] = { r, g, b, 1.0f };

			r = (float)colorImage->getPixel(i + 1, j, 0) / 255.0f;
			g = (float)colorImage->getPixel(i + 1, j, 1) / 255.0f;
			b = (float)colorImage->getPixel(i + 1, j, 2) / 255.0f;
			GLfloat COLOR_BOTTOM_RIGHT[4] = { r, g, b, 1.0f };

			r = (float)colorImage->getPixel(i, j + 1, 0) / 255.0f;
			g = (float)colorImage->getPixel(i, j + 1, 1) / 255.0f;
			b = (float)colorImage->getPixel(i, j + 1, 2) / 255.0f;
			GLfloat COLOR_TOP_LEFT[4] = { r, g, b, 1.0f };

			r = (float)colorImage->getPixel(i + 1, j + 1, 0) / 255.0f;
			g = (float)colorImage->getPixel(i + 1, j + 1, 1) / 255.0f;
			b = (float)colorImage->getPixel(i + 1, j + 1, 2) / 255.0f;
			GLfloat COLOR_TOP_RIGHT[4] = { r, g, b, 1.0f };

			// If glDrawArrays => Insert 6 Vertices' Colors; If glDrawElement => Insert 4 Only
			COLOR_COLORS.insert(COLOR_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);
			COLOR_COLORS.insert(COLOR_COLORS.end(), COLOR_BOTTOM_RIGHT, COLOR_BOTTOM_RIGHT + 4);
			COLOR_COLORS.insert(COLOR_COLORS.end(), COLOR_TOP_RIGHT, COLOR_TOP_RIGHT + 4);
			//COLOR_COLORS.insert(COLOR_COLORS.end(), COLOR_TOP_RIGHT, COLOR_TOP_RIGHT + 4);
			COLOR_COLORS.insert(COLOR_COLORS.end(), COLOR_TOP_LEFT, COLOR_TOP_LEFT + 4);
			//COLOR_COLORS.insert(COLOR_COLORS.end(), COLOR_BOTTOM_LEFT, COLOR_BOTTOM_LEFT + 4);
		}
	}
}

// Initialize VAOs and VBOs
void initilaizeVBOs() {

	/*******************************
	POINT VAO VBOs (For Point Mode)
	*******************************/

	// Generate and Bind VAO_POINT
	glGenVertexArrays(1, &VAO_POINT);
	glBindVertexArray(VAO_POINT);

	// Generate and Bind VBO_POSITION_POINT
	glGenBuffers(1, &VBO_POSITION_POINT);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION_POINT);
	glBufferData(GL_ARRAY_BUFFER, POINT_VERTICES.size() * sizeof(GLfloat), &POINT_VERTICES[0], GL_STATIC_DRAW);

	// Generate and Bind VBO_COLORS_POINT
	glGenBuffers(1, &VBO_COLORS_POINT);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS_POINT);
	glBufferData(GL_ARRAY_BUFFER, POINT_COLORS.size() * sizeof(GLfloat), &POINT_COLORS[0], GL_STATIC_DRAW);

	// Enable and Set Shader for VBO_POSITION_POINT
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION_POINT);
	loc = glGetAttribLocation(program, "position"); // Get the Location of the "position" Shader Variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and Set Shader for VBO_COLORS_POINT
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS_POINT);
	loc = glGetAttribLocation(program, "color"); // Get the Location of the "color" Shader Variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/*************************************
	TRIANGLE VAO VBOs (For Triangle Mode)
	*************************************/

	// Generate and Bind VAO_TRIANGLE
	glGenVertexArrays(1, &VAO_TRIANGLE);
	glBindVertexArray(VAO_TRIANGLE);

	// Generate and Bind VBO_POSITION_TRIANGLE
	glGenBuffers(1, &VBO_POSITION_TRIANGLE);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION_TRIANGLE);
	glBufferData(GL_ARRAY_BUFFER, TRIANGLE_VERTICES.size() * sizeof(GLfloat), &TRIANGLE_VERTICES[0], GL_STATIC_DRAW);

	// Generate and Bind VBO_COLORS_TRIANGLE
	glGenBuffers(1, &VBO_COLORS_TRIANGLE);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS_TRIANGLE);
	glBufferData(GL_ARRAY_BUFFER, TRIANGLE_COLORS.size() * sizeof(GLfloat), &TRIANGLE_COLORS[0], GL_STATIC_DRAW);

	// Enable and Set Shader for VBO_POSITION_TRIANGLE
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION_TRIANGLE);
	loc = glGetAttribLocation(program, "position"); // Get the Location of the "position" Shader Variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and Set Shader for VBO_COLORS_TRIANGLE
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS_TRIANGLE);
	loc = glGetAttribLocation(program, "color"); // Get the Location of the "color" Shader Variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// Generate and Bind IBO_TRIANGLE
	glGenBuffers(1, &IBO_TRIANGLE);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_TRIANGLE);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TRIANGLE_INDICES.size() * sizeof(GLuint), &TRIANGLE_INDICES[0], GL_STATIC_DRAW);

	/***************************************
	WIREFRAME VAO VBOs (For Wireframe Mode)
	***************************************/

	// Generate and Bind VAO_WIREFRAME
	glGenVertexArrays(1, &VAO_WIREFRAME);
	glBindVertexArray(VAO_WIREFRAME);

	// Generate and Bind VBO_POSITION_WIREFRAME
	glGenBuffers(1, &VBO_POSITION_WIREFRAME);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION_WIREFRAME);
	glBufferData(GL_ARRAY_BUFFER, WIREFRAME_VERTICES.size() * sizeof(GLfloat), &WIREFRAME_VERTICES[0], GL_STATIC_DRAW);

	// Generate and Bind VBO_COLORS_WIREFRAME
	glGenBuffers(1, &VBO_COLORS_WIREFRAME);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS_WIREFRAME);
	glBufferData(GL_ARRAY_BUFFER, WIREFRAME_COLORS.size() * sizeof(GLfloat), &WIREFRAME_COLORS[0], GL_STATIC_DRAW);

	// Enable and Set Shader for VBO_POSITION_WIREFRAME
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION_WIREFRAME);
	loc = glGetAttribLocation(program, "position"); // Get the Location of the "position" Shader Variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and Set Shader for VBO_COLORS_WIREFRAME
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS_WIREFRAME);
	loc = glGetAttribLocation(program, "color"); // Get the Location of the "color" Shader Variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/***************************************************
	HYBRID VAO VBOs (For Wireframe Overlaying Triangle)
	***************************************************/

	// Generate and Bind VAO_COLOR
	glGenVertexArrays(1, &VAO_HYBRID);
	glBindVertexArray(VAO_HYBRID);

	// Generate and Bind VBO_POSITION_HYBRID
	glGenBuffers(1, &VBO_POSITION_HYBRID);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION_HYBRID);
	glBufferData(GL_ARRAY_BUFFER, WIREFRAME_VERTICES.size() * sizeof(GLfloat), &WIREFRAME_VERTICES[0], GL_STATIC_DRAW);

	// Generate and Bind VBO_COLORS_HYBRID
	glGenBuffers(1, &VBO_COLORS_HYBRID);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS_HYBRID);
	glBufferData(GL_ARRAY_BUFFER, HYBRID_COLORS.size() * sizeof(GLfloat), &HYBRID_COLORS[0], GL_STATIC_DRAW);

	// Enable and Set Shader for VBO_POSITION_HYBRID
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION_HYBRID);
	loc = glGetAttribLocation(program, "position"); // Get the Location of the "position" Shader Variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and Set Shader for VBO_COLORS_HYBRID
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS_HYBRID);
	loc = glGetAttribLocation(program, "color"); // Get the Location of the "color" Shader Variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/***************************************************************************************************
	COLOR VAO VBOs (For Coloring Vertices Based on Color Values Taken From Another Image of Equal Size)
	***************************************************************************************************/

	// Generate and Bind VAO_COLOR
	glGenVertexArrays(1, &VAO_COLOR);
	glBindVertexArray(VAO_COLOR);

	// Generate and Bind VBO_POSITION_COLOR
	glGenBuffers(1, &VBO_POSITION_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION_COLOR);
	glBufferData(GL_ARRAY_BUFFER, TRIANGLE_VERTICES.size() * sizeof(GLfloat), &TRIANGLE_VERTICES[0], GL_STATIC_DRAW);

	// Generate and Bind VBO_COLORS_COLOR
	glGenBuffers(1, &VBO_COLORS_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS_COLOR);
	glBufferData(GL_ARRAY_BUFFER, COLOR_COLORS.size() * sizeof(GLfloat), &COLOR_COLORS[0], GL_STATIC_DRAW);

	// Enable and Set Shader for VBO_POSITION_COLOR
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION_COLOR);
	loc = glGetAttribLocation(program, "position"); // Get the Location of the "position" Shader Variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and Set Shader for VBO_COLORS_COLOR
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS_COLOR);
	loc = glGetAttribLocation(program, "color"); // Get the Location of the "color" Shader Variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// Generate and Bind IBO_COLOR
	glGenBuffers(1, &IBO_COLOR);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_COLOR);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TRIANGLE_INDICES.size() * sizeof(GLuint), &TRIANGLE_INDICES[0], GL_STATIC_DRAW);
}

// Initialize Milesione Vertices and VAO/VBO
void initializeMilestone() {
	// Set Number of Verices, Vertex Posions and Vertex Colors (RGBA)
	const int numVertices = 3;
	float vertices[3 * numVertices] = {
		0.0, 0.0, -1.0,
		1.0, 0.0, -1.0,
		0.0, 1.0, -1.0
	};
	float colors[4 * numVertices] = {
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0
	};

	// Generate and Bind VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Generate and Bind VBO_POSITION
	glGenBuffers(1, &VBO_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Generate and Bind VBO_COLORS
	glGenBuffers(1, &VBO_COLORS);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

	// Enable and Set Shader for VBO_POSITION
	glBindBuffer(GL_ARRAY_BUFFER, VBO_POSITION);
	GLuint loc = glGetAttribLocation(program, "position"); //get the location of the "position" shader variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and Set Shader for VBO_COLORS
	glBindBuffer(GL_ARRAY_BUFFER, VBO_COLORS);
	loc = glGetAttribLocation(program, "color"); //get the location of the "color" shader variable
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

// Display According to Display Mode
void displayFunc(){

	// Clear Color and Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Bind the Pipeline Program
	if(shaderNumber == 1)
		pipelineProgram.Bind();
	else pipelineProgramColor.Bind();

	// Initalize Modelview and Perspective Matrices
	initializeMatrix();

	switch (displayType) {
		case DISPLAY_POINTS:
			glBindVertexArray(VAO_POINT);
			glDrawArrays(GL_POINTS, 0, POINT_VERTICES.size() / 3);
			break;

		case DISPLAY_WIREFRAME:
			glBindVertexArray(VAO_WIREFRAME);
			glDrawArrays(GL_LINES, 0, WIREFRAME_VERTICES.size() / 3);
			break;

		case DISPLAY_TRIANGLES:
			glBindVertexArray(VAO_TRIANGLE);
			//glDrawArrays(GL_TRIANGLES, 0, TRIANGLE_VERTICES.size() / 3);
			glDrawElements(GL_TRIANGLES, TRIANGLE_INDICES.size(), GL_UNSIGNED_INT, (void*)0);
			break;

		case DISPLAY_WIREFRAME_ON_TRIANGLE:

			// Set Polygon Mode to Draw Lines Front and Back
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			// Allow an Offset Added to Depth Values of a Polygon's Fragments Before the Depth Comparison is Performed
			glEnable(GL_POLYGON_OFFSET_LINE);
			// Avoid Z-Fighting
			glPolygonOffset(-1.0, -1.0);

			glBindVertexArray(VAO_HYBRID);
			glDrawArrays(GL_LINES, 0, WIREFRAME_VERTICES.size() / 3);

			// Disable Offset Added
			glDisable(GL_POLYGON_OFFSET_LINE);
			// Set Polygon Mode to Filled Polygon Front and Back
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glBindVertexArray(VAO_TRIANGLE);
			//glDrawArrays(GL_TRIANGLES, 0, TRIANGLE_VERTICES.size() / 3);
			glDrawElements(GL_TRIANGLES, TRIANGLE_INDICES.size(), GL_UNSIGNED_INT, (void*)0);
			break;

		case DISPLAY_IMAGE_ON_TRIANGLE:
			glBindVertexArray(VAO_COLOR);
			//glDrawArrays(GL_TRIANGLES, 0, TRIANGLE_VERTICES.size() / 3);
			glDrawElements(GL_TRIANGLES, TRIANGLE_INDICES.size(), GL_UNSIGNED_INT, (void*)0);
			break;

		case DISPLAY_MILESTONE:
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			break;

	}

	// Unbind VAO
	glBindVertexArray(0);	

	// Swap Buffers
	glutSwapBuffers();
}

// Animation at the Beginning
void idleFunc(){
	// for example, here, you can save the screenshots to disk (to make the animation)
	// Animation Keeps Rotating
	/*if (screenshotCounter < 300) {
		if (screenshotCounter < 60) {
			displayType = DISPLAY_WIREFRAME;
			// Move in a Rotate Form
			if (screenshotCounter < 10) {
				landTranslate[0] += 4.0f;
			}
			else if (screenshotCounter < 20) {
				landTranslate[1] += 4.0f;
			}
			else if (screenshotCounter < 30) {
				landTranslate[0] -= 8.0f;
			}
			else if (screenshotCounter < 40) {
				landTranslate[1] -= 8.0f;
			}
			else if (screenshotCounter < 50) {
				landTranslate[0] += 4.0f;
				landTranslate[2] += 2.0f;
			}
			else {
				landTranslate[1] += 4.0f;
				landTranslate[2] -= 2.0f;
			}
			landRotate[1] += (float)(360 / 60);
		}
		else if (screenshotCounter < 160) {
			displayType = DISPLAY_POINTS;
			shaderNumber = -1;
			// Zoom In
			landScale[0] += 0.025f;
			landScale[1] += 0.2f;
			// Rotate y
			if (screenshotCounter < 110) {
				landRotate[0] += (float)(360 / 50);
			}
			landRotate[1] += (float)(360 / 100);
		}
		else if (screenshotCounter < 220) {
			displayType = DISPLAY_TRIANGLES;
			// Zoom Out
			landScale[0] -= 0.042f;
			landScale[1] -= 0.33f;
			// Rotate z
			shaderNumber = -1;
			if (screenshotCounter < 190) {
				landRotate[2] += (float)(360 / 30);
			}
			landRotate[1] += (float)(360 / 60);
		}
		else if (screenshotCounter < 260) {
			displayType = DISPLAY_WIREFRAME_ON_TRIANGLE;
			shaderNumber = (screenshotCounter % 4 == 0) ? -shaderNumber : shaderNumber ;
			// Zoom In and Zoom Out
			if (screenshotCounter < 240) {
				landScale[0] += 0.1f;
				landScale[1] += 0.1f;
				landScale[2] += 0.1f;
			}
			else {
				landScale[0] -= 0.1f;
				landScale[1] -= 0.1f;
				landScale[2] -= 0.1f;
			}
			landRotate[1] += (float)(360 / 40);
		}
		else {
			displayType = DISPLAY_IMAGE_ON_TRIANGLE;
			shaderNumber = 1;
			// Zoom Out
			if (screenshotCounter < 280) {
				landScale[0] += 0.02f;
				landScale[1] += 0.02f;
				landScale[2] += 0.02f;
			}
			else {
				displayType = DISPLAY_TRIANGLES;
			}
			landRotate[1] += (float)(360 / 40);
		}
		//take a screenshot
		char anim_num[5];
		sprintf(anim_num, "%03d", ++screenshotCounter);
		saveScreenshot(("./animation/" + string(anim_num) + ".jpg").c_str());
	}*/
	// Make the Screen Ipdate 
	glutPostRedisplay();
}

// Set Viewport, Prepare Projection Matrix and Set Back to Modelview Mode in the End of the Function
void reshapeFunc(int w, int h){
	glViewport(0, 0, w, h);

	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	// Load Identity Matrix
	matrix.LoadIdentity();
	// Set Prespective to Field of View = 45 degrees, Aspect Ratio = 1280:720, zNear = 0.01, zFar = 1000
	matrix.Perspective(fovy, aspect, 0.01, 1000.0);
	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
}

void mouseMotionDragFunc(int x, int y){
	// Mouse has Moved and One of the Mouse Buttons is Pressed (Dragging)
	// The Change in Mouse Position Since the Last Invocation of This Function
	int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

	switch (controlState){
		// Translate the Landscape
		case TRANSLATE:
			if (leftMouseButton){
				// Control x, y Translation via the Left Mouse Button
				landTranslate[0] += mousePosDelta[0] * 0.01f;
				landTranslate[1] -= mousePosDelta[1] * 0.01f;
			}
			if (middleMouseButton){
				// Control z Translation via the Middle Mouse Button
				landTranslate[2] += mousePosDelta[1] * 0.01f;
			}
			break;

		// Rotate the Landscape
		case ROTATE:
			if (leftMouseButton){
				// Control x, y Rotation via the Left Mouse Button
				landRotate[0] += mousePosDelta[1];
				landRotate[1] += mousePosDelta[0];
			}
			if (middleMouseButton){
				// control z Rotation via the Middle Mouse Button
				landRotate[2] += mousePosDelta[1];
			}
			break;

		// Scale the Landscape
		case SCALE:
			if (leftMouseButton){
				// Control x, y Scaling via the Left Mouse Button
				landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
				landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
			}
			if (middleMouseButton){
				// Control z Scaling via the Middle Mouse Button
				landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
			}
			break;
	}

	// Store the New Mouse Position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseMotionFunc(int x, int y){
	// Mouse has Moved
	// Store the New Mouse Position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y){
	// A Mouse Button has been Pressed or Unpressed

	// Keep Track of the Mouse Button State, in leftMouseButton, middleMouseButton, rightMouseButton Variables
	switch (button){
		case GLUT_LEFT_BUTTON:
			leftMouseButton = (state == GLUT_DOWN);
			break;

		case GLUT_MIDDLE_BUTTON:
			middleMouseButton = (state == GLUT_DOWN);
			break;

		case GLUT_RIGHT_BUTTON:
			rightMouseButton = (state == GLUT_DOWN);
			break;
	}

	// Keep Track of Whether CTRL and SHIFT keys are pressed
	switch (glutGetModifiers()){
		// If CTRL is Pressed, We are in Translate Mode
		case GLUT_ACTIVE_CTRL:
			controlState = TRANSLATE;
			break;

		// If SHIFT is Pressed, We are in Scale Mode
		case GLUT_ACTIVE_SHIFT:
			controlState = SCALE;
			break;

		// If CTRL and SHIFT are Not Pressed, We are in Rotate Mode
		default:
			controlState = ROTATE;
			break;
	}

	// Store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y){
	cout << (int)key << endl;
	switch (key){
		case 27: // ESC key
			exit(0); // Exit the program
			break;

		case ' ':
			cout << "You pressed the spacebar." << endl;
			break;

		case 'x':
			// Take a Screenshot
			saveScreenshot("screenshot.jpg");
			break;

		case '1':
			// 1 for Points Mode (Default)
			displayType = DISPLAY_POINTS;
			break;

		case '2':
			// 2 for Wireframe Mode
			displayType = DISPLAY_WIREFRAME;
			break;

		case '3':
			// 3 for Triangle Mode
			displayType = DISPLAY_TRIANGLES;
			break;

		case '4':
			// 4 for Wireframe overlay Traingle Mode
			displayType = DISPLAY_WIREFRAME_ON_TRIANGLE;
			break;

		case '5':
			// 5 for Coloring Triangle Vertices with Other Color Image
			displayType = DISPLAY_IMAGE_ON_TRIANGLE;
			break;

		case '6':
			// 6 for Changing Shader to Use
			shaderNumber = -shaderNumber;
			break;

		// a, d for x Translate; w, s for y Translate; q, e for z Translate
		case 'a':
			landTranslate[0] += 10.0f;
			break;

		case 'd':
			landTranslate[0] -= 10.0f;
			break;

		case 'w':
			landTranslate[1] += 10.0f;
			break;

		case 's':
			landTranslate[1] -= 10.0f;
			break;

		case 'q':
			landTranslate[2] += 10.0f;
			break;

		case 'e':
			landTranslate[2] -= 10.0f;
			break;

		// SHIFT + a, d for x Translate; w, s for y Translate; q, e for z Translate
		case 'A':
			landScale[0] += 1.0f;
			break;

		case 'D':
			landScale[0] -= 1.0f;
			break;

		case 'W':
			landScale[1] += 1.0f;
			break;

		case 'S':
			landScale[1] -= 1.0f;
			break;

		case 'Q':
			landScale[2] += 1.0f;
			break;

		case 'E':
			landScale[2] -= 1.0f;
			break;

		// CTRL + a, d for x Translate; w, s for y Translate; q, e for z Translate
		case 1:
			landRotate[0] += 1.0f;
			break;

		case 4:
			landRotate[0] -= 1.0f;
			break;

		case 23:
			landRotate[1] += 1.0f;
			break;

		case 19:
			landRotate[1] -= 1.0f;
			break;

		case 17:
			landRotate[2] += 1.0f;
			break;

		case 5:
			landRotate[2] -= 1.0f;
			break;
		
		case 'm':
			displayType = DISPLAY_MILESTONE;
			break;
  }
}

void initScene(int argc, char *argv[]){

	// Load heightmapImage From a JPEG Disk File to Main Memory
	heightmapImage = new ImageIO();
	if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK){
		cout << "Error reading image " << argv[1] << "." << endl;
		exit(EXIT_FAILURE);
	}
	// Load Color Image if heightmapImage size is following sizes;
	colorImage = new ImageIO();
	if (heightmapImage->getHeight() == 128 && heightmapImage->getWidth() == 128) {
		if (colorImage->loadJPEG("heightmap/color128.jpg") != ImageIO::OK) {
			cout << "Error reading image heightmap/color256.jpg ." << endl;
			exit(EXIT_FAILURE);
		}
	}
	else if (heightmapImage->getHeight() == 256 && heightmapImage->getWidth() == 256) {
		if (colorImage->loadJPEG("heightmap/color256.jpg") != ImageIO::OK) {
			cout << "Error reading image heightmap/color256.jpg ." << endl;
			exit(EXIT_FAILURE);
		}
	}
	else if (heightmapImage->getHeight() == 512 && heightmapImage->getWidth() == 512) {
		if (colorImage->loadJPEG("heightmap/color512.jpg") != ImageIO::OK) {
			cout << "Error reading image heightmap/color256.jpg ." << endl;
			exit(EXIT_FAILURE);
		}
	}
	else if (heightmapImage->getHeight() == 768 && heightmapImage->getWidth() == 768) {
		if (colorImage->loadJPEG("heightmap/color768.jpg") != ImageIO::OK) {
			cout << "Error reading image heightmap/color256.jpg ." << endl;
			exit(EXIT_FAILURE);
		}
	}

	// Set Background to Black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable Depth Test
	glEnable(GL_DEPTH_TEST);

	// Initialize and Get Handle of the Pipeline Program of 2 Shaders
	pipelineProgram.Init("../openGLHelper-starterCode", "basic.vertexShader.glsl", "basic.fragmentShader.glsl");
	program = pipelineProgram.GetProgramHandle();

	pipelineProgramColor.Init("../openGLHelper-starterCode", "basic.vertexShader.glsl", "basic.fragmentShaderColor.glsl");
	programColor = pipelineProgramColor.GetProgramHandle();

	// Get Uniform Matrix Location
	h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
	h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");

	// Initalize Milestone 
	initializeMilestone();

	// Initailze heightmapImage Vertices According to Its Number of Channels
	if (heightmapImage->getBytesPerPixel() == 1) {
		initializeVertices();
	}
	else if (heightmapImage->getBytesPerPixel() == 3) {
		initializeVerticesRGB();
	}

	// Initialize Overlay Color Image
	initializeColorImage();

	// Initialize all VAOs and VBOs
	initilaizeVBOs();
}

int main(int argc, char *argv[])
{
	if (argc != 2){
		cout << "The arguments are incorrect." << endl;
		cout << "usage: ./hw1 <heightmap file>" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Initializing GLUT..." << endl;
	glutInit(&argc,argv);

	cout << "Initializing OpenGL..." << endl;

	#ifdef __APPLE__
		glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	#else
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	#endif

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);  
	glutCreateWindow(windowTitle);

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// tells glut to use a particular display function to redraw 
	glutDisplayFunc(displayFunc);
	// perform animation inside idleFunc
	glutIdleFunc(idleFunc);
	// callback for mouse drags
	 glutMotionFunc(mouseMotionDragFunc);
	// callback for idle mouse movement
	 glutPassiveMotionFunc(mouseMotionFunc);
	// callback for mouse button changes
	glutMouseFunc(mouseButtonFunc);
	// callback for resizing the window
	glutReshapeFunc(reshapeFunc);
	// callback for pressing the keys on the keyboard
	glutKeyboardFunc(keyboardFunc);

	// init glew
	#ifdef __APPLE__
		// nothing is needed on Apple
	#else
		// Windows, Linux
		GLint result = glewInit();
		if (result != GLEW_OK){
			cout << "error: " << glewGetErrorString(result) << endl;
			exit(EXIT_FAILURE);
		}
	#endif

	// do initialization
	initScene(argc, argv);

	// sink forever into the glut loop
	glutMainLoop();
}


