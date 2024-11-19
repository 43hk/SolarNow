#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
    std::vector<Vec3f> verts_;
    std::vector<std::vector<Vec3i>> faces_; // attention, this Vec3i means vertex/uv/normal
    std::vector<Vec3f> norms_;
    std::vector<Vec2f> uv_;
    TGAImage diffusemap_;
    void load_texture(std::string filename, const char *suffix, TGAImage &img);
public:
    Model(const char *filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f norm(int idx);
    Vec3f vert(int i);
    Vec2i uv(int idx);
    TGAColor diffuse(Vec2i uv);
    std::vector<int> face(int idx);

    std::vector<std::vector<Vec3i>> triangulate_face(int idx); // 新增方法：将面拆分为三角形
};

#endif //__MODEL_H__
