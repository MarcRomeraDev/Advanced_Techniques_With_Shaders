#define STB_IMAGE_IMPLEMENTATION

#include "GL_framework.h"

#include "Object.h"
#include "Billboard.h"
#include "Constants.h"
#include "Texture.h"
#include "FrameBuffer.h"

// Declaraci� de la funci� del loadCubemap
extern unsigned int loadCubemap(std::vector<std::string> faces);

#pragma region Variables globals
Light light;
Scene scene;
std::vector<Object> objects; //--> Vector que emmagatzema els objectes que s'instancien a l'escena.
std::vector<Billboard> billboards; //--> Vector que emmagatzema les billboards que s'instancien a l'escena.
std::string s; //--> String declarat global per no redeclarar-lo a cada frame. S'usa pels noms del ImGui.
FrameBuffer frameBuffer;

int gWidth, gHeight;

glm::vec3 cameraOffset = glm::vec3(1.71, -4.55, 0.53);

bool isFirstPerson = false;
bool usingInstancing = false;
Shader objShader;
Shader bbShader;
int amount = 10;
glm::mat4* modelMatrices;

#pragma endregion

namespace RenderVars {
	float FOV = glm::radians(90.f);
	float zNear = 0.01f;
	float zFar = 5000.f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _MVP;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	struct prevMouse {
		float lastx, lasty;
		MouseEvent::Button button = MouseEvent::Button::None;
		bool waspressed = false;
	} prevMouse;

	float panv[3] = { 0.f, -5.f, -15.f };
	float rota[2] = { 0.f, 0.f };
}
namespace RV = RenderVars;

namespace GeometryShadersInfo
{
	// Explosion animation
	bool explosionAnim = false;
	float currentTime = 0;
	float auxTime = 0;
	float magnitude = 5;
	bool subDivide = false;

	// Billboards
	float width = 5.0f;
	float height = 10.0f;
}
namespace GSI = GeometryShadersInfo;

///////// fw decl
namespace ImGui {
	void Render();
}
namespace Axis {
	void setupAxis();
	void cleanupAxis();
	void draw();
}
//////////////

void GLResize(int width, int height) {
	glViewport(0, 0, width, height);
	if (height != 0) RV::_projection = glm::perspective(glm::radians(RV::FOV), (float)width / (float)height, RV::zNear, RV::zFar);
	else RV::_projection = glm::perspective(glm::radians(RV::FOV), 0.f, RV::zNear, RV::zFar);
	gWidth = width;
	gHeight = height;

}

void GLmousecb(MouseEvent ev) {
	if (RV::prevMouse.waspressed && RV::prevMouse.button == ev.button) {
		float diffx = ev.posx - RV::prevMouse.lastx;
		float diffy = ev.posy - RV::prevMouse.lasty;
		switch (ev.button) {
		case MouseEvent::Button::Left: // ROTATE
			RV::rota[0] += diffx * 0.005f;
			RV::rota[1] += diffy * 0.005f;
			break;
		case MouseEvent::Button::Right: // MOVE XY
			RV::panv[0] += diffx * 0.03f;
			RV::panv[1] -= diffy * 0.03f;
			break;
		case MouseEvent::Button::Middle: // MOVE Z
			RV::panv[2] += diffy * 0.05f;
			break;
		default: break;
		}
	}
	else {
		RV::prevMouse.button = ev.button;
		RV::prevMouse.waspressed = true;
	}
	RV::prevMouse.lastx = ev.posx;
	RV::prevMouse.lasty = ev.posy;
}

//////////////////////////////////////////////// AXIS
namespace Axis {
	Shader axisShader;
	GLuint AxisVao;
	GLuint AxisVbo[3];

	float AxisVerts[] = {
		0.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0
	};
	float AxisColors[] = {
		1.0, 0.0, 0.0, 1.0,
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0,
		0.0, 0.0, 1.0, 1.0
	};
	GLubyte AxisIdx[] = {
		0, 1,
		2, 3,
		4, 5
	};

	void setupAxis() {
		glGenVertexArrays(1, &AxisVao);
		glBindVertexArray(AxisVao);
		glGenBuffers(3, AxisVbo);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisColors, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AxisVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 6, AxisIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		axisShader = Shader(axisVS, axisFS);

	}
	void cleanupAxis() {
		glDeleteBuffers(3, AxisVbo);
		glDeleteVertexArrays(1, &AxisVao);

		axisShader.CleanUpShader();
	}
	void draw() {
		glBindVertexArray(AxisVao);
		axisShader.Use();
		axisShader.SetMat4("mvpMat", 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		glDrawElements(GL_LINES, 6, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
	}
}

namespace CubeMap {
	unsigned int skyboxVAO, skyboxVBO;
	Shader cubeMapShader;

	glm::vec3 skyboxVertices[] = {
		// positions          
		{-10.0f,  10.0f, -10.0f},
		{-10.0f, -10.0f, -10.0f},
		{ 10.0f, -10.0f, -10.0f},
		{ 10.0f, -10.0f, -10.0f},
		{ 10.0f,  10.0f, -10.0f},
		{-10.0f,  10.0f, -10.0f},

		{-10.0f, -10.0f,  10.0f},
		{-10.0f, -10.0f, -10.0f},
		{-10.0f,  10.0f, -10.0f},
		{-10.0f,  10.0f, -10.0f},
		{-10.0f,  10.0f,  10.0f},
		{-10.0f, -10.0f,  10.0f},

		{ 10.0f, -10.0f, -10.0f},
		{ 10.0f, -10.0f,  10.0f},
		{ 10.0f,  10.0f,  10.0f},
		{ 10.0f,  10.0f,  10.0f},
		{ 10.0f,  10.0f, -10.0f},
		{ 10.0f, -10.0f, -10.0f},

		{-10.0f, -10.0f,  10.0f},
		{-10.0f,  10.0f,  10.0f},
		{ 10.0f,  10.0f,  10.0f},
		{ 10.0f,  10.0f,  10.0f},
		{ 10.0f, -10.0f,  10.0f},
		{-10.0f, -10.0f,  10.0f},

		{-10.0f,  10.0f, -10.0f},
		{ 10.0f,  10.0f, -10.0f},
		{ 10.0f,  10.0f,  10.0f},
		{ 10.0f,  10.0f,  10.0f},
		{-10.0f,  10.0f,  10.0f},
		{-10.0f,  10.0f, -10.0f},

		{-10.0f, -10.0f, -10.0f},
		{-10.0f, -10.0f,  10.0f},
		{ 10.0f, -10.0f, -10.0f},
		{ 10.0f, -10.0f, -10.0f},
		{-10.0f, -10.0f,  10.0f},
		{ 10.0f, -10.0f,  10.0f}
	};

	std::vector<std::string> faces
	{
		"materials/right.jpg",
		"materials/left.jpg",
		"materials/top.jpg",
		"materials/bottom.jpg",
		"materials/front.jpg",
		"materials/back.jpg"
	};
	unsigned int cubemapTexture;

	void SetUp()
	{
		cubemapTexture = loadCubemap(faces);
		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		cubeMapShader = Shader(skyBoxVS, skyBoxFS);
	}

	void cleanup() {
		glDeleteBuffers(1, &skyboxVBO);
		glDeleteVertexArrays(1, &skyboxVAO);
		cubeMapShader.CleanUpShader();
		glDeleteTextures(1, &cubemapTexture);
	}

	void draw() {
		cubeMapShader.Use();

		glm::mat4 cameraPoint = glm::translate(RV::_modelView, glm::vec3(glm::inverse(RV::_modelView)[3]));
		cubeMapShader.SetMat4("view", 1, GL_FALSE, glm::value_ptr(cameraPoint));
		cubeMapShader.SetMat4("projection", 1, GL_FALSE, glm::value_ptr(RV::_projection));

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);
		glUseProgram(0);
	}
}

void GLinit(int width, int height) {

	gWidth = width;
	gHeight = height;
	srand(static_cast<unsigned>(time(nullptr))); //--> Seed del random
	stbi_set_flip_vertically_on_load(true); //--> Invertim verticalment la textura

	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);

	// Setup shaders & geometry
	light.type = Light::EType::DIRECTIONAL; //--> Inicialitzem el primer tipus d'iluminac� a direccional
	Axis::setupAxis();
	CubeMap::SetUp();

	// Shaders
	objShader = Shader(modelVS, carFS);	 //--> Shader de tots els models
	bbShader = Shader(bbVS, bbFS, bbGS); //--> Shader de totes les billboards

	// FrameBuffer
	frameBuffer = FrameBuffer(frameBufferVS, frameBufferFS);

	// Carreguem i generem les textures
	Texture camaroTexture(Texture::ETYPE::OBJ, carTexture); //--> Textura Cotxes

	Texture treeText01(Texture::ETYPE::BB, treeTexture1); //--> Textura Billboard
	Texture treeText02(Texture::ETYPE::BB, treeTexture2); //--> Textura Billboard
	Texture treeText03(Texture::ETYPE::BB, treeTexture3); //--> Textura Billboard

	Texture treeTextures[3] = { treeText01, treeText02, treeText03 };

#pragma region Models

	// Carguem i generem els models
	Model carModel(carObj);

	// Crida al constructor de la classe amb els diferents objectes
	Object camaro(&carModel, camaroTexture.GetID(), glm::vec3(-3.11f, 1.6f, 2.71f), glm::vec3(0, 4.71f, 0), glm::vec3(0.05f, 0.05f, 0.05f), objShader);

	// Emmagatzema els objectes creats al vector
	objects.push_back(camaro);

#pragma endregion Carreguem, generem i emmagatzemem els models

	// Creem i emmagatzemem billboards
	Billboard tree1(bbShader, glm::vec3(0, 0, 0), treeTextures[0].GetID(), 5, 5);
	billboards.push_back(tree1);

	scene = Scene::PHONG; //--> Inicialitzem la primera escena
}

void GLcleanup() {
	Axis::cleanupAxis();
	CubeMap::cleanup();

	// Cleanup per cada objecte dins del vector
	for (Object obj : objects) obj.CleanUp();
	objects.clear(); //--> Allibera mem�ria del vector d'objectes

	for (Billboard bb : billboards) bb.CleanUp();
	billboards.clear(); //--> Allibera mem�ria del vector de billboards

	frameBuffer.CleanUp();
}

// Funci� que dibuixa (i actualitza) en ordre tots els elements de l'escena
void RenderDraw()
{
	CubeMap::draw();
	Axis::draw();
	for (Object obj : objects) { obj.Update(); obj.Draw(light); }
}

void drawStencilBuffer()
{
	glEnable(GL_STENCIL_TEST); //--> Activem stencil

	glStencilFunc(GL_ALWAYS, 1, 0xFF); //--> Tots els fragments passen l'stencil test
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glEnable(GL_BLEND); //--> Activem el blend per aplicar transpar�ncia a les finestres
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glStencilMask(0xFF); //--> Permet escriure al stencil buffer
	objects[0].usingStencil = true;
	objects[0].Draw(light); //--> Dibuixem el primer cotxe descartant tots els fragments excepte els de les finestres
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF); //--> Nom�s es dibuixa el que no sigui = a 1 (finestres)
	glDisable(GL_BLEND); //--> Desactivem blending per a que no s'apliqui transpar�ncia a la resta del cotxe
	glStencilMask(0x00); //--> Desactivem la opci� d'escriptura al stencil buffer
	objects[0].usingStencil = false;
	objects[0].Draw(light); //--> Dibuixem 2n cotxe descartant els fragments de les finestres
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);

	glDisable(GL_STENCIL_TEST); //--> Activem stencil
}

void GLrender(float dt) {
	glEnable(GL_DEPTH_TEST);

	RV::_modelView = glm::mat4(1);

	// C�mara lliure (transformacions normals de la c�mara + render d'escena normal
	if (!isFirstPerson)
	{

		RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
		RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
		RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));
		RV::_MVP = RV::_projection * RV::_modelView;
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderDraw();
	}
	else //--> C�mara 1a persona (dins d'un cotxe)
	{
#pragma region FrameBuffer

		// 1er render (Escena que servira de textura al framebuffer object)
		RV::FOV = MIRROR_FOV;
		glViewport(0, 0, 800, 600);
		RV::_projection = glm::perspective(glm::radians(RV::FOV), 1.33f, RV::zNear, RV::zFar); // 1.33f = 800 / 600

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.fbo);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		RV::_modelView = glm::translate(RV::_modelView, frameBuffer.GetRearCameraPosition());
		RV::_modelView = glm::rotate(RV::_modelView, -objects[0].rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		RV::_modelView = glm::translate(RV::_modelView, -objects[0].position);
		RV::_MVP = RV::_projection * RV::_modelView;
		CubeMap::draw();
		Axis::draw();

		objects[0].Update();
		drawStencilBuffer();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion 1er render

		//-------------------------------------------------------------------------------//

#pragma region Escena

		// 2n render (Escena dibuixada normal)
		RV::FOV = 90.f;
		GLResize(gWidth, gHeight);

		RV::_modelView = glm::mat4(1);
		RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
		RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));
		RV::_modelView = glm::translate(RV::_modelView, cameraOffset);
		RV::_modelView = glm::rotate(RV::_modelView, -objects[0].rotation.y - PI, glm::vec3(0.0f, 1.0f, 0.0f));
		RV::_modelView = glm::translate(RV::_modelView, -objects[0].position);
		RV::_MVP = RV::_projection * RV::_modelView;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		CubeMap::draw();
		Axis::draw();

		objects[0].Update();
		frameBuffer.Update(objects[0].position, objects[0].rotation); //--> Actualitza matriu d'objecte del frame buffer object per a que es mogui i roti amb el cotxe
		frameBuffer.DrawQuadFBOTex(); //--> Dibuixa la textura de l'escena al quad (retrovisor central del cotxe)

		drawStencilBuffer(); //--> Dibuixa les finestres d'un 2n segon cotxe dibuixat amb stencil i aplicant blend per a que es vegi la semitranspar�ncia

#pragma endregion 2n render
	}

	ImGui::Render();
}

void GUI()
{
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::Checkbox("First Person View", &isFirstPerson);

	if (isFirstPerson) ImGui::DragFloat3("Camera Offset", (float*)&cameraOffset, 0.01f, -50.f, 50.f);
	ImGui::DragFloat3("Mirror Position", (float*)&frameBuffer.localPosition, 0.01f, -50.f, 50.f);

	ImGui::DragFloat3("Car Position", (float*)&objects[0].position, 0.01f, -50.f, 50.f);

	ImGui::DragFloat("Car Rotation", (float*)&objects[0].rotation.y, 0.01f, -100, 100);

	ImGui::End();
}