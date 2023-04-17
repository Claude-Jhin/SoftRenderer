#include "model.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Model::Model(const char* filename)
    : verts_(),
      faces_(),
      uv_()
{
    // Use istream to read the file
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail())
    {
        return;
    }

    std::string line;
    // eof - end of file
    while (!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        // vertex line
        if (!line.compare(0, 2, "v "))
        {
            // Skip 'v'
            iss >> trash;
            // Read the vertex data
            Vec3f v;
            for (int i = 0; i < 3; ++i)
            {
                iss >> v.raw[i];
            }
            verts_.push_back(v);
        }
        // face line
        else if (!line.compare(0, 2, "f "))
        {
            std::vector<Vec3i> f;
            Vec3i temp;
            // Skip 'f'
            iss >> trash;
            // faces_ stores the index of vertices, uv, and normal -> Vec3i
            while (iss >> temp.raw[0] >> trash >> temp.raw[1] >> trash >> temp.raw[2])
            {
                // in wavefront obj all indices start at 1, not zero
                for (int i = 0; i < 3; ++i)
                {
                    temp.raw[i]--;
                }
                f.push_back(temp);
            }
            faces_.push_back(f);
        }
        // uv line
        else if (!line.compare(0, 2, "vt"))
        {
            // vt  0.298 0.774 0.000
            iss >> trash >> trash;
            Vec2f uv;
            for (int i = 0; i < 2; ++i)
            {
                iss >> uv.raw[i];
            }
            uv_.push_back(uv);
        }
        // normal line
        else if (!line.compare(0, 2, "vn"))
        {
            // vn  0.042 0.469 0.882
            iss >> trash >> trash;
            Vec3f n;
            for (int i = 0; i < 3; ++i)
            {
                iss >> n.raw[i];
            }
            norm_.push_back(n);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
    load_texture(filename, "_diffuse.tga", diffusemap_);
}

Model::~Model()
{
}

int Model::nverts()
{
    return (int)verts_.size();
}

int Model::nfaces()
{
    return (int)faces_.size();
}

Vec3f Model::vert(int idx)
{
    return verts_[idx];
}

std::vector<Vec3i> Model::face(int idx)
{
    return faces_[idx];
}

Vec2f Model::uv(int idx)
{
    return uv_[idx];
}

Vec3f Model::norm(int idx)
{
    return norm_[idx].normalize();
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage& img)
{
    std::string textfile(filename);
    auto dot = textfile.find_last_of(".");
    if (dot != std::string::npos)
    {
        textfile = textfile.substr(0, dot) + std::string(suffix);
        std::cerr << "texture file " << textfile << " loading " << (img.read_tga_file(textfile.c_str())
                                                                        ? "ok"
                                                                        : "failed") << endl;
        img.flip_vertically();
    }
}

TGAColor Model::diffuse(Vec2f uv)
{
    return diffusemap_.get(uv.x * diffusemap_.get_width(), uv.y * diffusemap_.get_height());
}
