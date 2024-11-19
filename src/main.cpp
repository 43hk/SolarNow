#include <iostream>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cmath>
#include <limits>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

constexpr int width = 1600;
constexpr int height = 1600;
constexpr int depth = 255 ;

const auto white = TGAColor(255, 255, 255, 255);
const auto red = TGAColor(255, 0, 0, 255);
const auto green = TGAColor(0, 255, 0, 255);


Vec3f light_dir = Vec3f(-1,-1,1).normalize();
Vec3f camera(0,0,5);

//视线方向
Vec3f view(0, 0, -1);
//float light_strength = 0.7;
//计算半程向量
Vec3f half = (light_dir + view).normalize();
//创建环境光强
constexpr float ambient_light = 0.2;

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

Vec2i operator/(Vec2i a, float b)
{
	return {static_cast<int>(a.x*b), static_cast<int>(a.y*b)};
}

void triangleDraw(Vec3i &t0, Vec3i &t1, Vec3i &t2, float &ity0, float &ity1, float &ity2,
              Vec2i &uv0, Vec2i &uv1, Vec2i &uv2, TGAImage &image, int *zbuffer)
{
    if (t0.y == t1.y && t0.y == t2.y) return;  // 退化三角形忽略

    // 排序顶点，确保从低到高的顺序是 t0, t1, t2
    if (t0.y > t1.y) { std::swap(t0, t1); std::swap(ity0, ity1); std::swap(uv0, uv1); }
    if (t0.y > t2.y) { std::swap(t0, t2); std::swap(ity0, ity2); std::swap(uv0, uv2); }
    if (t1.y > t2.y) { std::swap(t1, t2); std::swap(ity1, ity2); std::swap(uv1, uv2); }

    int total_height = t2.y - t0.y;

    for (int i = 0; i < total_height; i++)
    {
        bool second_half = i > t1.y - t0.y || t1.y == t0.y; // 判断是否在下半部分
        int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y; // 片段高度
        float alpha = (float)i / total_height;
        float beta  = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;

        Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
        Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;

        float ityA = ity0 + (ity2 - ity0) * alpha;
        float ityB = second_half ? ity1 + (ity2 - ity1) * beta : ity0 + (ity1 - ity0) * beta;

    	Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
    	Vec2i uvB = second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;

        if (A.x > B.x) { std::swap(A, B); std::swap(ityA, ityB); std::swap(uvA, uvB); }

        for (int j = A.x; j <= B.x; j++)
        {
            float phi = (B.x == A.x) ? 1.f : (float)(j - A.x) / (float)(B.x - A.x); // 插值参数
            Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;  // 当前点
        	Vec2i uvP =  uvA + (uvB - uvA) * phi;

        	float ityP = ityA + (ityB - ityA) * phi; // 当前点的光照强度

            int idx = P.x + P.y * width;
            if (zbuffer[idx] < P.z)
            {
                zbuffer[idx] = P.z;
                TGAColor color = model->diffuse(uvP);  // 获取纹理颜色
                image.set(P.x, P.y, color * (ityP > 0 ? (ityP + ambient_light) : 0));
            }
        }
    }
}


int main(int argc, char **argv) {
	model = new Model("model/earth_day.obj");
	TGAImage image(width, height, TGAImage::RGB);


	//初始化z_buffer
	zbuffer = new int[width*height];
	for (int i=0; i<width*height; i++)
	{
		zbuffer[i] = std::numeric_limits<int>::min();
	}

	Matrix Projection = Matrix::identity(4);
	Matrix ViewPort   = viewPort(width/8, height/8, width*3/4, height*3/4);
	Projection[3][2] = -1.f/camera.z;


	//绘制过程计时
	const auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < model->nfaces(); i++)
	{
		auto triangles = model->triangulate_face(i); // 将面拆分为多个三角形

		for (auto &triangle : triangles)
		{
			Vec3i screen_coords[3];
			Vec2i uv[3];
			float intensity[3];

			for (int j = 0; j < 3; j++)
			{
				Vec3i idx = triangle[j];
				screen_coords[j] = Vec3f(ViewPort * Projection * Matrix(model->vert(idx[0])));
				uv[j] = model->uv(idx[1]);
				intensity[j] = std::max(model->norm(idx[2]) * light_dir, 0.f);
			}

			triangleDraw(screen_coords[0], screen_coords[1], screen_coords[2],
					 intensity[0], intensity[1], intensity[2],
					 uv[0], uv[1], uv[2],
					 image, zbuffer);
		}
	}

	//计时结束
	const auto end = std::chrono::high_resolution_clock::now();
	const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	//输出绘制时间
	std::cout << "绘制时间: " << duration << "毫秒" << std::endl;


	image.flip_vertically();
	image.write_tga_file("output.tga");
	delete model;
	delete []zbuffer;

	//用ffmpeg库写一个convert批处理，在导出TGA图片后自转换为PNG方便查看
	std::string batFilePath = "convert.bat";
	system(batFilePath.c_str());

	return 0;
}

