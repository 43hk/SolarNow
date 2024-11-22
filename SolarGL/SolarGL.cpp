#define STB_IMAGE_IMPLEMENTATION

#include "SolarGL.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

//---------------------------------------------------------------------------------------
//geometry
template <> Vec3<float>::Vec3(Matrix m) : x(m[0][0] / m[3][0]), y(m[1][0] / m[3][0]), z(m[2][0] / m[3][0]) {}
template <> template <> Vec3<int>::Vec3(const Vec3<float>& v) : x(int(v.x + .5)), y(int(v.y + .5)), z(int(v.z + .5)) {}
template <> template <> Vec3<float>::Vec3(const Vec3<int>& v) : x(v.x), y(v.y), z(v.z) {}

Matrix::Matrix(Vec3f v) : m(std::vector<std::vector<float> >(4, std::vector<float>(1, 1.f))), rows(4), cols(1) {
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
}


Matrix::Matrix(int r, int c) : m(std::vector<std::vector<float> >(r, std::vector<float>(c, 0.f))), rows(r), cols(c) {}

int Matrix::nrows() {
    return rows;
}

int Matrix::ncols() {
    return cols;
}

Matrix Matrix::identity(int dimensions) {
    Matrix E(dimensions, dimensions);
    for (int i = 0; i < dimensions; i++) {
        for (int j = 0; j < dimensions; j++) {
            E[i][j] = (i == j ? 1.f : 0.f);
        }
    }
    return E;
}

std::vector<float>& Matrix::operator[](const int i) {
    assert(i >= 0 && i < rows);
    return m[i];
}

Matrix Matrix::operator*(const Matrix& a) {
    assert(cols == a.rows);
    Matrix result(rows, a.cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < a.cols; j++) {
            result.m[i][j] = 0.f;
            for (int k = 0; k < cols; k++) {
                result.m[i][j] += m[i][k] * a.m[k][j];
            }
        }
    }
    return result;
}

Matrix Matrix::transpose() {
    Matrix result(cols, rows);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            result[j][i] = m[i][j];
    return result;
}

Matrix Matrix::inverse() {
    assert(rows == cols);
    // augmenting the square matrix with the identity matrix of the same dimensions a => [ai]
    Matrix result(rows, cols * 2);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            result[i][j] = m[i][j];
    for (int i = 0; i < rows; i++)
        result[i][i + cols] = 1;
    // first pass
    for (int i = 0; i < rows - 1; i++) {
        // normalize the first row
        for (int j = result.cols - 1; j >= 0; j--)
            result[i][j] /= result[i][i];
        for (int k = i + 1; k < rows; k++) {
            float coeff = result[k][i];
            for (int j = 0; j < result.cols; j++) {
                result[k][j] -= result[i][j] * coeff;
            }
        }
    }
    // normalize the last row
    for (int j = result.cols - 1; j >= rows - 1; j--)
        result[rows - 1][j] /= result[rows - 1][rows - 1];
    // second pass
    for (int i = rows - 1; i > 0; i--) {
        for (int k = i - 1; k >= 0; k--) {
            float coeff = result[k][i];
            for (int j = 0; j < result.cols; j++) {
                result[k][j] -= result[i][j] * coeff;
            }
        }
    }
    // cut the identity matrix back
    Matrix truncate(rows, cols);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            truncate[i][j] = result[i][j + cols];
    return truncate;
}

std::ostream& operator<<(std::ostream& s, Matrix& m) {
    for (int i = 0; i < m.nrows(); i++) {
        for (int j = 0; j < m.ncols(); j++) {
            s << m[i][j];
            if (j < m.ncols() - 1) s << "\t";
        }
        s << "\n";
    }
    return s;
}



//-----------------------------------------------------------------------------
//model
Model::Model(const char* filename) : verts_(), faces_(), norms_(), uv_(), diffusemap_()
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v "))
        {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 3, "vn "))
        {
            iss >> trash >> trash;
            Vec3f n;
            for (int i = 0; i < 3; i++) iss >> n[i];
            norms_.push_back(n);
        } else if (!line.compare(0, 3, "vt "))
        {
            iss >> trash >> trash;
            Vec2f uv;
            for (int i = 0; i < 2; i++) iss >> uv[i];
            uv_.push_back(uv);
        } else if (!line.compare(0, 2, "f "))
        {
            std::vector<Vec3i> f;
            Vec3i tmp;
            iss >> trash;
            // 动态解析每个顶点
            while (iss >> tmp[0])
            {
                tmp[1] = tmp[2] = 0; // 默认值
                if (iss.peek() == '/')
                {
                    iss >> trash;
                    if (iss.peek() != '/') iss >> tmp[1]; // 纹理索引
                    if (iss.peek() == '/')
                    {
                        iss >> trash >> tmp[2]; // 法线索引
                    }
                }
                for (int i = 0; i < 3; i++) tmp[i]--; // OBJ 索引从 1 开始，数组是从0开始
                f.push_back(tmp);
            }
            faces_.push_back(f); // 存储面
        }
    }

    load_texture(filename, ".tga", diffusemap_);

    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << " vt# " << uv_.size() << " vn# " << norms_.
            size() << std::endl;
}

Model::~Model() {}


int Model::nfaces()
{
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx)
{
    std::vector<int> face;
    for (Vec3i v : faces_[idx]) {
        face.push_back(v[0]); // 只提取顶点索引
    }
    return face;
}

Vec3f Model::getVert(int idx)
{
    return verts_[idx];
}

void Model::load_texture(std::string filename, const char *suffix, TGAImage &img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot!=std::string::npos) {
        texfile = texfile.substr(0,dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}

TGAColor Model::diffuse(Vec2i uv) {
    return diffusemap_.get(uv.x , uv.y);
}

Vec2i Model::getUv(int idx)
{
    return Vec2i(uv_[idx].x * diffusemap_.get_width(), uv_[idx].y * diffusemap_.get_height());
}

Vec3f Model::getNorm(int idx)
{
    return norms_[idx].normalize();
}

std::vector<std::vector<Vec3i>> Model::triangulate_face(int idx) {
    std::vector<Vec3i> &face = faces_[idx];
    std::vector<std::vector<Vec3i>> triangles;

    if (face.size() == 4)
    {
        triangles.push_back({ face[0], face[1], face[2] });
        triangles.push_back({ face[0], face[2], face[3] });
    }
    if (face.size() == 3)
    {
        triangles.push_back({ face[0], face[1], face[2] });
    }

    return triangles;
}

//---------------------------------------------------------------------------------------
void triangleDraw(Vec3i &t0, Vec3i &t1, Vec3i &t2,
                  float &ity0, float &ity1, float &ity2,
                  Vec2i &uv0, Vec2i &uv1, Vec2i &uv2,
                  float ambient_light,
                  int width,
                  Zbuffer &zbuffer,
                  Model *model,
                  TGAImage &image)
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

            int Z_idx = P.x + P.y * width;
            if (zbuffer.buffer[Z_idx] < P.z)
            {
                zbuffer.buffer[Z_idx] = P.z;
                TGAColor color = model->diffuse(uvP);  // 获取纹理颜色
                image.set(P.x, P.y, color * (ityP > 0 ? (ityP + ambient_light) : ambient_light));
            }
        }

    }
}



void render(Matrix &ViewPort, Matrix &Projection,
            Vec3f &light_dir,
            float ambient_light,
            int width,
            int height,
            Zbuffer &zbuffer,
            Model *model,
            TGAImage &image)
{
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
                screen_coords[j] = Vec3f(ViewPort * Projection * Matrix(model->getVert(idx[0])));
                uv[j] = model->getUv(idx[1]);
                intensity[j] = std::max(model->getNorm(idx[2]) * light_dir, 0.f);
            }

            triangleDraw(screen_coords[0], screen_coords[1], screen_coords[2],
                     intensity[0], intensity[1], intensity[2],
                     uv[0], uv[1], uv[2],
                     ambient_light,
                     width,
                     zbuffer,
                     model,
                     image);
        }
    }
}


// 函数：获取指定目录下的所有 .jpg 和 .png 文件名
std::vector<std::string> getImageFiles(const std::string& directory) {
    std::vector<std::string> imageFiles;
    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();
                if (extension == ".jpg" || extension == ".png") {
                    imageFiles.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error(std::string("File system error: ") + e.what());
    }
    return imageFiles;
}


