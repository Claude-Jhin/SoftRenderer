﻿#pragma once
#include <vector>
#include "geometry.h"
#include "tgaimage.h"

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
    Vec3f norm(int idx);

    void load_texture(std::string filename, const char* suffix, TGAImage& img);

    TGAColor diffuse(Vec2f uv);

private:
    std::vector<Vec3f> verts_;
    // faces_ stores the index of vertices, uv, and normal -> Vec3i
    std::vector<std::vector<Vec3i>> faces_;
    // normalized uv coordinates
    std::vector<Vec2f> uv_;

    std::vector<Vec3f> norm_;
    // diffuse map
    TGAImage diffusemap_;
};
