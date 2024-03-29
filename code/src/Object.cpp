#include "Object.h"

Object::Object() {}

Object::Object(Model* _model, unsigned int texId, glm::vec3 _startPos, glm::vec3 _startRot, glm::vec3 _startScale,
	Shader _shader) : textureID(texId), position(_startPos), rotation(_startRot), scale(_startScale),
	initPos(_startPos), initRot(_startRot), initScale(_startScale), shader(_shader)
{
	numVertices = _model->GetVertices().size();
	name = _model->GetName();

	glGenVertexArrays(1, &ObjVao);
	glBindVertexArray(ObjVao);

	glGenBuffers(3, ObjVbo);

	glBindBuffer(GL_ARRAY_BUFFER, ObjVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), &_model->GetVertices()[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, ObjVbo[1]);
	glBufferData(GL_ARRAY_BUFFER, _model->GetUvs().size() * sizeof(glm::vec2), &_model->GetUvs()[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, ObjVbo[2]);
	glBufferData(GL_ARRAY_BUFFER, _model->GetNormals().size() * sizeof(glm::vec3), &_model->GetNormals()[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	shader.Use();

	glBindAttribLocation(shader.GetID(), 0, "aPos");
	glBindAttribLocation(shader.GetID(), 1, "aUvs");
	glBindAttribLocation(shader.GetID(), 2, "aNormal");

	shader.SetInt("diffuseTexture", 0);
}

void Object::Update()
{
	glm::mat4 t = glm::translate(glm::mat4(), position);
	glm::mat4 r1 = glm::rotate(glm::mat4(), rotation.x, glm::vec3(1, 0, 0));
	glm::mat4 r2 = glm::rotate(glm::mat4(), rotation.y, glm::vec3(0, 1, 0));
	glm::mat4 r3 = glm::rotate(glm::mat4(), rotation.z, glm::vec3(0, 0, 1));
	glm::mat4 s = glm::scale(glm::mat4(), scale);
	objMat = t * r1 * r2 * r3 * s;
}

void Object::Draw(Light light)
{
	shader.Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glBindVertexArray(ObjVao);

	// Variables que pasem als shaders com a uniforms per ser usats per la gr�fica
	if (usingStencil) shader.SetBool("Stencil", true);
	else shader.SetBool("Stencil", false);
	shader.SetMat4("model", 1, GL_FALSE, glm::value_ptr(objMat));
	shader.SetMat4("view", 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
	shader.SetMat4("projection", 1, GL_FALSE, glm::value_ptr(RenderVars::_projection));
	shader.SetFloat3("lightColor", light.color);
	shader.SetFloat3("lightPos", light.position);
	shader.SetFloat3("spotLightDir", light.spotLightDirection);
	shader.SetFloat("lightIntensity", light.intensity);
	shader.SetInt("attenuationActive", light.attenuationActivated);
	shader.SetInt("lightType", (int)light.type);
	shader.SetFloat("constant", light.constant);
	shader.SetFloat("linear", light.linear);
	shader.SetFloat("quadratic", light.quadratic);
	shader.SetFloat("cutOff", light.cutOff);
	shader.SetFloat("ambientStrength", light.ambientIntensity);
	shader.SetFloat3("ambientColor", light.ambientColor);
	shader.SetFloat("diffuseStrength", light.diffuseIntensity);
	shader.SetFloat("specularStrength", light.specularIntensity);
	shader.SetFloat3("specularColor", light.specularColor);
	shader.SetFloat("shininessValue", light.shininessValue);

	shader.SetFloat("alphaIntensity", windowAlpha);
	glDrawArrays(GL_TRIANGLES, 0, numVertices);
	glUseProgram(0);
	glBindVertexArray(0);
}

void Object::Draw(Light light, glm::mat4 objMat[])
{
	shader.Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glBindVertexArray(ObjVao);

	// Variables que pasem als shaders com a uniforms per ser usats per la gr�fica
	if (usingStencil) shader.SetBool("Stencil", true);
	else shader.SetBool("Stencil", false);

	for (int i = 0; i < 10; i++)
	{
		shader.SetMat4("model[" + std::to_string(i)+"]", 1, GL_FALSE, glm::value_ptr(objMat[i]));
	}

	shader.SetFloat("alphaIntensity", windowAlpha);
	shader.SetMat4("view", 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
	shader.SetMat4("projection", 1, GL_FALSE, glm::value_ptr(RenderVars::_projection));
	shader.SetFloat3("lightColor", light.color);
	shader.SetFloat3("lightPos", light.position);
	shader.SetFloat3("spotLightDir", light.spotLightDirection);
	shader.SetFloat("lightIntensity", light.intensity);
	shader.SetInt("attenuationActive", light.attenuationActivated);
	shader.SetInt("lightType", (int)light.type);
	shader.SetFloat("constant", light.constant);
	shader.SetFloat("linear", light.linear);
	shader.SetFloat("quadratic", light.quadratic);
	shader.SetFloat("cutOff", light.cutOff);
	shader.SetFloat("ambientStrength", light.ambientIntensity);
	shader.SetFloat3("ambientColor", light.ambientColor);
	shader.SetFloat("diffuseStrength", light.diffuseIntensity);
	shader.SetFloat("specularStrength", light.specularIntensity);
	shader.SetFloat3("specularColor", light.specularColor);
	shader.SetFloat("shininessValue", light.shininessValue);

	glDrawArraysInstanced(GL_TRIANGLES, 0, numVertices, 10);
	glUseProgram(0);
	glBindVertexArray(0);
}

void Object::CleanUp()
{
	glDeleteBuffers(3, ObjVbo);
	glDeleteVertexArrays(1, &ObjVao);
	shader.CleanUpShader();

	glDeleteTextures(1, &textureID);
}