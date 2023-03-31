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
            std::vector<int> f;
            int itrash, idx;
            // Skip 'f'
            iss >> trash;
            // faces_ stores the index of vertices  
            while (iss >> idx >> trash >> itrash >> trash >> itrash)
            {
                idx--;
                f.push_back(idx);
            }
            faces_.push_back(f);
        }
        // uv line
        else if (!line.compare(0, 2, "vt"))
        {
            iss >> trash >> trash;
            Vec2f uv;
            for (int i = 0; i < 2; ++i)
            {
                iss >> uv.raw[i];
            }
            uv_.push_back(uv);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
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

std::vector<int> Model::face(int idx)
{
    return faces_[idx];
}

Vec2f Model::uv(int idx)
{
    return uv_[idx];
}
