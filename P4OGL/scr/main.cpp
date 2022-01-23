#include "BOX.h"
#include "auxiliar.h"
#include "PLANE.h"

#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>

#define RAND_SEED 31415926
#define SCREEN_SIZE 500,500

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
float angle = 0.0f;

//VAO
unsigned int vao;

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

unsigned int colorTexId;
unsigned int emiTexId;

//VAO y VBO del plano de renderizado
unsigned int planeVAO;
unsigned int planeVertexVBO;

//Variables Uniform 
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

//Convolution Filter Uniform Variables
int uMask3x3;
int uMask5x5;
int uMaskVEdges3x3;
int uMaskHEdges3x3;

//Texturas Uniform
int uColorTex;
int uEmiTex;

//Atributos
int inPos;
int inColor;
int inNormal;
int inTexCoord;

// Forward Render shaders
unsigned int vshader;
unsigned int fshader;
unsigned int program;

// Post processing shaders
const int numPostProcessPrograms = 5;
unsigned int postProcessVShaders[numPostProcessPrograms];
unsigned int postProcessFShaders[numPostProcessPrograms];
unsigned int postProcessPrograms[numPostProcessPrograms];

//Uniform
unsigned int uColorTexPP[numPostProcessPrograms];
unsigned int uDepthTexPP[numPostProcessPrograms];
unsigned int fbo;
unsigned int colorBuffTexId;
unsigned int depthBuffTexId;

bool isOnMotionBlur = false;
bool isOnNegativeColor = false;
bool isOnFilterGauss3x3 = false; 
const float maskFactorGauss3x3 = float(1.0f / 16.0f);
const float maskGauss3x3[9] = {
								float(1.0f * maskFactorGauss3x3), float(2.0f * maskFactorGauss3x3), float(1.0f * maskFactorGauss3x3),
								float(2.0f * maskFactorGauss3x3), float(4.0f * maskFactorGauss3x3), float(2.0f * maskFactorGauss3x3),
								float(1.0f * maskFactorGauss3x3), float(2.0f * maskFactorGauss3x3), float(1.0f * maskFactorGauss3x3)
							  };

bool isOnFilterGauss5x5 = false;
const float maskFactorGauss5x5 = float(1.0f / 273.0f);
const float maskGauss5x5[25] = {
								float(1.0f * maskFactorGauss5x5), float(4.0f * maskFactorGauss5x5),  float(7.0f * maskFactorGauss5x5),  float(4.0f * maskFactorGauss5x5),  float(1.0f * maskFactorGauss5x5),
								float(4.0f * maskFactorGauss5x5), float(16.0f * maskFactorGauss5x5), float(26.0f * maskFactorGauss5x5), float(16.0f * maskFactorGauss5x5), float(4.0f * maskFactorGauss5x5),
								float(7.0f * maskFactorGauss5x5), float(26.0f * maskFactorGauss5x5), float(41.0f * maskFactorGauss5x5), float(26.0f * maskFactorGauss5x5), float(7.0f * maskFactorGauss5x5),
								float(4.0f * maskFactorGauss5x5), float(16.0f * maskFactorGauss5x5), float(26.0f * maskFactorGauss5x5), float(16.0f * maskFactorGauss5x5), float(4.0f * maskFactorGauss5x5),
								float(1.0f * maskFactorGauss5x5), float(4.0f * maskFactorGauss5x5),  float(7.0f * maskFactorGauss5x5),  float(4.0f * maskFactorGauss5x5),  float(1.0f * maskFactorGauss5x5)
							   };

bool isOnFilterVerticalEdges = false;
const float maskVertical[9] = {
								float(1.0f),  float(0.0f), float(-1.0f),
								float(1.0f),  float(0.0f), float(-1.0f),
								float(1.0f),  float(0.0f), float(-1.0f)
							  };

bool isOnFilterHorizontalEdges = false;
const float maskHorizontal[9] = {
								float(1.0f),  float(1.0f), float(1.0f),
								float(0.0f),  float(0.0f), float(0.0f),
								float(-1.0f), float(-1.0f), float(-1.0f)
								};

//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

void renderCube();

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShaderFw(const char *vname, const char *fname);
void initObj();
void destroy();

//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);

//////////////////////////////////////////////////////////////
// Nuevas variables auxiliares
//////////////////////////////////////////////////////////////
void initPlane();
void initShaderPP(const char* vname, const char* fname, int ppProgram_i, unsigned int& ppVshader, unsigned int& ppFshader);
void initFBO();
void resizeFBO(unsigned int w, unsigned int h);

//////////////////////////////////////////////////////////////
// Nuevas funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));

	initContext(argc, argv);
	initOGL();
	initShaderFw("../shaders_P4/fwRendering.vert", "../shaders_P4/fwRendering.frag");
	initObj();
	
	initShaderPP("../shaders_P4/postProcessing.vert", "../shaders_P4/postProcessing.frag", 0, postProcessVShaders[0], postProcessFShaders[0]);
	initShaderPP("../shaders_P4/postProcessingGauss3x3.vert", "../shaders_P4/postProcessingGauss3x3.frag", 1, postProcessVShaders[1], postProcessFShaders[1]);
	initShaderPP("../shaders_P4/postProcessingGauss5x5.vert", "../shaders_P4/postProcessingGauss5x5.frag", 2, postProcessVShaders[2], postProcessFShaders[2]);
	initShaderPP("../shaders_P4/postProcessingVerticalEdges.vert", "../shaders_P4/postProcessingVerticalEdges.frag", 3, postProcessVShaders[3], postProcessFShaders[3]);
	initShaderPP("../shaders_P4/postProcessingHorizontalEdges.vert", "../shaders_P4/postProcessingHorizontalEdges.frag", 4, postProcessVShaders[4], postProcessFShaders[4]);
	initPlane();
	initFBO();
	resizeFBO(SCREEN_SIZE);

	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_SIZE);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas GLSL");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}

	const GLubyte *oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 1.0f, 50.0f);

	view = glm::mat4(1.0f);
	view[3].z = -25.0f;
}

void destroy()
{
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);

	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);
}

void initShaderFw(const char* vname, const char* fname)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");

	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");

	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");
}

void initShaderPP(const char* vname, const char* fname, int ppProgram_i, unsigned int& ppVshader, unsigned int& ppFshader)
{
	ppVshader = loadShader(vname, GL_VERTEX_SHADER);
	ppFshader = loadShader(fname, GL_FRAGMENT_SHADER);

	postProcessPrograms[ppProgram_i] = glCreateProgram();
	glAttachShader(postProcessPrograms[ppProgram_i], ppVshader);
	glAttachShader(postProcessPrograms[ppProgram_i], ppFshader);
	glLinkProgram(postProcessPrograms[ppProgram_i]);

	int linked;
	glGetProgramiv(postProcessPrograms[ppProgram_i], GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(postProcessPrograms[ppProgram_i], GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(postProcessPrograms[ppProgram_i], logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete logString;
		glDeleteProgram(postProcessPrograms[ppProgram_i]);
		postProcessPrograms[ppProgram_i] = 0;
		exit(-1);
	}

	uColorTexPP[ppProgram_i] = glGetUniformLocation(postProcessPrograms[ppProgram_i], "colorTex");
	uDepthTexPP[ppProgram_i] = glGetUniformLocation(postProcessPrograms[ppProgram_i], "depthTex");

	if (ppProgram_i == 1) uMask3x3 = glGetUniformLocation(postProcessPrograms[ppProgram_i], "mask3x3");
	if (ppProgram_i == 2) uMask5x5 = glGetUniformLocation(postProcessPrograms[ppProgram_i], "mask5x5");
	if (ppProgram_i == 3) uMaskVEdges3x3 = glGetUniformLocation(postProcessPrograms[ppProgram_i], "maskVE3x3");
	if (ppProgram_i == 4) uMaskHEdges3x3 = glGetUniformLocation(postProcessPrograms[ppProgram_i], "maskHE3x3");

	if (0 != glGetAttribLocation(postProcessPrograms[ppProgram_i], "inPos"))
		exit(-1);
}

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}

	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 2,
			cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		cubeNTriangleIndex*sizeof(unsigned int) * 3, cubeTriangleIndex,
		GL_STATIC_DRAW);

	model = glm::mat4(1.0f);

	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
}

GLuint loadShader(const char *fileName, GLenum type)
{
	unsigned int fileLen;
	char *source = loadStringFromFile(fileName, fileLen);

	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar **)&source, (const GLint *)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compilo bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteShader(shader);
		exit(-1);
	}

	return shader;
}

unsigned int loadTex(const char *fileName)
{
	unsigned char *map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);

	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, (GLvoid*)map);
	delete[] map;
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}

void renderFunc()
{
	//Activar el fbo
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	//Texturas
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}

	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}

	model = glm::mat4(2.0f);
	model[3].w = 1.0f;
	model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));
	renderCube();

	std::srand(RAND_SEED);
	for (unsigned int i = 0; i < 10; i++)
	{
		float size = float(std::rand() % 3 + 1);

		glm::vec3 axis(glm::vec3(float(std::rand() % 2),
			float(std::rand() % 2), float(std::rand() % 2)));
		if (glm::all(glm::equal(axis, glm::vec3(0.0f))))
			axis = glm::vec3(1.0f);

		float trans = float(std::rand() % 7 + 3) * 1.00f + 0.5f;
		glm::vec3 transVec = axis * trans;
		transVec.x *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.y *= (std::rand() % 2) ? 1.0f : -1.0f;
		transVec.z *= (std::rand() % 2) ? 1.0f : -1.0f;

		model = glm::rotate(glm::mat4(1.0f), angle*2.0f*size, axis);
		model = glm::translate(model, transVec);
		model = glm::rotate(model, angle*2.0f*size, axis);
		model = glm::scale(model, glm::vec3(1.0f / (size*0.7f)));
		renderCube();
	}

	//Desactivar el fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Desactivamos test de cull y profundidad para todos los shaders de post procesado
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	//Pulsando la tecla 5 se determina si se aplica motion blur o no
	if (isOnMotionBlur) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA);
		glBlendColor(0.5f, 0.5f, 0.5f, 0.6f);
		glBlendEquation(GL_FUNC_ADD);
	}
	//Pulsando la tecla 5 se determina si se aplica la negación de color o no
	if (isOnNegativeColor) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ZERO);
		glBlendEquation(GL_FUNC_SUBTRACT);
	}

	if (!isOnFilterGauss3x3 && !isOnFilterGauss5x5 && !isOnFilterVerticalEdges && !isOnFilterHorizontalEdges) {  //Caso de post-procesado sin filtros
		glUseProgram(postProcessPrograms[0]);

		if (uColorTexPP[0] != -1)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
			glUniform1i(uColorTexPP[0], 0);
		}
		if (uDepthTexPP[0] != -1)
		{
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
			glUniform1i(uDepthTexPP[0], 1);
		}
	}
	else {  //Casos de post-procesado con filtros que se pueden aplicar de forma aditiva por teclado
		if (isOnFilterGauss3x3) {  //Filtrado Gaussiano 3x3
			glUseProgram(postProcessPrograms[1]);

			glUniform1fv(uMask3x3, 9, maskGauss3x3);

			if (uColorTexPP[1] != -1)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
				glUniform1i(uColorTexPP[1], 0);
			}
			if (uDepthTexPP[1] != -1)
			{
				glActiveTexture(GL_TEXTURE0 + 1);
				glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
				glUniform1i(uDepthTexPP[1], 1);
			}
		}
		if (isOnFilterGauss5x5) {  //Filtrado Gaussiano 5x5
			glUseProgram(postProcessPrograms[2]);

			glUniform1fv(uMask5x5, 25, maskGauss5x5);

			if (uColorTexPP[2] != -1)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
				glUniform1i(uColorTexPP[2], 0);
			}
			if (uDepthTexPP[2] != -1)
			{
				glActiveTexture(GL_TEXTURE0 + 1);
				glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
				glUniform1i(uDepthTexPP[2], 1);
			}
		}
		if (isOnFilterVerticalEdges) {  //Filtrado de Bordes Verticales
			glUseProgram(postProcessPrograms[3]);

			glUniform1fv(uMaskVEdges3x3, 9, maskVertical);

			if (uColorTexPP[3] != -1)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
				glUniform1i(uColorTexPP[3], 0);
			}
			if (uDepthTexPP[3] != -1)
			{
				glActiveTexture(GL_TEXTURE0 + 1);
				glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
				glUniform1i(uDepthTexPP[3], 1);
			}
		}
		if (isOnFilterHorizontalEdges) {  //Filtrado de Bordes Horizontales
			glUseProgram(postProcessPrograms[4]);

			glUniform1fv(uMaskHEdges3x3, 9, maskHorizontal);

			if (uColorTexPP[4] != -1)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
				glUniform1i(uColorTexPP[4], 0);
			}
			if (uDepthTexPP[4] != -1)
			{
				glActiveTexture(GL_TEXTURE0 + 1);
				glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
				glUniform1i(uDepthTexPP[4], 1);
			}
		}
	}

	//Se dibuja la textura en el plano de proyección
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	//Se desactiva el blending y se reactiva el cálculo de culling y el test de profundidad para el siguiente fwRendering step
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glutSwapBuffers();
}

void renderCube()
{
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
		&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
		&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
		&(normal[0][0]));
	
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
		GL_UNSIGNED_INT, (void*)0);
}

void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) /float(height), 1.0f, 50.0f);

	resizeFBO(width, height);

	glutPostRedisplay();
}

void idleFunc()
{
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.02f;
	
	glutPostRedisplay();
}

void initPlane()
{
	glGenBuffers(1, &planeVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, planeNVertex * sizeof(float) * 3,
		planeVertexPos, GL_STATIC_DRAW);

	glGenVertexArrays(1, &planeVAO);
	glBindVertexArray(planeVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

void initFBO()
{
	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &colorBuffTexId);
	glGenTextures(1, &depthBuffTexId);
}

void resizeFBO(unsigned int w, unsigned int h)
{
	glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
		GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0,
		GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, colorBuffTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		depthBuffTexId, 0);

	const GLenum buffs[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffs);
	
	if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		std::cerr << "Error configurando el FBO" << std::endl;
		exit(-1);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
		case '1':  //Filtro Gaussiano 3x3
			isOnFilterGauss3x3 = !isOnFilterGauss3x3;
			isOnFilterGauss5x5 = false;
			isOnFilterVerticalEdges = false;
			isOnFilterHorizontalEdges = false;
			break;
		case '2':  //Filtro Gaussiano 5x5
			isOnFilterGauss5x5 = !isOnFilterGauss5x5;
			isOnFilterGauss3x3 = false;
			isOnFilterVerticalEdges = false;
			isOnFilterHorizontalEdges = false;
			break;
		case '3':  //Filtro Bordes Verticales
			isOnFilterVerticalEdges = !isOnFilterVerticalEdges;
			isOnFilterGauss3x3 = false;
			isOnFilterGauss5x5 = false;
			isOnFilterHorizontalEdges = false;
			break;
		case '4':  //Filtro Bordes Horizontales
			isOnFilterHorizontalEdges = !isOnFilterHorizontalEdges;
			isOnFilterGauss3x3 = false;
			isOnFilterGauss5x5 = false;
			isOnFilterVerticalEdges = false;
			break;
		case '5':  //Filtro Bordes Horizontales
			isOnMotionBlur = !isOnMotionBlur;
			isOnNegativeColor = false;
			break;
		case '6':  //Filtro Bordes Horizontales
			isOnNegativeColor = !isOnNegativeColor;
			isOnMotionBlur = false;
			break;
	}
}

void mouseFunc(int button, int state, int x, int y)
{
	if (state == 0)
		std::cout << "Se ha pulsado el botón ";
	else
		std::cout << "Se ha soltado el botón ";

	if (button == 0) std::cout << "de la izquierda del ratón " << std::endl;
	if (button == 1) std::cout << "central del ratón " << std::endl;
	if (button == 2) std::cout << "de la derecha del ratón " << std::endl;

	std::cout << "en la posición " << x << " " << y << std::endl << std::endl;
}