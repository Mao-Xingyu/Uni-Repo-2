#include "utilities.h"
#include "Texture.h"
#include "SimpleModel.h"

// global variables
// settings
unsigned int gWindowWidth = 800;
unsigned int gWindowHeight = 600;

// frame stats
float gFrameRate = 60.0f;
float gFrameTime = 1 / gFrameRate;
static float rotateW;
static float rotateR;
static float yaw;
static float pitch;
bool ringRotate = false;


// scene content
std::map<std::string, ShaderProgram> gShader;	// shader program objects
GLuint gVBO[10];		// vertex buffer object identifier
GLuint gVAO[10];		// vertex array object identifier
GLuint gIBO[10];

std::map<std::string, glm::mat4> gViewMatrix;			// view matrix
std::map<std::string, glm::mat4> gProjectionMatrix;		// projection matrices
std::map<std::string, glm::mat4> gModelMatrix;			// object matrix
std::map<std::string, glm::mat4> MVP;
std::map<std::string, glm::mat3> normalMatrix;

Light gLight;					// light properties
std::map<std::string, Material> gMaterial;				// material properties
std::map<std::string, Texture> gTexture;	// texture objects
SimpleModel gModel;

static glm::vec3 basePos = glm::vec3(0.0f, 0.0f, 0.0f);
// controls
bool gWireframe = false;	// wireframe control

//Ground
// vertex positions, normals and texture coordinates
std::vector<GLfloat> verticesG =
{
	-5.0f, 0.0f, 5.0f,	// vertex 0: position
	0.0f, 1.0f, 0.0f,	// vertex 0: normal
	0.0f, 0.0f,			// vertex 0: texture coordinate
	5.0f, 0.0f, 5.0f,	// vertex 1: position
	0.0f, 1.0f, 0.0f,	// vertex 1: normal
	10.0f, 0.0f,		// vertex 1: texture coordinate
	-5.0f, 0.0f, -5.0f,	// vertex 2: position
	0.0f, 1.0f, 0.0f,	// vertex 2: normal
	0.0f, 10.0f,		// vertex 2: texture coordinate
	5.0f, 0.0f, -5.0f,	// vertex 3: position
	0.0f, 1.0f, 0.0f,	// vertex 3: normal
	10.0f, 10.0f,		// vertex 3: texture coordinate
};

//Wall
// vertex positions, normals, tangents and texture coordinates
std::vector<GLfloat> verticesW =
{
		-8.0f, -5.0f, 0.0f,	// vertex 0: position
		0.0f, 0.0f, 1.0f,	// vertex 0: normal
		1.0f, 0.0f, 0.0f,	// vertex 0: tangent
		0.0f, 0.0f,			// vertex 0: texture coordinate
		8.0f, -5.0f, 0.0f,	// vertex 1: position
		0.0f, 0.0f, 1.0f,	// vertex 1: normal
		1.0f, 0.0f, 0.0f,	// vertex 1: tangent
		4.0f, 0.0f,			// vertex 1: texture coordinate
		-8.0f, 5.0f, 0.0f,	// vertex 2: position
		0.0f, 0.0f, 1.0f,	// vertex 2: normal
		1.0f, 0.0f, 0.0f,	// vertex 2: tangent
		0.0f, 4.0f,			// vertex 2: texture coordinate
		8.0f, 5.0f, 0.0f,	// vertex 3: position
		0.0f, 0.0f, 1.0f,	// vertex 3: normal
		1.0f, 0.0f, 0.0f,	// vertex 3: tangent
		4.0f, 4.0f,			// vertex 3: texture coordinate
};

//Picture
std::vector<GLfloat> verticesP =
{

	-5.0f, 0.0f, 5.0f,	// vertex 0: position
	0.0f, 1.0f, 0.0f,	// vertex 0: normal
	0.0f, 0.0f,			// vertex 0: texture coordinate
	5.0f, 0.0f, 5.0f,	// vertex 1: position
	0.0f, 1.0f, 0.0f,	// vertex 1: normal
	1.0f, 0.0f,			// vertex 1: texture coordinate
	-5.0f, 0.0f, -5.0f,	// vertex 2: position
	0.0f, 1.0f, 0.0f,	// vertex 2: normal
	0.0f, 1.0f,			// vertex 2: texture coordinate
	5.0f, 0.0f, -5.0f,	// vertex 3: position
	0.0f, 1.0f, 0.0f,	// vertex 3: normal
	1.0f, 1.0f,			// vertex 3: texture coordinate

};

//line
std::vector<GLfloat> line =
{

		// lines
		0.0f, 300.0f, 0.0f,		// line 1 vertex 0: position
		1.0f, 1.0f, 1.0f,		// line 1 vertex 0: colour
		800.0f, 300.0f, 0.0f,	// line 1 vertex 1: position
		1.0f, 1.0f, 1.0f,		// line 1 vertex 1: colour
		400.0f, 0.0f, 0.0f,		// line 2 vertex 0: position
		1.0f, 1.0f, 1.0f,		// line 2 vertex 0: colour
		400.0f, 600.0f, 0.0f,	// line 2 vertex 1: position
		1.0f, 1.0f, 1.0f,		// line 2 vertex 1: colour

};

// function initialise scene and render settings
static void init(GLFWwindow* window)
{
	// set the color the color buffer should be cleared to
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glEnable(GL_DEPTH_TEST);	// enable depth buffer test

	// compile and link a vertex and fragment shader pair
	gShader["ground"].compileAndLink("lightingAndTexture.vert", "pointLightTexture.frag");
	gShader["line"].compileAndLink("modelTransform.vert", "color.frag");
	gShader["wall"].compileAndLink("normalMap.vert", "normalMap.frag");
	gShader["ring"].compileAndLink("lighting.vert", "lighting_cubemap.frag");

	// initialise view matrix camera settings
	gViewMatrix["btmRight"];

	gViewMatrix["topRight"] =	glm::lookAt(glm::vec3(0.5f, 2.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, 0.0f), 
								glm::vec3(0.0f, 1.0f, 0.0f));

	gViewMatrix["main"]	=		glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), 
								glm::vec3(0.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, 0.0f));

	gViewMatrix["btmLeft"] =	glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
								glm::vec3(0.0f, 0.0f, 0.0f),
								glm::vec3(0.0f, 1.0f, 0.0f));

	// initialise projection matrix
	gProjectionMatrix["main"] = glm::ortho(0.0f, static_cast<float>(gWindowWidth), 0.0f, static_cast<float>(gWindowHeight), 0.1f, 10.0f);
	gProjectionMatrix["topRight"] = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
	gProjectionMatrix["btmLeft"] = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
	gProjectionMatrix["btmRight"] = glm::perspective(glm::radians(45.0f), static_cast<float>(gWindowWidth) / gWindowHeight, 0.1f, 10.0f);

	// initialise model matrices
	gModelMatrix["ground"] = glm::mat4(1.0f);
	gModelMatrix["wallbl"] = glm::mat4(1.0f);
	gModelMatrix["wall1"] = glm::mat4(1.0f);
	gModelMatrix["wall2"] = glm::mat4(1.0f);
	gModelMatrix["wall3"] = glm::mat4(1.0f);
	gModelMatrix["wall4"] = glm::mat4(1.0f);
	gModelMatrix["painting"] = glm::mat4(1.0f);
	gModelMatrix["paintingbl"] = glm::mat4(1.0f);
	gModelMatrix["ring"] = glm::mat4(1.0f);
	gModelMatrix["ringbr"] = glm::mat4(1.0f);

	// initialise point light properties
	gLight.pos = glm::vec3(0.0f, 0.1f, 0.0f);
	gLight.La = glm::vec3(0.3f);
	gLight.Ld = glm::vec3(1.0f);
	gLight.Ls = glm::vec3(1.0f);
	gLight.att = glm::vec3(1.0f, 0.0f, 0.0f);

	//ground
	// initialise material properties
	gMaterial["ground"].Ka = glm::vec3(0.32f, 0.22f, 0.02f);
	gMaterial["ground"].Kd = glm::vec3(0.78f, 0.56f, 0.11f);
	gMaterial["ground"].Ks = glm::vec3(0.99f, 0.94f, 0.8f);
	gMaterial["ground"].shininess = 27.89f;

	// load texture
	gTexture["ground"].generate("./images/check.bmp");

	//wall
	// initialise material properties
	gMaterial["wall"].Ka = glm::vec3(0.2f);
	gMaterial["wall"].Kd = glm::vec3(0.2f, 0.7f, 1.0f);
	gMaterial["wall"].Ks = glm::vec3(0.2f, 0.7f, 1.0f);
	gMaterial["wall"].shininess = 40.0f;

	// load texture and normal map
	gTexture["Stone"].generate("./images/Fieldstone.bmp");
	gTexture["StoneNormalMap"].generate("./images/FieldstoneBumpDOT3.bmp");

	//painting
	// initialise material properties
	gMaterial["painting"].Ka = glm::vec3(0.25f);
	gMaterial["painting"].Kd = glm::vec3(0.4f);
	gMaterial["painting"].Ks = glm::vec3(0.774597f);
	gMaterial["painting"].shininess = 76.8f;

	// load texture
	gTexture["painting"].generate("./images/picture.jpg");

	//ring
	// initialise material properties
	gMaterial["ring"].Ka = glm::vec3(3.0f);
	gMaterial["ring"].Kd = glm::vec3(3.0f);
	gMaterial["ring"].Ks = glm::vec3(3.0f);
	gMaterial["ring"].shininess = 100.0f;

	// load cube environment map texture
	gTexture["ring"] .generate(
		"./images/cm_front.bmp", "./images/cm_back.bmp",
		"./images/cm_left.bmp", "./images/cm_right.bmp",
		"./images/cm_top.bmp", "./images/cm_bottom.bmp");

	// load model
	gModel.loadModel("./models/torus.obj");
	
	//ground
	// create VBO
	glGenBuffers(1, &gVBO[0]);					// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticesG.size(), &verticesG[0], GL_STATIC_DRAW);

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO[0]);			// generate unused VAO identifier
	glBindVertexArray(gVAO[0]);				// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[0]);	// bind the VBO

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex),
		reinterpret_cast<void*>(offsetof(VertexNormTex, position)));	// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex),
		reinterpret_cast<void*>(offsetof(VertexNormTex, normal)));		// specify format of colour data
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex),
		reinterpret_cast<void*>(offsetof(VertexNormTex, texCoord)));	// specify format of texture coordinate data

	glEnableVertexAttribArray(0);	// enable vertex attributes
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);


	//line
	// create VBO
	glGenBuffers(1, &gVBO[1]);					// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * line.size(), &line[0], GL_STATIC_DRAW);

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO[1]);			// generate unused VAO identifier
	glBindVertexArray(gVAO[1]);				// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[1]);	// bind the VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, position)));	// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, color)));		// specify format of colour data

	glEnableVertexAttribArray(0);	// enable vertex attributes
	glEnableVertexAttribArray(1);

	//wall
	// create VBO
	glGenBuffers(1, &gVBO[2]);					// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticesW.size(), &verticesW[0], GL_STATIC_DRAW);

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO[2]);			// generate unused VAO identifier
	glBindVertexArray(gVAO[2]);				// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[2]);	// bind the VBO

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, position)));		// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, normal)));		// specify format of colour data
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, tangent)));		// specify format of tangent data
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, texCoord)));		// specify format of texture coordinate data

	glEnableVertexAttribArray(0);	// enable vertex attributes
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	//painting
	// create VBO
	glGenBuffers(1, &gVBO[3]);					// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* verticesP.size(), &verticesP[0], GL_STATIC_DRAW);

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO[3]);			// generate unused VAO identifier
	glBindVertexArray(gVAO[3]);				// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO[3]);	// bind the VBO

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex),
		reinterpret_cast<void*>(offsetof(VertexNormTex, position)));	// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex),
		reinterpret_cast<void*>(offsetof(VertexNormTex, normal)));		// specify format of colour data
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex),
		reinterpret_cast<void*>(offsetof(VertexNormTex, texCoord)));	// specify format of texture coordinate data

	glEnableVertexAttribArray(0);	// enable vertex attributes
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

}

// function used to update the scene
static void update_scene(GLFWwindow* window)
{

	if (ringRotate == true)
	{
		rotateR++;
	}

	gModelMatrix["ground"] = glm::scale(glm::vec3(0.25f,0.25f,0.25f));

	gModelMatrix["wall1"] = glm::translate(glm::vec3(0.0f,0.0f,-0.5f))
		* glm::scale(glm::vec3(0.08f, 0.15f, 0.08f));

	gModelMatrix["wall2"] = glm::translate(glm::vec3(-0.5f, 0.0f, 0.0f))
		* glm::rotate(1.57f, glm::vec3(0.0f,1.0f,0.0f))
		* glm::scale(glm::vec3(0.08f, 0.15f, 0.08f));

	gModelMatrix["wall3"] = glm::translate(glm::vec3(0.5f, 0.0f, 0.0f))
		* glm::rotate(-1.57f, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::vec3(0.08f, 0.15f, 0.08f));

	gModelMatrix["wall4"] = glm::translate(glm::vec3(0.0f, 0.0f, 0.5f))
		* glm::rotate(3.14f, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::vec3(0.08f, 0.15f, 0.08f));

	gModelMatrix["wallbl"] = glm::translate(glm::vec3(0.0f, 0.0f, -0.5f))
		* glm::scale(glm::vec3(0.15f, 0.15f, 0.15f));

	gModelMatrix["painting"] = glm::translate(glm::vec3(0.0f, 0.4f, -0.4f))
		* glm::scale(glm::vec3(0.025f, 0.025f, 0.025f))
		* glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f));

	gModelMatrix["paintingbl"] = glm::translate(glm::vec3(0.0f, 0.0f, -0.4f))
		* glm::scale(glm::vec3(0.075f, 0.075f, 0.075f))
		* glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f));

	gModelMatrix["ringbl"] = glm::rotate(glm::radians(rotateR), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::scale(glm::vec3(0.25f,0.25f,0.25f));

	gModelMatrix["ring"] = glm::rotate(glm::radians(rotateR), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::translate(glm::vec3(0.0f,0.0f,-0.25f))
		* glm::scale(glm::vec3(0.1f, 0.1f, 0.1f));

	gModelMatrix["ringtr"] = glm::rotate(glm::radians(rotateR), glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::rotate(1.57f, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::translate(glm::vec3(0.0f, 0.0f, -0.25f))
		* glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));

	gViewMatrix["btmRight"] = glm::rotate(pitch, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::rotate(yaw, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::translate(glm::vec3(0.0f,-0.3f,-0.37f));
}

// function to render the scene
static void render_scene()
{
	// clear colour buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/**************************************
	 * Main viewport
	 **************************************/
	glViewport(0, 0, 800, 600);

	gShader["line"].use();
	// draw lines in screen space
	// use the main orthographic projection matrix to set model-view-project matrix
	MVP["line"] = gProjectionMatrix["main"] * gViewMatrix["main"];
	// set uniform model transformation matrix
	glBindVertexArray(gVAO[1]);
	gShader["line"].setUniform("uModelViewProjectionMatrix", MVP["line"]);
	glDrawArrays(GL_LINES, 0, 4);	// display the lines


	/**************************************
	 * topRight viewport
	 **************************************/
	glViewport(400, 300, 400, 300);

	// use the shaders associated with the shader program
	gShader["ground"].use();

	// set light properties
	gShader["ground"].setUniform("uLight.pos", gLight.pos);
	gShader["ground"].setUniform("uLight.La", gLight.La);
	gShader["ground"].setUniform("uLight.Ld", gLight.Ld);
	gShader["ground"].setUniform("uLight.Ls", gLight.Ls);
	gShader["ground"].setUniform("uLight.att", gLight.att);


	//ground
	// set material properties
	gShader["ground"].setUniform("uMaterial.Ka", gMaterial["ground"].Ka);
	gShader["ground"].setUniform("uMaterial.Kd", gMaterial["ground"].Kd);
	gShader["ground"].setUniform("uMaterial.Ks", gMaterial["ground"].Ks);
	gShader["ground"].setUniform("uMaterial.shininess", gMaterial["ground"].shininess);

	// set viewing position
	gShader["ground"].setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));

	// calculate matrices
	MVP["ground"] = gProjectionMatrix["topRight"] * gViewMatrix["topRight"] * gModelMatrix["ground"];
	normalMatrix["ground"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["ground"])));

	// set uniform variables
	gShader["ground"].setUniform("uModelViewProjectionMatrix", MVP["ground"]);
	gShader["ground"].setUniform("uModelMatrix", gModelMatrix["ground"]);
	gShader["ground"].setUniform("uNormalMatrix", normalMatrix["ground"]);

	// set texture
	gShader["ground"].setUniform("uTextureSampler", 0);

	glActiveTexture(GL_TEXTURE0);
	gTexture["ground"].bind();

	glBindVertexArray(gVAO[0]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertice

	//ring
	// use the shaders associated with the shader program
	gShader["ring"].use();

	// set light properties
	gShader["ring"].setUniform("uLight.La", gLight.La);
	gShader["ring"].setUniform("uLight.Ld", gLight.Ld);
	gShader["ring"].setUniform("uLight.Ls", gLight.Ls);

	// set material properties
	gShader["ring"].setUniform("uMaterial.Ka", gMaterial["ring"].Ka);
	gShader["ring"].setUniform("uMaterial.Kd", gMaterial["ring"].Kd);
	gShader["ring"].setUniform("uMaterial.Ks", gMaterial["ring"].Ks);
	gShader["ring"].setUniform("uMaterial.shininess", gMaterial["ring"].shininess);

	// set viewing position
	gShader["ring"].setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));

	// calculate matrices
	MVP["ringtr"] = gProjectionMatrix["topRight"] * gViewMatrix["topRight"] * gModelMatrix["ringtr"];
	normalMatrix["ringtr"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["ringtr"])));

	// set uniform variables
	gShader["ring"].setUniform("uModelViewProjectionMatrix", MVP["ringtr"]);
	gShader["ring"].setUniform("uModelMatrix", gModelMatrix["ringtr"]);
	gShader["ring"].setUniform("uNormalMatrix", normalMatrix["ringtr"]);

	// set cube environment map
	gShader["ring"].setUniform("uEnvironmentMap", 0);

	glActiveTexture(GL_TEXTURE0);
	gTexture["ring"].bind();

	// render model
	gModel.drawModel();



	/**************************************
	 * btmRight viewport
	 **************************************/
	glViewport(400, 0, 400, 300);

	// use the shaders associated with the shader program
	gShader["ground"].use();

	// set light properties
	gShader["ground"].setUniform("uLight.pos", gLight.pos);
	gShader["ground"].setUniform("uLight.La", gLight.La);
	gShader["ground"].setUniform("uLight.Ld", gLight.Ld);
	gShader["ground"].setUniform("uLight.Ls", gLight.Ls);
	gShader["ground"].setUniform("uLight.att", gLight.att);


	//ground
	// set material properties
	gShader["ground"].setUniform("uMaterial.Ka", gMaterial["ground"].Ka);
	gShader["ground"].setUniform("uMaterial.Kd", gMaterial["ground"].Kd);
	gShader["ground"].setUniform("uMaterial.Ks", gMaterial["ground"].Ks);
	gShader["ground"].setUniform("uMaterial.shininess", gMaterial["ground"].shininess);

	// set viewing position
	gShader["ground"].setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));

	// calculate matrices
	MVP["ground"] = gProjectionMatrix["btmRight"] * gViewMatrix["btmRight"] * gModelMatrix["ground"];
	normalMatrix["ground"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["ground"])));

	// set uniform variables
	gShader["ground"].setUniform("uModelViewProjectionMatrix", MVP["ground"]);
	gShader["ground"].setUniform("uModelMatrix", gModelMatrix["ground"]);
	gShader["ground"].setUniform("uNormalMatrix", normalMatrix["ground"]);

	// set texture
	gShader["ground"].setUniform("uTextureSampler", 0);

	glActiveTexture(GL_TEXTURE0);
	gTexture["ground"].bind();

	glBindVertexArray(gVAO[0]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertice

	//painting mat with ground shader
	// set material properties
	gShader["ground"].setUniform("uMaterial.Ka", gMaterial["painting"].Ka);
	gShader["ground"].setUniform("uMaterial.Kd", gMaterial["painting"].Kd);
	gShader["ground"].setUniform("uMaterial.Ks", gMaterial["painting"].Ks);
	gShader["ground"].setUniform("uMaterial.shininess", gMaterial["painting"].shininess);

	// set viewing position
	gShader["ground"].setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));

	// calculate matrices
	MVP["painting"] = gProjectionMatrix["btmRight"] * gViewMatrix["btmRight"] * gModelMatrix["painting"];
	normalMatrix["painting"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["painting"])));

	// set uniform variables
	gShader["ground"].setUniform("uModelViewProjectionMatrix", MVP["painting"]);
	gShader["ground"].setUniform("uModelMatrix", gModelMatrix["painting"]);
	gShader["ground"].setUniform("uNormalMatrix", normalMatrix["painting"]);

	// set texture
	gShader["ground"].setUniform("uTextureSampler", 0);

	glActiveTexture(GL_TEXTURE0);
	gTexture["painting"].bind();

	glBindVertexArray(gVAO[3]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertice


	//ring
	// use the shaders associated with the shader program
	gShader["ring"].use();

	// set light properties
	gShader["ring"].setUniform("uLight.La", gLight.La);
	gShader["ring"].setUniform("uLight.Ld", gLight.Ld);
	gShader["ring"].setUniform("uLight.Ls", gLight.Ls);

	// set material properties
	gShader["ring"].setUniform("uMaterial.Ka", gMaterial["ring"].Ka);
	gShader["ring"].setUniform("uMaterial.Kd", gMaterial["ring"].Kd);
	gShader["ring"].setUniform("uMaterial.Ks", gMaterial["ring"].Ks);
	gShader["ring"].setUniform("uMaterial.shininess", gMaterial["ring"].shininess);

	// set viewing position
	gShader["ring"].setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));

	// calculate matrices
	MVP["ring"] = gProjectionMatrix["btmRight"] * gViewMatrix["btmRight"] * gModelMatrix["ring"];
	normalMatrix["ring"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["ring"])));

	// set uniform variables
	gShader["ring"].setUniform("uModelViewProjectionMatrix", MVP["ring"]);
	gShader["ring"].setUniform("uModelMatrix", gModelMatrix["ring"]);
	gShader["ring"].setUniform("uNormalMatrix", normalMatrix["ring"]);

	// set cube environment map
	gShader["ring"].setUniform("uEnvironmentMap", 0);

	glActiveTexture(GL_TEXTURE0);
	gTexture["ring"].bind();

	// render model
	gModel.drawModel();


	// wall
	gShader["wall"].use();

	//light
	gShader["wall"].setUniform("uLight.pos", gLight.pos);
	gShader["wall"].setUniform("uLight.La", gLight.La);
	gShader["wall"].setUniform("uLight.Ld", gLight.Ld);
	gShader["wall"].setUniform("uLight.Ls", gLight.Ls);
	gShader["wall"].setUniform("uLight.att", gLight.att);

	// set material properties
	gShader["wall"].setUniform("uMaterial.Ka", gMaterial["wall"].Ka);
	gShader["wall"].setUniform("uMaterial.Kd", gMaterial["wall"].Kd);
	gShader["wall"].setUniform("uMaterial.Ks", gMaterial["wall"].Ks);
	gShader["wall"].setUniform("uMaterial.shininess", gMaterial["wall"].shininess);

	gShader["wall"].setUniform("uViewpoint", glm::vec3(0.0f, 0.0f, 4.0f));

	MVP["wall"] = gProjectionMatrix["btmRight"] * gViewMatrix["btmRight"] * gModelMatrix["wall1"];
	normalMatrix["wall"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["wall1"])));
	// set uniform variables
	gShader["wall"].setUniform("uModelViewProjectionMatrix", MVP["wall"]);
	gShader["wall"].setUniform("uModelMatrix", gModelMatrix["wall1"]);
	gShader["wall"].setUniform("uNormalMatrix", normalMatrix["wall"]);

	// set texture and normal map
	gShader["wall"].setUniform("uTextureSampler", 0);
	gShader["wall"].setUniform("uNormalSampler", 1);

	glActiveTexture(GL_TEXTURE0);
	gTexture["Stone"].bind();

	glActiveTexture(GL_TEXTURE1);
	gTexture["StoneNormalMap"].bind();

	glBindVertexArray(gVAO[2]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertices
	
	MVP["wall"] = gProjectionMatrix["btmRight"] * gViewMatrix["btmRight"] * gModelMatrix["wall2"];
	normalMatrix["wall"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["wall2"])));
	// set uniform variables
	gShader["wall"].setUniform("uModelViewProjectionMatrix", MVP["wall"]);
	gShader["wall"].setUniform("uModelMatrix", gModelMatrix["wall2"]);
	gShader["wall"].setUniform("uNormalMatrix", normalMatrix["wall"]);

	// set texture and normal map
	gShader["wall"].setUniform("uTextureSampler", 0);
	gShader["wall"].setUniform("uNormalSampler", 1);

	glActiveTexture(GL_TEXTURE0);
	gTexture["Stone"].bind();

	glActiveTexture(GL_TEXTURE1);
	gTexture["StoneNormalMap"].bind();

	glBindVertexArray(gVAO[2]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertices

	MVP["wall"] = gProjectionMatrix["btmRight"] * gViewMatrix["btmRight"] * gModelMatrix["wall3"];
	normalMatrix["wall"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["wall3"])));
	// set uniform variables
	gShader["wall"].setUniform("uModelViewProjectionMatrix", MVP["wall"]);
	gShader["wall"].setUniform("uModelMatrix", gModelMatrix["wall3"]);
	gShader["wall"].setUniform("uNormalMatrix", normalMatrix["wall"]);

	// set texture and normal map
	gShader["wall"].setUniform("uTextureSampler", 0);
	gShader["wall"].setUniform("uNormalSampler", 1);

	glActiveTexture(GL_TEXTURE0);
	gTexture["Stone"].bind();

	glActiveTexture(GL_TEXTURE1);
	gTexture["StoneNormalMap"].bind();

	glBindVertexArray(gVAO[2]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertices

	MVP["wall"] = gProjectionMatrix["btmRight"] * gViewMatrix["btmRight"] * gModelMatrix["wall4"];
	normalMatrix["wall"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["wall4"])));
	// set uniform variables
	gShader["wall"].setUniform("uModelViewProjectionMatrix", MVP["wall"]);
	gShader["wall"].setUniform("uModelMatrix", gModelMatrix["wall4"]);
	gShader["wall"].setUniform("uNormalMatrix", normalMatrix["wall"]);

	// set texture and normal map
	gShader["wall"].setUniform("uTextureSampler", 0);
	gShader["wall"].setUniform("uNormalSampler", 1);

	glActiveTexture(GL_TEXTURE0);
	gTexture["Stone"].bind();

	glActiveTexture(GL_TEXTURE1);
	gTexture["StoneNormalMap"].bind();

	glBindVertexArray(gVAO[2]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertices

	/**************************************
	 * btmLeft viewport
	 **************************************/
	glViewport(0, 0, 400, 300);

	// set viewing position
	gShader["wall"].setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));
	//wall
	// set material properties
	gShader["wall"].setUniform("uMaterial.Ka", gMaterial["wall"].Ka);
	gShader["wall"].setUniform("uMaterial.Kd", gMaterial["wall"].Kd);
	gShader["wall"].setUniform("uMaterial.Ks", gMaterial["wall"].Ks);
	gShader["wall"].setUniform("uMaterial.shininess", gMaterial["wall"].shininess);


	MVP["wall"] = gProjectionMatrix["btmLeft"] * gViewMatrix["btmLeft"] * gModelMatrix["wallbl"];
	normalMatrix["wall"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["wallbl"])));
	// set uniform variables
	gShader["wall"].setUniform("uModelViewProjectionMatrix", MVP["wall"]);
	gShader["wall"].setUniform("uModelMatrix", gModelMatrix["wallbl"]);
	gShader["wall"].setUniform("uNormalMatrix", normalMatrix["wall"]);

	// set texture and normal map
	gShader["wall"].setUniform("uTextureSampler", 0);
	gShader["wall"].setUniform("uNormalSampler", 1);

	glActiveTexture(GL_TEXTURE0);
	gTexture["Stone"].bind();

	glActiveTexture(GL_TEXTURE1);
	gTexture["StoneNormalMap"].bind();

	glBindVertexArray(gVAO[2]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertices

	// use the shaders associated with the shader program
	gShader["ground"].use();

	// set light properties
	gShader["ground"].setUniform("uLight.pos", gLight.pos);
	gShader["ground"].setUniform("uLight.La", gLight.La);
	gShader["ground"].setUniform("uLight.Ld", gLight.Ld);
	gShader["ground"].setUniform("uLight.Ls", gLight.Ls);
	gShader["ground"].setUniform("uLight.att", gLight.att);


	//painting mat with ground shader
	// set material properties
	gShader["ground"].setUniform("uMaterial.Ka", gMaterial["painting"].Ka);
	gShader["ground"].setUniform("uMaterial.Kd", gMaterial["painting"].Kd);
	gShader["ground"].setUniform("uMaterial.Ks", gMaterial["painting"].Ks);
	gShader["ground"].setUniform("uMaterial.shininess", gMaterial["painting"].shininess);

	// set viewing position
	gShader["ground"].setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));

	// calculate matrices
	MVP["paintingbl"] = gProjectionMatrix["btmLeft"] * gViewMatrix["btmLeft"] * gModelMatrix["paintingbl"];
	normalMatrix["paintingbl"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["paintingbl"])));

	// set uniform variables
	gShader["ground"].setUniform("uModelViewProjectionMatrix", MVP["paintingbl"]);
	gShader["ground"].setUniform("uModelMatrix", gModelMatrix["paintingbl"]);
	gShader["ground"].setUniform("uNormalMatrix", normalMatrix["paintingbl"]);

	// set texture
	gShader["ground"].setUniform("uTextureSampler", 0);

	glActiveTexture(GL_TEXTURE0);
	gTexture["painting"].bind();

	glBindVertexArray(gVAO[3]);				// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertice

	//ring
	// use the shaders associated with the shader program
	gShader["ring"].use();

	// set light properties
	gShader["ring"].setUniform("uLight.La", gLight.La);
	gShader["ring"].setUniform("uLight.Ld", gLight.Ld);
	gShader["ring"].setUniform("uLight.Ls", gLight.Ls);

	// set material properties
	gShader["ring"].setUniform("uMaterial.Ka", gMaterial["ring"].Ka);
	gShader["ring"].setUniform("uMaterial.Kd", gMaterial["ring"].Kd);
	gShader["ring"].setUniform("uMaterial.Ks", gMaterial["ring"].Ks);
	gShader["ring"].setUniform("uMaterial.shininess", gMaterial["ring"].shininess);

	// set viewing position
	gShader["ring"].setUniform("uViewpoint", glm::vec3(0.0f, 2.0f, 4.0f));

	// calculate matrices
	MVP["ringbl"] = gProjectionMatrix["btmLeft"] * gViewMatrix["btmLeft"] * gModelMatrix["ringbl"];
	normalMatrix["ringbl"] = glm::mat3(glm::transpose(glm::inverse(gModelMatrix["ringbl"])));

	// set uniform variables
	gShader["ring"].setUniform("uModelViewProjectionMatrix", MVP["ringbl"]);
	gShader["ring"].setUniform("uModelMatrix", gModelMatrix["ringbl"]);
	gShader["ring"].setUniform("uNormalMatrix", normalMatrix["ringbl"]);

	// set cube environment map
	gShader["ring"].setUniform("uEnvironmentMap", 0);

	glActiveTexture(GL_TEXTURE0);
	gTexture["ring"].bind();

	// render model
	gModel.drawModel();


	// flush the graphics pipeline
	glFlush();
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// close the window when the ESCAPE key is pressed
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
}

// mouse movement callback function
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// pass cursor position to tweak bar
	TwEventMousePosGLFW(static_cast<int>(xpos), static_cast<int>(ypos));
}

// mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// pass mouse button status to tweak bar
	TwEventMouseButtonGLFW(button, action);
}

// error callback function
static void error_callback(int error, const char* description)
{
	std::cerr << description << std::endl;	// output error description
}

// create and populate tweak bar elements
TwBar* create_UI(const std::string name)
{
	// create a tweak bar
	TwBar* twBar = TwNewBar(name.c_str());

	// give tweak bar the size of graphics window
	TwWindowSize(gWindowWidth, gWindowHeight);
	TwDefine(" TW_HELP visible=false ");	// disable help menu
	TwDefine(" GLOBAL fontsize=3 ");		// set large font size

	TwDefine(" Main label='User Interface' refresh=0.02 text=light size='250 250' position='10 10' ");

	// create frame stat entries
	TwAddVarRO(twBar, "Frame Rate", TW_TYPE_FLOAT, &gFrameRate, " group='Frame Stats' precision=2 ");
	TwAddVarRO(twBar, "Frame Time", TW_TYPE_FLOAT, &gFrameTime, " group='Frame Stats' ");

	// scene controls
	TwAddVarRW(twBar, "Toggle", TW_TYPE_BOOLCPP, &ringRotate, " group='Animation' ");

	// light controls
	TwAddVarRW(twBar, "Position X", TW_TYPE_FLOAT, &gLight.pos.x, " group='Light' min=-3.0 max=3.0 step=0.01 ");
	TwAddVarRW(twBar, "Position Y", TW_TYPE_FLOAT, &gLight.pos.y, " group='Light' min=-3.0 max=3.0 step=0.01 ");
	TwAddVarRW(twBar, "Position Z", TW_TYPE_FLOAT, &gLight.pos.z, " group='Light' min=-3.0 max=3.0 step=0.01 ");

	//camera controls
	TwAddVarRW(twBar, "Yaw", TW_TYPE_FLOAT, &yaw, " group='Camera' step=0.01 "); // left right
	TwAddVarRW(twBar, "Pitch", TW_TYPE_FLOAT, &pitch, " group='Camera' step=0.01 "); // up down

	return twBar;
}

int main(void)
{
	GLFWwindow* window = nullptr;	// GLFW window handle

	glfwSetErrorCallback(error_callback);	// set GLFW error callback function

	// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window and its OpenGL context
	window = glfwCreateWindow(gWindowWidth, gWindowHeight, "A2", nullptr, nullptr);

	// check if window created successfully
	if (window == nullptr)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval

	// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		std::cerr << "GLEW initialisation failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	// set GLFW callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// initialise scene and render settings
	init(window);

	// initialise AntTweakBar
	TwInit(TW_OPENGL_CORE, nullptr);
	TwBar* tweakBar = create_UI("Main");		// create and populate tweak bar elements

	// timing data
	double lastUpdateTime = glfwGetTime();	// last update time
	double elapsedTime = lastUpdateTime;	// time since last update
	int frameCount = 0;						// number of frames since last update

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		update_scene(window);	// update the scene

		// if wireframe set polygon render mode to wireframe
		if (gWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		render_scene();			// render the scene

		// set polygon render mode to fill
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		TwDraw();				// draw tweak bar

		glfwSwapBuffers(window);	// swap buffers
		glfwPollEvents();			// poll for events

		frameCount++;
		elapsedTime = glfwGetTime() - lastUpdateTime;	// time since last update

		// if elapsed time since last update > 1 second
		if (elapsedTime > 1.0)
		{
			gFrameTime = elapsedTime / frameCount;	// average time per frame
			gFrameRate = 1 / gFrameTime;			// frames per second
			lastUpdateTime = glfwGetTime();			// set last update time to current time
			frameCount = 0;							// reset frame counter
		}
	}

	// clean up
	glDeleteBuffers(1, &gVBO[0]);
	glDeleteVertexArrays(1, &gVAO[0]);
	glDeleteBuffers(1, &gVBO[1]);
	glDeleteVertexArrays(1, &gVAO[1]);
	glDeleteBuffers(1, &gVBO[2]);
	glDeleteVertexArrays(1, &gVAO[2]);
	glDeleteBuffers(1, &gVBO[3]);
	glDeleteVertexArrays(1, &gVAO[3]);

	// uninitialise tweak bar
	TwDeleteBar(tweakBar);
	TwTerminate();

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}