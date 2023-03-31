﻿#pragma once
#include <vector>
#include "geometry.h"

class Model
{
private:
    std::vector<Vec3f> verts_;
    std::vector<std::vector<int>> faces_;
    std::vector<Vec2f> uv_;
public:
    Model(const char* filename);
    ~Model();

    int nverts();
    int nfaces();
    Vec3f vert(int idx);
    std::vector<int> face(int idx);
    Vec2f uv(int idx);
};
