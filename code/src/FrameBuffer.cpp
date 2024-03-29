#include "FrameBuffer.h"

FrameBuffer::FrameBuffer() {}

FrameBuffer::FrameBuffer(const char* vertexPath, const char* fragmentPath)
{
#pragma region Quad
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbo);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), &texCoords, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#pragma endregion

#pragma region FrameBuffer + Texture
	// Set up frame buffer
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Creem la textura
	glGenTextures(1, &fboTexture);
	glBindTexture(GL_TEXTURE_2D, fboTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

	// Generem un Render Buffer Object pel depth buffer
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion Set up frame buffer i textura

	shader = Shader(vertexPath, fragmentPath);
}

void FrameBuffer::Update(glm::vec3 carPosition, glm::vec3 carRotation)
{
	objMat = glm::mat4(1);
	glm::mat4 t = glm::translate(glm::mat4(), mirrorPos);
	glm::mat4 r2 = glm::rotate(glm::mat4(), carRotation.y, glm::vec3(0, 1, 0));
	glm::mat4 t2 = glm::translate(glm::mat4(), carPosition);

	objMat = t2 * r2 * t;
}

void FrameBuffer::DrawQuadFBOTex()
{
	shader.Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboTexture);

	glBindVertexArray(vao);

	// Variables que passem als shaders com a uniforms per ser usats per la gr�fica
	shader.SetMat4("mvp", 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
	shader.SetMat4("objm", 1, GL_FALSE, glm::value_ptr(objMat));
	shader.SetFloat("width", 5);
	shader.SetFloat("height", 5);

	glDrawArrays(GL_TRIANGLES, 0, 6); // --> Dibuixa quad

	glUseProgram(0);
	glBindVertexArray(0);
}

void FrameBuffer::CleanUp()
{
	shader.CleanUpShader();
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(2, vbo);

	glDeleteTextures(1, &fboTexture);
}