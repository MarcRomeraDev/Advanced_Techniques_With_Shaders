#pragma once
#include "Shader.h"
#include "Model.h"
//Classe "Objecte": Agrupa els atributs necessaris dels objectes que carguem i instanciem a l'escena.
//Tamb� cont� els seus shaders
class Object
{
private:
	GLuint ObjVao;
	unsigned int textureID;
	GLuint ObjVbo[3];
	

	//int texWidth, texHeight, nrChannels;
	//unsigned char* data;
	std::string name;
	glm::vec3 initPos, initRot, initScale;

	int numVertices;


public:
	Object(Model, unsigned int, glm::vec3, glm::vec3, glm::vec3, Shader);

	glm::vec3 position, rotation, scale;
	Shader shader;
	glm::mat4 objMat;

	void Update();
	void Draw();
	void Draw(Light);
	void Draw(float,float ,float,bool,bool);
	void CleanUp();

	std::string GetName() { return name; } //--> Retorna el nom de l'objecte corresponent per poder "printar-lo" per pantalla
};