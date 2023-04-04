#pragma once
#include <vector>
#include "geometry.h"

class Model
{
public:
    Model(const char* filename);
    ~Model();

    int nverts();
    int nfaces();
    Vec3f vert(int idx);
    // return the index of vertices, uv, and normal
    std::vector<Vec3i> face(int idx);
    Vec2f uv(int idx);

private:
    std::vector<Vec3f> verts_;
    // faces_ stores the index of vertices, uv, and normal -> Vec3i
    std::vector<std::vector<Vec3i>> faces_;
    // normalized uv coordinates
    std::vector<Vec2f> uv_;
};
