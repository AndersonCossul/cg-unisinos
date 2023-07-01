/* Hello Triangle - c�digo adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gr�fico - Jogos Digitais - Unisinos
 * Vers�o inicial: 7/4/2017
 * �ltima atualiza��o em 02/03/2022
 *
 */

//Bibliotecas
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#define STB_IMAGE_IMPLEMENTATION

//Linkagem de arquivos
#include <stb_image.h>
#include "Mesh.h"
#include "Camera.h"
#include "Hermite.h"
#include "Bezier.h"
#include "CatmullRom.h"


// Prot�tipos das fun��es de teclado e mouse
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

//Prot�tipo da fun��o de configura��o da geometria do objeto 3D.
int setupGeometry();

//Prot�tipos das fun��es de textura e carregamento dos objetos
int loadTexture(string path);
void loadOBJ(string path);
void loadMTL(string path);

//Prot�tipos das fun��es de gera��o de ponto e anima��o
vector<glm::vec3> generateControlPointsSet(const std::string& input);
std::vector<glm::vec3> createVerticalCurve(float amplitude, int numPoints, float speed = 1.0f, float offset = 1.0f);

//Declara��o do tamanho da janela
const GLuint WIDTH = 500, HEIGHT = 500;

//Declara��o das vari�veis de rota��o da bola de futebol
bool rotateX = false, rotateY = false, rotateZ = false;
float footballX = 0.0f,footballY = 0.0f;
float moveNumber = 0.1f, maxMove = 10.0f, objectScale = 1.0f;
bool footballRotation = false;

//Declara��o de vari�veis de controle do objeto, textura e ilumina��o
vector<GLfloat> positions;
vector<GLfloat> textureCoords;
vector<GLfloat> normals;
vector<GLfloat> ka;
vector<GLfloat> ks;
float ns;
string mtlFile = "";
string texturePath = "";
Mesh Football;
Mesh Basketball;

//Declara��o de vari�veis com mapeamento dos objetos
string tresdModelsPaths = "../../3D_Models";
string footballPath = tresdModelsPaths + "/Football/bola.obj";
string basketballPath = tresdModelsPaths + "/Basketball/bola.obj";

//Declara��o de vari�veis Anima��o
int numPoints = 100;
float amplitudeBasketball = 2.0f;
float amplitudeFootball = -2.0f;
float speed = 1.0f;
float scale = 0.9f;
glm::vec3 translation(0.0f, 0.0f, 0.0f);
 
//Declara��o da Camera
Camera camera;


// Fun��o MAIN
int main()
{
	// Inicializa o contexto GLFW e cria uma janela GLFW
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GB -- Anderson e Carolina", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	//Configura as a��es do mouse e do teclado
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	//Informa a posi��o do cursor na metade da tela
	glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Tratamento de erro
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
	}

	//Obt�m informa��es sobre o renderizador do contexto OpenGL
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	//Obt�m o tamanho do framebuffer 
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	
	//O tamanho do viewport � definido como o tamanho do framebuffer
	glViewport(0, 0, width, height);


	//Carregamento dos shaders
	Shader shader("../shaders/hello.vs", "../shaders/hello.fs");

	//Carregamento do objeto 3D e textura da bola de futebol
	loadOBJ(footballPath);
	loadMTL(tresdModelsPaths + "/Football/" + mtlFile);
	GLuint textureFootball = loadTexture(tresdModelsPaths + "/Football/" + texturePath);
	GLuint VAOFootball = setupGeometry();
	glUseProgram(shader.ID);

	//Inicializa��o do Mash
	Football.initialize(VAOFootball, positions.size() / 3, &shader, textureFootball, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(objectScale));

	//Carregamento do objeto 3D e textura da bola de basquete
	loadOBJ(basketballPath);
	loadMTL(tresdModelsPaths + "/Basketball/" + mtlFile);
	GLuint textureBasketball = loadTexture(tresdModelsPaths + "/Basketball/" + texturePath);
	//Vertex Array Object da bola de basquete obtem os estados dos v�rtices
	GLuint VAOBasketball = setupGeometry();
	//Especifica o Shader a ser usado
	glUseProgram(shader.ID);
	
	//Inicializa��o do Mash
	Basketball.initialize(VAOBasketball, positions.size() / 3, &shader, textureBasketball, glm::vec3(6.0f, 0.0f, 1.0f), glm::vec3(objectScale),0.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	//Inicializa��o da Camera
	camera.initialize(&shader, width, height, 0.05f,0.05f,-90.0f,glm::vec3(0.0f,0.0f, -1.0f), glm::vec3(1.5f, 0.0f, 15.0f));


	//Defini��o das propriedades do material para renderizar o objeto 3D.
	shader.setVec3("ka", ka[0], ka[1], ka[2]);
	shader.setFloat("kd", 0.5);
	shader.setVec3("ks", ks[0], ks[1], ks[2]);
	shader.setFloat("q", ns);

	//Defini��o das propriedades da luz para renderizar o objeto 3D
	shader.setVec3("lightPos", -2.0f, 100.0f, 2.0f);
	shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

	//Cria��o dos objetos para as bolas de basquete
	std::vector<glm::vec3> controlPointsBasketball = createVerticalCurve(amplitudeBasketball, numPoints, speed, 1.f);
	std::vector<glm::vec3> controlPointsBasketball2 = createVerticalCurve(amplitudeBasketball, numPoints, speed, 1.f);
	
	//Implementa��o da curva de bezier
	Bezier bezierBasketball;
	//Seta os pontos de controle que definem a curva
	bezierBasketball.setControlPoints(controlPointsBasketball);
	//Seta o shader
	bezierBasketball.setShader(&shader);
	//Gera a curva
	bezierBasketball.generateCurve(100);
	//Armazena os pontos 
	int nbPointsBasketball = bezierBasketball.getNbCurvePoints();

	//Cria��o dos pontos da bola de futebol.
	std::vector<glm::vec3> controlPointsFootball = createVerticalCurve(amplitudeFootball, numPoints, speed, 0.f);

	//Implementa��o da curva de belzier
	Bezier bezierFootball;
	//Seta os pontos de controle que definem a curva
	bezierFootball.setControlPoints(controlPointsFootball);
	//Seta o shader
	bezierFootball.setShader(&shader);
	//Gera a curva
	bezierFootball.generateCurve(100);
	int nbPointsFootball = bezierFootball.getNbCurvePoints();
	int i = 0;
	
	//Defini��o da cor da p�gina
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		camera.update();

		Football.updatePosition(glm::vec3(footballX, footballY, 0.0f));
		Football.update();
		if (rotateX) {
			Football.rotate(1, glm::vec3(1.0f, 0.0f, 0.0f));
		}
		if (rotateY) {
			Football.rotate(1, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (rotateZ) {
			Football.rotate(1, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		Football.draw();


		glm::vec3 pointOnCurveBasketball = bezierBasketball.getPointOnCurve(i);
		Basketball.updatePosition(pointOnCurveBasketball);
		Basketball.update();
		Basketball.rotate(1, glm::vec3(0.0f, 1.0f, 0.0f));
		Basketball.draw();

		i = (i + 1) % nbPointsBasketball;

		glm::vec3 pointOnCurveFootball = bezierFootball.getPointOnCurve(i);
		Basketball.updatePosition(pointOnCurveFootball);
		Basketball.update();
		Basketball.rotate(1, glm::vec3(0.0f, 1.0f, 0.0f));
		Basketball.draw();

		i = (i + 1) % nbPointsFootball;

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAOFootball);
	glDeleteVertexArrays(1, &VAOBasketball);
	glfwTerminate();
	return 0;
}

//Fun��o que controla a sele��o das teclas no teclado para movimenta��o da bola de futebol
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	camera.move(window, key, action);

	//Para a execu��o ao clicar na tecla ESC
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//Identifica o clique da tecla F para realizar as a��es na bola de futebol
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		//Inverte o estado da vari�vel, podendo ativar ou desativar os movimentos
		footballRotation = !footballRotation;

		//Caso o valor da vari�vel seja falso, o movimento da bola de futebol ir� parar.
		if (!footballRotation) {
			rotateX = false;
			rotateY = false;
			rotateZ = false;
			footballY = 0.0f;
			footballX = 0.0f;
		}
	}

	//Tecla X - Rotaciona no eixo X
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = footballRotation;
		rotateY = false;
		rotateZ = false;
	}

	//Tecla Y - Rotaciona no eixo Y
	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = footballRotation;
		rotateZ = false;
	}

	//Tecla Z - Rotaciona no eixo Z
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = footballRotation;
	}

	//Tecla I - Aumenta a escala
	if (key == GLFW_KEY_I && footballRotation && action == GLFW_PRESS) {
		Football.increase();
	}

	//Tecla J - Diminui a escala
	if (key == GLFW_KEY_J && footballRotation && action == GLFW_PRESS) {
		Football.decrease();
	}

	//Teclas UP/DOWN/LEFT/RIGHT fazem a transla��o do objeto
	if (footballRotation && action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_UP:
			if (footballY < maxMove) {
				footballY += moveNumber; 
			}
			else {
				footballY = 0.0f;
			}
			break;
		case GLFW_KEY_DOWN:
			if (footballY > -maxMove) {
				footballY -= moveNumber; 
			}
			else {
				footballY = 0.0f;
			}
			break;
		case GLFW_KEY_RIGHT:
			if (footballX < maxMove) {
				footballX += moveNumber;
			}
			else {
				footballX = 0.0f;
			}
		case GLFW_KEY_LEFT:
			cout << "max" << maxMove << "sunX" << footballX << endl;
			if (footballX > -maxMove) {
				footballX -= moveNumber; 
			}
			else {
				footballX = 0.0f;
			}
			break;
		
			break;
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//Realiza a movimenta��o da camera
	camera.rotate(window, xpos, ypos);
}

// Cria��o dos buffers que armazenam a geometria de um tri�ngulo
// Apenas atributo coordenada nos v�rtices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A fun��o retorna o identificador do VAO
int setupGeometry()
{
	GLuint VAO, VBO[3];

	//Descri��o dos identificadores para armazenar os dados da geometria
	glGenVertexArrays(1, &VAO);
	glGenBuffers(2, VBO);

	glBindVertexArray(VAO);

	//Buffer de v�rtices
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//Buffer de coordenadas
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(GLfloat), textureCoords.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	//Buffer de normais
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	//Desvincula��o dos buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Teste de profundidade
	glEnable(GL_DEPTH_TEST);

	return VAO;

}

//Fun��o para carregar o objeto, que recebe como parametro o caminho onde ele est� armazenado
void loadOBJ(string path)
{
	//Cria vetores de vertices e textura
	vector<glm::vec3> vertexIndices;
	vector<glm::vec2> textureIndices;
	vector<glm::vec3> normalIndices;

	//Tratamento de erro
	ifstream file(path);
	if (!file.is_open())
	{
		cerr << "Failed to open file: " << path << endl;
		return;
	}

	//Realiza a leitura do arquivo OBJ
	string line;
	while (getline(file, line))
	{
		istringstream iss(line);
		string prefix;
		iss >> prefix;

		if (prefix == "mtllib")
		{
			iss >> mtlFile;
		}
		else if (prefix == "v")
		{
			float x, y, z;
			iss >> x >> y >> z;
			vertexIndices.push_back(glm::vec3(x, y, z));
		}
		else if (prefix == "vt")
		{
			float u, v;
			iss >> u >> v;
			textureIndices.push_back(glm::vec2(u, v));
		}
		else if (prefix == "vn")
		{
			float x, y, z;
			iss >> x >> y >> z;
			normalIndices.push_back(glm::vec3(x, y, z));
		}
		else if (prefix == "f")
		{
			string v1, v2, v3;
			iss >> v1 >> v2 >> v3;


			//Declara��o de 3 vetores de indice
			glm::ivec3 vIndices, tIndices, nIndices;

			//Extra��o das informa��es do arquivo e armazenamento nos componentes vetor
			istringstream(v1.substr(0, v1.find('/'))) >> vIndices.x;
			istringstream(v1.substr(v1.find('/') + 1, v1.rfind('/') - v1.find('/') - 1)) >> tIndices.x;
			istringstream(v1.substr(v1.rfind('/') + 1)) >> nIndices.x;
			istringstream(v2.substr(0, v2.find('/'))) >> vIndices.y;
			istringstream(v2.substr(v2.find('/') + 1, v2.rfind('/') - v2.find('/') - 1)) >> tIndices.y;
			istringstream(v2.substr(v2.rfind('/') + 1)) >> nIndices.y;
			istringstream(v3.substr(0, v3.find('/'))) >> vIndices.z;
			istringstream(v3.substr(v3.find('/') + 1, v3.rfind('/') - v3.find('/') - 1)) >> tIndices.z;
			istringstream(v3.substr(v3.rfind('/') + 1)) >> nIndices.z;

			//Processamento dos vetores
			for (int i = 0; i < 3; i++)
			{
				//Obt�m o valor das coordenadas de v�rtice, textura e coordenadas da normal e armazenam nas vari�veis, para uso na renderiza��o
				const glm::vec3& vertex = vertexIndices[vIndices[i] - 1];
				const glm::vec2& texture = textureIndices[tIndices[i] - 1];
				const glm::vec3& normal = normalIndices[nIndices[i] - 1];

				//Armazena os componentes x, y e z aos vetores para uso na renderiza��o dos objetos.
				positions.push_back(vertex.x);
				positions.push_back(vertex.y);
				positions.push_back(vertex.z);
				textureCoords.push_back(texture.x);
				textureCoords.push_back(texture.y);
				normals.push_back(normal.x);
				normals.push_back(normal.y);
				normals.push_back(normal.z);
			}
		}
	}

	file.close();
}


void loadMTL(string path)
{
	// Vari�veis locais para armazenar dados do arquivo MTL
	string line, readValue;
	ifstream mtlFile(path);

	//Loop para percorrer o arquivo MTL linha por linha
	while (!mtlFile.eof())
	{
		//Le a linha do arquivo
		getline(mtlFile, line);

		istringstream iss(line);

		// Verifica se a linha cont�m a tag "map_Kd" (mapeamento de textura difusa)
		if (line.find("map_Kd") == 0)
		{
			iss >> readValue >> texturePath;
		}
		// Verifica se a linha cont�m a tag "Ka" (coeficiente de absor��o ambiente)
		else if (line.find("Ka") == 0)
		{
			// Vari�veis para armazenar os valores R, G e B do Ka
			float ka1, ka2, ka3;
			iss >> readValue >> ka1 >> ka2 >> ka3;
			ka.push_back(ka1);
			ka.push_back(ka2);
			ka.push_back(ka3);
		}
		else if (line.find("Ks") == 0)
		{
			// Verifica se a linha cont�m a tag "Ks" (coeficiente de absor��o especular)
			float ks1, ks2, ks3;
			iss >> readValue >> ks1 >> ks2 >> ks3;
			ks.push_back(ks1);
			ks.push_back(ks2);
			ks.push_back(ks3);
		}
		else if (line.find("Ns") == 0)
		{
			// Verifica se a linha cont�m a tag "Ks" (coeficiente de absor��o especular) e extrai o valor dele
			iss >> readValue >> ns;
		}
	}
	mtlFile.close();
}

int loadTexture(string path)
{
	// Vari�vel para o identificador da textura
	GLuint texID;

	// Gera��o do identificador da textura
	glGenTextures(1, &texID);

	// Vincula��o da textura
	glBindTexture(GL_TEXTURE_2D, texID);

	// Configura��o dos par�metros de repeti��o da textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Configura��o dos par�metros de filtro da textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Carregamento dos dados da imagem da textura
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	// Verifica��o se os dados da imagem foram carregados corretamente
	if (data)
	{
		// Verifica��o do n�mero de canais da imagem
		if (nrChannels == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		// Gera��o dos n�veis mipmaps da textura
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture" << endl;
	}
	// Libera��o da mem�ria dos dados da imagem
	stbi_image_free(data);

	// Desvincula��o da textura
	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}

//Implementa��o do algoritmo que gera os pontos de controle para a classe de Belzier
std::vector<glm::vec3> generateControlPointsSet(const std::string& input) {
	// Vetor para armazenar os pontos de controle
	std::vector<glm::vec3> controlPoints;

	// Fluxo de entrada para ler os pontos de controle do string input
	std::istringstream iss(input);

	// Vari�veis tempor�rias para armazenar as coordenadas dos pontos de controle
	float x, y, z;

	// Loop para ler os pontos de controle do fluxo de entrada
	while (iss >> x >> y >> z) {
		// Cria��o do vetor glm::vec3 com as coordenadas e adi��o ao vetor de pontos de controle
		controlPoints.push_back(glm::vec3(x, y, z));
	}

	// Retorna o vetor de pontos de controle
	return controlPoints;
}

//Fun��o para criar a curva da anima��o das bolas de basquete
std::vector<glm::vec3> createVerticalCurve(float amplitude, int numPoints, float speed, float offset)
{
	// Vetor para armazenar os pontos da curva
	std::vector<glm::vec3> curvePoints;

	// C�lculo do incremento do �ngulo com base no n�mero de pontos e na velocidade
	float angleIncrement = (2.0f * glm::pi<float>() / numPoints) * speed;
	
	// �ngulo inicial
	float currentAngle = 0.0f;

	// Loop para criar os pontos da curva
	for (int i = 0; i < numPoints; i++) {
		// C�lculo das coordenadas x, y e z do ponto da curva
		float x = amplitude;
		float y = amplitude * std::cos(currentAngle);
		float z = amplitude;

		// Cria��o do vetor glm::vec3 com as coordenadas e adi��o ao vetor de pontos da curva
		curvePoints.push_back(glm::vec3(x, y, z));
		
		// Atualiza��o do �ngulo
		currentAngle += angleIncrement;
	}

	// Retorna o vetor de pontos da curva
	return curvePoints;
}