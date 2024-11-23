#include <iostream>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cmath>
#include <limits>
#include <filesystem>
#include <numbers>

#include "SolarGL.h"


namespace fs = std::filesystem;


constexpr int width = 1000;
constexpr int height = 1000;
constexpr int depth = 255;


Vec3f light_dir = Vec3f(1,-1,1).normalize();
Vec3f camera(0,0,5);

//视线方向
Vec3f view(0, 0, -1);
//计算半程向量
Vec3f half = (light_dir + view).normalize();
//创建环境光强
float ambient_light = .0;

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

Matrix rotationY(float angle) {
	Matrix m = Matrix::identity(4);
	m[0][0] = std::cos(angle);
	m[0][2] = std::sin(angle);
	m[2][0] = -std::sin(angle);
	m[2][2] = std::cos(angle);
	return m;
}


int main()
{
	std::cout << "是否已经渲染过？（若已经更模型请选择n）（y/n）：";
	char  temp = ' ';
	std::cin >> temp;
	if (temp == 'N' || temp == 'n')
	{
		std::cout << "请设定环境光强度，它会影响背光面亮度（如果类似于宇宙环境，请尽可能降低环境光）：";
		std::cin >> ambient_light;

		const std::string model_dir = "model";
		//先清空output文件夹，尽管可以在已经存在渲染序列的情况下直接jump到display
		//然而考虑到可能会更换model，所以每次运行都重新渲染
		// 使用 del 命令删除文件夹中的所有文件
		std::string command = "del /q " + std::string("output\\*");
		int result = system(command.c_str());
		if (result == 0)
		{
			std::cout << "Directory has been cleared." << std::endl;
		}
		else
		{
			std::cerr << "Failed to clear the directory." << std::endl;
		}

		//--------------------------------------------------------------------------
		// 查找文件夹下的 .obj 文件
		std::string obj_file;
		for (const auto& entry : fs::directory_iterator(model_dir))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".obj")
			{
				if (obj_file.empty()) obj_file = entry.path().string();
			}
		}

		if (obj_file.empty())
		{
			std::cerr << "No .obj file found in the directory." << std::endl;
			return 1;
		}

		//查找png贴图并转换位tga
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
				std::string filename = fname.substr(0, fname.length() - 4);
				std::string tgaFile = model_dir + "/" + filename + ".tga";

				// 检查 .tga 文件是否已存在
				if (!std::filesystem::exists(tgaFile))
				{
					std::string convert_command = "ffmpeg -i " + model_dir + "/" + fname + " " + tgaFile;
					std::cout << convert_command << std::endl;

					int convert_result = system(convert_command.c_str());
					if (convert_result == 0)
					{
						std::cout << "Success to convert " << fname << " to " << tgaFile << std::endl;
					}
					else
					{
						std::cerr << "Failed to convert " << fname << " to " << tgaFile << std::endl;
					}
				}
				else
				{
					std::cout << "File " << tgaFile << " already exists, skipping conversion." << std::endl;
				}
			}
		}


		//--------------------------------------------------------------------------
		//初始化资源
		Zbuffer z_buffer(width, height);
		model = new Model(obj_file.data());
		TGAImage image(width, height, TGAImage::RGB);

		//--------------------------------------------------------------------------
		//设定视角
		Matrix Projection = Matrix::identity(4);
		Matrix ViewPort   = viewPort(width/8, height/8, width*3/4, height*3/4);

		Projection[3][2] = -1.f/camera.z;



		//-------------------------------------------------------------------------
		//执行渲染循环写入
		for (int i = 0;i < 121;++i)//
		{
			float angle = i * std::numbers::pi / 60 ;
			Matrix Rotation = rotationY(angle);

			render(ViewPort, Projection, Rotation, light_dir, ambient_light, width, height, z_buffer, model, image);

			image.flip_vertically();
			std::ostringstream stream;
			stream << std::setw(3) << std::setfill('0') << i;
			std::string output_file = "output/output" + stream.str() + ".tga";
			image.write_tga_file(output_file.c_str());

			z_buffer.fresh();

			std::cout << ".";
		}
	}
	else if (temp == 'Y' || temp == 'y'){}
	else
	{
		std::cout << "请正确输入";
		return 1;
	}

	//-------------------------------------------------------------------------
	//展示
	std::string display_command = R"(ffmpeg\ffplay -loop 0 -vf "fps=24" -pattern_type sequence -i output\output%03d.tga)";
	int display_result = system(display_command.c_str());
	if (display_result == 0) std::cout << "Success to display" << std::endl;
	else std::cerr << "Failed to display." << std::endl;



	// 释放内存
	delete model;

	return 0;
}

