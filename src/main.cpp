#include <iostream>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cmath>
#include <limits>

#include "Application.h"
#include "glad/glad.h"
#include "SolarGL.h"

#define app Application::getInstance()

int width = 1920;
int height = 1080;
constexpr int depth = 255 ;



Vec3f light_dir = Vec3f(-1,-1,1).normalize();
Vec3f camera(0,0,5);

//视线方向
Vec3f view(0, 0, -1);
//计算半程向量
Vec3f half = (light_dir + view).normalize();
//创建环境光强
constexpr float ambient_light = 0.1;

Model *model = nullptr;
int *zbuffer_ptr = nullptr;

Matrix viewPort(int x, int y, int w, int h)
{
	Matrix m = Matrix::identity(4);
	m[0][3] = x+w/2.f;
	m[1][3] = y+h/2.f;
	m[2][3] = depth/2.f;

	m[0][0] = w/2.f;
	m[1][1] = h/2.f;
	m[2][2] = depth/2.f;
	return m;
}


void OnResize(int width, int height)
{
	glViewport(0, 0, width, height);
	std::cout << "OnResize" << std::endl;
}

void OnKey(int key, int action, int mods)
{
	std::cout << key << std::endl;
}

GLuint createVBO(const ImageData& image) {
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// 将数据转换为 unsigned char 格式
	std::vector<unsigned char> imageDataBytes(image.width * image.height * 3);
	for (int y = 0; y < image.height; ++y) {
		for (int x = 0; x < image.width; ++x) {
			const Vec3i& color = image.data[x + y * image.width];
			imageDataBytes[(y * image.width + x) * 3 + 0] = static_cast<unsigned char>(color.x);
			imageDataBytes[(y * image.width + x) * 3 + 1] = static_cast<unsigned char>(color.y);
			imageDataBytes[(y * image.width + x) * 3 + 2] = static_cast<unsigned char>(color.z);
		}
	}

	// 上传数据到 VBO
	glBufferData(GL_ARRAY_BUFFER, imageDataBytes.size(), imageDataBytes.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return vbo;
}


//简单的顶点着色器
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    ourColor = aColor;
}
)";

//简单的片段着色器
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec3 ourColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)";

GLuint compileShader(const char* source, GLenum type) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	return shader;
}

GLuint createProgram(const char* vertexSource, const char* fragmentSource) {
	GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
	GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}


int main() {

	//申请zbuffer
	Zbuffer z_buffer(width, height);

	Matrix Projection = Matrix::identity(4);
	Matrix ViewPort   = viewPort(width/8, height/8, width*3/4, height*3/4);
	Projection[3][2] = -1.f/camera.z;


	//初始化窗体
	if (!app->init(1920, 1080))
	{
		return -1;
	}

	//设置回调
	app->setResizeCallback(OnResize);
	app->setKeyBoardCallback(OnKey);
	//初始化窗体和背景色
	glViewport(0, 0, 1920, 1080);
	glClearColor(0.2f, 0.2f, 0.22f, 1.0f);

	//---------------------------------------------------------------------------

	ImageData image;
	image.width = 1920;
	image.height = 1080;

	//创建 VBO
	GLuint vbo = createVBO(image);
	//创建着色器程序
	GLuint shaderProgram = createProgram(vertexShaderSource, fragmentShaderSource);

	//顶点数据
	float vertices[] = {
		-1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f
	};

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	model = new Model("model/earth_day.obj");

	render(ViewPort,Projection,
			   light_dir,
			   ambient_light,
			   width,
			   height,
			   z_buffer,
			   model,
			   image);

	//窗体循环
	while (app->update())
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

	}

	app->destroy();

	delete model;
	//z_buffer.fresh();

	return 0;
}

