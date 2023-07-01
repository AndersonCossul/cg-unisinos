#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

//Protótipos das funções da classe Camera
class Camera
{
public:
	Camera() {}
	~Camera() {}

	// Inicializa a câmera com os parâmetros fornecidos
	void initialize(Shader* shader, int width, int height, float sensitivity = 0.05, float pitch = 0.0, float yaw = -90.0, glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0), glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 3.0), glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0));
	
	// Move a câmera com base na tecla pressionada
	void move(GLFWwindow* window, int key, int action);

	// Rotaciona a câmera com base nos movimentos do mouse
	void rotate(GLFWwindow* window, double xpos, double ypos);

	// Atualiza a câmera
	void update();

protected:
	Shader* shader;
	bool firstMouse, rotateX, rotateY, rotateZ;
	float lastX, lastY, pitch, yaw;
	float sensitivity;
	glm::vec3 cameraFront, cameraPos, cameraUp;
};
#pragma once
