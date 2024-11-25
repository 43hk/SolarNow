#pragma once


#include <vector>
#include <sstream>
#include <iostream>

#include "tgaimage.h"



//---------------------------------------------------------------------------------------
//geometry
class Matrix;

template <class t> struct Vec2
{
    t x, y;
    Vec2<t>() : x(t()), y(t()) {}
    Vec2<t>(t _x, t _y) : x(_x), y(_y) {}
    Vec2<t> operator +(const Vec2<t>& V) const { return Vec2<t>(x + V.x, y + V.y); }
    Vec2<t> operator -(const Vec2<t>& V) const { return Vec2<t>(x - V.x, y - V.y); }
    Vec2<t> operator *(float f)          const { return Vec2<t>(x * f, y * f); }
    t& operator[](const int i) { return i <= 0 ? x : y; }
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> struct Vec3
{
    t x, y, z;
    Vec3<t>() : x(t()), y(t()), z(t()) {}
    Vec3<t>(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
    Vec3<t>(Matrix m);
    template <class u> Vec3<t>(const Vec3<u>& v);
    Vec3<t> operator ^(const Vec3<t>& v) const { return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    Vec3<t> operator +(const Vec3<t>& v) const { return Vec3<t>(x + v.x, y + v.y, z + v.z); }
    Vec3<t> operator -(const Vec3<t>& v) const { return Vec3<t>(x - v.x, y - v.y, z - v.z); }
    Vec3<t> operator *(float f)          const { return Vec3<t>(x * f, y * f, z * f); }
    t       operator *(const Vec3<t>& v) const { return x * v.x + y * v.y + z * v.z; }
    float norm() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3<t>& normalize(t l = 1) { *this = (*this) * (l / norm()); return *this; }
    t& operator[](const int i) { return i <= 0 ? x : (1 == i ? y : z); }
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;

template <> template <> Vec3<int>::Vec3(const Vec3<float>& v);
template <> template <> Vec3<float>::Vec3(const Vec3<int>& v);


template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v)
{
    s << "(" << v.x << ", " << v.y << ")\n";
    return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v)
{
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
    return s;
}


class Matrix
{
    std::vector<std::vector<float> > m;
    int rows, cols;
public:
    Matrix(int r = 4, int c = 4);
    Matrix(Vec3f v);
    int nrows();
    int ncols();
    static Matrix identity(int dimensions);
    std::vector<float>& operator[](const int i);
    Matrix operator*(const Matrix& a);
    friend std::ostream& operator<<(std::ostream& s, Matrix& m);
};


//-------------------------------------------------------------------------
//model
class Model
{
    std::vector<Vec3f> verts_;
    std::vector<std::vector<Vec3i>> faces_; // attention, this Vec3i means vertex/uv/normal
    std::vector<Vec3f> norms_;
    std::vector<Vec2f> uv_;
    TGAImage diffusemap_;
    void load_texture(std::string filename, const char *suffix, TGAImage &img);
public:
    Model(const char* filename);
    ~Model();
    int nfaces();
    Vec3f getNorm(int idx);
    Vec3f getVert(int idx);
    Vec2i getUv(int idx);
    TGAColor diffuse(Vec2i uv);
    std::vector<int> face(int idx);
    std::vector<std::vector<Vec3i>> triangulate_face(int idx); // 新增方法：将面拆分为三角形
};


struct Zbuffer
{
    int width;
    int height;
    //缓存区
    std::vector<int> buffer;

    Zbuffer(const int w, const int h) : width(w), height(h), buffer(w * h)
    {
        for (int i=0; i<width*height; i++)
        {
            buffer[i] = std::numeric_limits<int>::min();
        }
    }

    ~Zbuffer()
    {
        //缓存缩为0
        buffer = std::vector<int>();
    }
    void fresh()
    {
        for (int i=0; i<width*height; i++)
        {
            buffer[i] = std::numeric_limits<int>::min();
        }
    }

};


void triangleDraw(Vec3i &t0, Vec3i &t1, Vec3i &t2,
                  float &ity0, float &ity1, float &ity2,
                  Vec2i &uv0, Vec2i &uv1, Vec2i &uv2,
                  float ambient_light,
                  int width,
                  Zbuffer &zbuffer,
                  Model *model,
                  TGAImage &image);

void render(Matrix &ViewPort, Matrix &Projection, Matrix &Rotation,
            Vec3f &light_dir,
            float ambient_light,
            int width,
            int height,
            Zbuffer &zbuffer,
            Model *model,
            TGAImage &image);

std::vector<std::string> getImageFiles(const std::string& directory);