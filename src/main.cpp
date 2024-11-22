#include <iostream>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cmath>
#include <limits>
#include <filesystem>

#include "SolarGL.h"

constexpr int width = 1000;
constexpr int height = 1000;
constexpr int depth = 255;


Vec3f light_dir = Vec3f(-1,-1,1).normalize();
Vec3f camera(0,0,5);

//视线方向
Vec3f view(0, 0, -1);
//计算半程向量
Vec3f half = (light_dir + view).normalize();
//创建环境光强
constexpr float ambient_light = 0.1;

Model* model = nullptr;

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



int main()
{
	const std::string model_dir = "model";
	std::string filename = "";

	std::vector<std::string> imageFiles = getImageFiles(model_dir);

	if (imageFiles.empty())
	{
		std::cerr << "No .png files found in the current directory." << std::endl;
	}
	else
	{
		std::cout << "Found .png files in the current directory:" << std::endl;
		for (const auto& fname : imageFiles)
		{
			std::cout << ".png file is " << fname << std::endl;
			filename = fname.substr(0, fname.length() - 4);
			std::cout << filename << std::endl;
		}
	}



	//构建命令
	std::string convert_command = "ffmpeg -i " + model_dir + "/" + filename + ".png " + model_dir + "/" + filename + ".tga";
	std::cout << convert_command << std::endl;
	//执行命令
	int convert_result = system(convert_command.c_str());

	if (convert_result == 0) std::cout << "Success to convert" << std::endl;
	else std::cerr << "Failed to convert." << std::endl;


	//--------------------------------------------------------------------------
	//申请zbuffer
	Zbuffer z_buffer(width, height);

	Matrix Projection = Matrix::identity(4);
	Matrix ViewPort   = viewPort(width/8, height/8, width*3/4, height*3/4);
	Projection[3][2] = -1.f/camera.z;

	model = new Model("model/earth_day.obj");
	TGAImage image(width, height, TGAImage::RGB);

	render(ViewPort, Projection, light_dir, ambient_light, width, height, z_buffer, model, image);

	image.flip_vertically();
	image.write_tga_file("output.tga");

	// 释放内存
	delete model;


	//图片文件路径
	const char* display_path = "output.tga";
	//构建命令
	std::string display_command = "ffplay -i " + std::string(display_path);
	//执行命令
	int display_result = system(display_command.c_str());
	if (display_result == 0) std::cout << "Success to display" << std::endl;
	else std::cerr << "Failed to display." << std::endl;


	return 0;
}

