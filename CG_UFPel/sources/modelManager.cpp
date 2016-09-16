#include "modelManager.hpp"
#include <GL/glew.h>
#include <model.hpp>
#include <texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include <iostream>
#include <shader.hpp>
#include "controls.hpp"


//Constructor
ModelManager::ModelManager(char *shaderPath1, char *shaderPath2)
{
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	programID = LoadShaders(shaderPath1, shaderPath2);
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	MatrixID = glGetUniformLocation(programID, "MVP");
}

//Getters
std::vector<Model> * ModelManager::getModels() {
	return &models;
}
std::vector<Mesh> * ModelManager::getMeshes() {
	return &meshes;
}
GLuint ModelManager::getProgramID() {
	return programID;
}
GLuint ModelManager::getVertexArrayID() {
	return VertexArrayID;
}
GLuint ModelManager::getMatrixID() {
	return MatrixID;
}
GLuint ModelManager::getLightID() {
	return LightID;
}

double Model::getLastTransformed() {
	return lastTransformed;
}

double Model::getTimeBtwn() {
	return timeBtwn;
}

//creates a new model and adds to the vector
void ModelManager::createModel(char *textPath, char *textSampler, Mesh &mesh, glm::vec3 position) {
	models.push_back(Model(textPath, textSampler, programID, mesh, position));
}
void ModelManager::createMesh(char *path) {
	meshes.push_back(Mesh(path));
}

void ModelManager::cleanup() {

	for (auto it = meshes.begin(); it != meshes.end(); ++it) {
		(*it).cleanup();
	
	}
	for (auto it = models.begin(); it != models.end(); ++it) {
		//Delete Texture
		glDeleteTextures(1, (*it).getTexture());
	}
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

}

void ModelManager::drawModels(GLuint ViewMatrixID, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, GLFWwindow* g_pWindow) {
	
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use our shader
	glUseProgram(programID);

	//For each model in the manager 
	for (auto it = models.begin(); it != models.end(); ++it) {
		//Calculate MVP matrix
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * (*it).getModelMatrix();

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv((*it).getModelMatrixID(), 1, GL_FALSE, &(*it).getModelMatrix()[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *(*it).getTexture());
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i((*it).getTextureID(), 0);

		(*(*it).getMesh()).loadMesh();

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,        // mode
			(*(*(*it).getMesh()).getIndices()).size(),      // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0             // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Draw tweak bars
		TwDraw();
	}
	// Swap buffers			
	glfwSwapBuffers(g_pWindow);
	glfwPollEvents();


}

void ModelManager::transformModels() {
	int i = 0;
	for (auto it = ModelManager::models.begin(); it != ModelManager::models.end(); ++it, i++) {
		//std::cout << (*it->getTransformationQueue()).size() << std::endl;
		//Queue not empty, state == 1, transformation ocurring, get time bewtween transformations and tests if it has already passed
		if (it->getState()) {
			it->applyTransformation();
		}
	//	else if (it->getState() && !(it->getTransformationQueue().empty()))
		//	std::cout << "Not enough time on model " << i << " has passed since last transformation..." << std::endl;
		//else if (it->getState() && it->getTransformationQueue().empty()) {		//Queue empty, transformations finished
			//std::cout << "Transformation of model " << i << " has finished. Queue is empty? " << it->getTransformationQueue().empty() << " Size: " << models[0].getTransformationQueue().size() << std::endl;
			//it->setState(0);
		//}
	}
}

void ModelManager::setModelTransformation(int modelID) {
	if (models.size() >= modelID) {
		models.at(modelID).setState(1);
		std::cout << "State of model " << modelID << " is now "<< models.at(modelID).getState() << ". Model is tranforming." << std::endl;
	}
}
