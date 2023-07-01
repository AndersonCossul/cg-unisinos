#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

//Prot�tipos das fun��es da classe Camera
class Camera
{
public:
	Camera() {}
	~Camera() {}

	// Inicializa a c�mera com os par�metros fornecidos
	void initialize(Shader* shader, int width, int height, float sensitivity = 0.05, float pitch = 0.0, float yaw = -90.0, glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0), glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 3.0), glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0));
	
	// Move a c�mera com base na tecla pressionada
	void move(GLFWwindow* window, int key, int action);

	// Rotaciona a c�mera com base nos movimentos do mouse
	void rotate(GLFWwindow* window, double xpos, double ypos);

	// Atualiza a c�mera
	void update();

protected:
	Shader* shader;
	bool firstMouse, rotateX, rotateY, rotateZ;
	float lastX, lastY, pitch, yaw;
	float sensitivity;
	glm::vec3 cameraFront, cameraPos, cameraUp;
};
#pragma once
