/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gráfico - Jogos Digitais - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 02/03/2022
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


// Protótipos das funções de teclado e mouse
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

//Protótipo da função de configuração da geometria do objeto 3D.
int setupGeometry();

//Protótipos das funções de textura e carregamento dos objetos
int loadTexture(string path);
void loadOBJ(string path);
void loadMTL(string path);

//Protótipos das funções de geração de ponto e animação
vector<glm::vec3> generateControlPointsSet(const std::string& input);
std::vector<glm::vec3> createVerticalCurve(float amplitude, int numPoints, float speed = 1.0f, float offset = 1.0f);

//Declaração do tamanho da janela
const GLuint WIDTH = 500, HEIGHT = 500;

//Declaração das variáveis de rotação da bola de futebol
bool rotateX = false, rotateY = false, rotateZ = false;
float footballX = 0.0f,footballY = 0.0f;
float moveNumber = 0.1f, maxMove = 10.0f, objectScale = 1.0f;
bool footballRotation = false;

//Declaração de variáveis de controle do objeto, textura e iluminação
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

//Declaração de variáveis com mapeamento dos objetos
string tresdModelsPaths = "../../3D_Models";
string footballPath = tresdModelsPaths + "/Football/bola.obj";
string basketballPath = tresdModelsPaths + "/Basketball/bola.obj";

//Declaração de variáveis Animação
int numPoints = 100;
float amplitudeBasketball = 2.0f;
float amplitudeFootball = -2.0f;
float speed = 1.0f;
float scale = 0.9f;
glm::vec3 translation(0.0f, 0.0f, 0.0f);
 
//Declaração da Camera
Camera camera;


// Função MAIN
int main()
{
	// Inicializa o contexto GLFW e cria uma janela GLFW
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GB -- Anderson e Carolina", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	//Configura as ações do mouse e do teclado
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	//Informa a posição do cursor na metade da tela
	glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Tratamento de erro
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
	}

	//Obtém informações sobre o renderizador do contexto OpenGL
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	//Obtém o tamanho do framebuffer 
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	
	//O tamanho do viewport é definido como o tamanho do framebuffer
	glViewport(0, 0, width, height);


	//Carregamento dos shaders
	Shader shader("../shaders/hello.vs", "../shaders/hello.fs");

	//Carregamento do objeto 3D e textura da bola de futebol
	loadOBJ(footballPath);
	loadMTL(tresdModelsPaths + "/Football/" + mtlFile);
	GLuint textureFootball = loadTexture(tresdModelsPaths + "/Football/" + texturePath);
	GLuint VAOFootball = setupGeometry();
	glUseProgram(shader.ID);

	//Inicialização do Mash
	Football.initialize(VAOFootball, positions.size() / 3, &shader, textureFootball, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(objectScale));

	//Carregamento do objeto 3D e textura da bola de basquete
	loadOBJ(basketballPath);
	loadMTL(tresdModelsPaths + "/Basketball/" + mtlFile);
	GLuint textureBasketball = loadTexture(tresdModelsPaths + "/Basketball/" + texturePath);
	//Vertex Array Object da bola de basquete obtem os estados dos vértices
	GLuint VAOBasketball = setupGeometry();
	//Especifica o Shader a ser usado
	glUseProgram(shader.ID);
	
	//Inicialização do Mash
	Basketball.initialize(VAOBasketball, positions.size() / 3, &shader, textureBasketball, glm::vec3(6.0f, 0.0f, 1.0f), glm::vec3(objectScale),0.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	//Inicialização da Camera
	camera.initialize(&shader, width, height, 0.05f,0.05f,-90.0f,glm::vec3(0.0f,0.0f, -1.0f), glm::vec3(1.5f, 0.0f, 15.0f));


	//Definição das propriedades do material para renderizar o objeto 3D.
	shader.setVec3("ka", ka[0], ka[1], ka[2]);
	shader.setFloat("kd", 0.5);
	shader.setVec3("ks", ks[0], ks[1], ks[2]);
	shader.setFloat("q", ns);

	//Definição das propriedades da luz para renderizar o objeto 3D
	shader.setVec3("lightPos", -2.0f, 100.0f, 2.0f);
	shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

	//Criação dos objetos para as bolas de basquete
	std::vector<glm::vec3> controlPointsBasketball = createVerticalCurve(amplitudeBasketball, numPoints, speed, 1.f);
	std::vector<glm::vec3> controlPointsBasketball2 = createVerticalCurve(amplitudeBasketball, numPoints, speed, 1.f);
	
	//Implementação da curva de bezier
	Bezier bezierBasketball;
	//Seta os pontos de controle que definem a curva
	bezierBasketball.setControlPoints(controlPointsBasketball);
	//Seta o shader
	bezierBasketball.setShader(&shader);
	//Gera a curva
	bezierBasketball.generateCurve(100);
	//Armazena os pontos 
	int nbPointsBasketball = bezierBasketball.getNbCurvePoints();

	//Criação dos pontos da bola de futebol.
	std::vector<glm::vec3> controlPointsFootball = createVerticalCurve(amplitudeFootball, numPoints, speed, 0.f);

	//Implementação da curva de belzier
	Bezier bezierFootball;
	//Seta os pontos de controle que definem a curva
	bezierFootball.setControlPoints(controlPointsFootball);
	//Seta o shader
	bezierFootball.setShader(&shader);
	//Gera a curva
	bezierFootball.generateCurve(100);
	int nbPointsFootball = bezierFootball.getNbCurvePoints();
	int i = 0;
	
	//Definição da cor da página
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

//Função que controla a seleção das teclas no teclado para movimentação da bola de futebol
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	camera.move(window, key, action);

	//Para a execução ao clicar na tecla ESC
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//Identifica o clique da tecla F para realizar as ações na bola de futebol
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		//Inverte o estado da variável, podendo ativar ou desativar os movimentos
		footballRotation = !footballRotation;

		//Caso o valor da variável seja falso, o movimento da bola de futebol irá parar.
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

	//Teclas UP/DOWN/LEFT/RIGHT fazem a translação do objeto
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
	//Realiza a movimentação da camera
	camera.rotate(window, xpos, ypos);
}

// Criação dos buffers que armazenam a geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{
	GLuint VAO, VBO[3];

	//Descrição dos identificadores para armazenar os dados da geometria
	glGenVertexArrays(1, &VAO);
	glGenBuffers(2, VBO);

	glBindVertexArray(VAO);

	//Buffer de vértices
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

	//Desvinculação dos buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Teste de profundidade
	glEnable(GL_DEPTH_TEST);

	return VAO;

}

//Função para carregar o objeto, que recebe como parametro o caminho onde ele está armazenado
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


			//Declaração de 3 vetores de indice
			glm::ivec3 vIndices, tIndices, nIndices;

			//Extração das informações do arquivo e armazenamento nos componentes vetor
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
				//Obtém o valor das coordenadas de vértice, textura e coordenadas da normal e armazenam nas variáveis, para uso na renderização
				const glm::vec3& vertex = vertexIndices[vIndices[i] - 1];
				const glm::vec2& texture = textureIndices[tIndices[i] - 1];
				const glm::vec3& normal = normalIndices[nIndices[i] - 1];

				//Armazena os componentes x, y e z aos vetores para uso na renderização dos objetos.
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
	// Variáveis locais para armazenar dados do arquivo MTL
	string line, readValue;
	ifstream mtlFile(path);

	//Loop para percorrer o arquivo MTL linha por linha
	while (!mtlFile.eof())
	{
		//Le a linha do arquivo
		getline(mtlFile, line);

		istringstream iss(line);

		// Verifica se a linha contém a tag "map_Kd" (mapeamento de textura difusa)
		if (line.find("map_Kd") == 0)
		{
			iss >> readValue >> texturePath;
		}
		// Verifica se a linha contém a tag "Ka" (coeficiente de absorção ambiente)
		else if (line.find("Ka") == 0)
		{
			// Variáveis para armazenar os valores R, G e B do Ka
			float ka1, ka2, ka3;
			iss >> readValue >> ka1 >> ka2 >> ka3;
			ka.push_back(ka1);
			ka.push_back(ka2);
			ka.push_back(ka3);
		}
		else if (line.find("Ks") == 0)
		{
			// Verifica se a linha contém a tag "Ks" (coeficiente de absorção especular)
			float ks1, ks2, ks3;
			iss >> readValue >> ks1 >> ks2 >> ks3;
			ks.push_back(ks1);
			ks.push_back(ks2);
			ks.push_back(ks3);
		}
		else if (line.find("Ns") == 0)
		{
			// Verifica se a linha contém a tag "Ks" (coeficiente de absorção especular) e extrai o valor dele
			iss >> readValue >> ns;
		}
	}
	mtlFile.close();
}

int loadTexture(string path)
{
	// Variável para o identificador da textura
	GLuint texID;

	// Geração do identificador da textura
	glGenTextures(1, &texID);

	// Vinculação da textura
	glBindTexture(GL_TEXTURE_2D, texID);

	// Configuração dos parâmetros de repetição da textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Configuração dos parâmetros de filtro da textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Carregamento dos dados da imagem da textura
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	// Verificação se os dados da imagem foram carregados corretamente
	if (data)
	{
		// Verificação do número de canais da imagem
		if (nrChannels == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		// Geração dos níveis mipmaps da textura
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "Failed to load texture" << endl;
	}
	// Liberação da memória dos dados da imagem
	stbi_image_free(data);

	// Desvinculação da textura
	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}

//Implementação do algoritmo que gera os pontos de controle para a classe de Belzier
std::vector<glm::vec3> generateControlPointsSet(const std::string& input) {
	// Vetor para armazenar os pontos de controle
	std::vector<glm::vec3> controlPoints;

	// Fluxo de entrada para ler os pontos de controle do string input
	std::istringstream iss(input);

	// Variáveis temporárias para armazenar as coordenadas dos pontos de controle
	float x, y, z;

	// Loop para ler os pontos de controle do fluxo de entrada
	while (iss >> x >> y >> z) {
		// Criação do vetor glm::vec3 com as coordenadas e adição ao vetor de pontos de controle
		controlPoints.push_back(glm::vec3(x, y, z));
	}

	// Retorna o vetor de pontos de controle
	return controlPoints;
}

//Função para criar a curva da animação das bolas de basquete
std::vector<glm::vec3> createVerticalCurve(float amplitude, int numPoints, float speed, float offset)
{
	// Vetor para armazenar os pontos da curva
	std::vector<glm::vec3> curvePoints;

	// Cálculo do incremento do ângulo com base no número de pontos e na velocidade
	float angleIncrement = (2.0f * glm::pi<float>() / numPoints) * speed;
	
	// Ângulo inicial
	float currentAngle = 0.0f;

	// Loop para criar os pontos da curva
	for (int i = 0; i < numPoints; i++) {
		// Cálculo das coordenadas x, y e z do ponto da curva
		float x = amplitude;
		float y = amplitude * std::cos(currentAngle);
		float z = amplitude;

		// Criação do vetor glm::vec3 com as coordenadas e adição ao vetor de pontos da curva
		curvePoints.push_back(glm::vec3(x, y, z));
		
		// Atualização do ângulo
		currentAngle += angleIncrement;
	}

	// Retorna o vetor de pontos da curva
	return curvePoints;
}