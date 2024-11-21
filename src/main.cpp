#include <iostream>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cmath>
#include <limits>

#include "Application.h"
#include "glad/glad.h"
#include "SolarGL.h"
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"

#define app Application::getInstance()

constexpr int width = 2000;
constexpr int height = 2000;
constexpr int depth = 255 ;



Vec3f light_dir = Vec3f(-1,-1,1).normalize();
Vec3f camera(0,0,5);

//视线方向
Vec3f view(0, 0, -1);
//float light_strength = 0.7;
//计算半程向量
Vec3f half = (light_dir + view).normalize();
//创建环境光强
constexpr float ambient_light = 0.1;

Model *model = nullptr;
int *zbuffer = nullptr;

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


int main(int argc, char **argv) {
	model = new Model("model/earth_day.obj");


	//初始化z_buffer
	zbuffer = new int[width*height];
	for (int i=0; i<width*height; i++)
	{
		zbuffer[i] = std::numeric_limits<int>::min();
	}

	Matrix Projection = Matrix::identity(4);
	Matrix ViewPort   = viewPort(width/8, height/8, width*3/4, height*3/4);
	Projection[3][2] = -1.f/camera.z;





	if (!app->init(1920, 1080))
	{
		return -1;
	}


	app->setResizeCallback(OnResize);
	app->setKeyBoardCallback(OnKey);

	glViewport(0, 0, 1920, 1080);
	glClearColor(0.2f, 0.2f, 0.22f, 1.0f);


	while (app->update())
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}
	app->destroy();

	delete model;
	delete []zbuffer;

	return 0;
}

