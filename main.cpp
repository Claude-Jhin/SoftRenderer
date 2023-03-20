#include "model.h"
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

Model* model = nullptr;
const int width = 800;
const int height = 800;

void DrawPixel(int x, int y, TGAImage& image, TGAColor color)
{
    image.set(x, y, color);
}

void DrawLine(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
    bool steep = false;
    if (abs(x1 - x0) < abs(y1 - y0))
    {
        swap(x0, y0);
        swap(x1, y1);
        steep = true;
    }

    if (x0 > x1)
    {
        swap(x0, x1);
        swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = abs(dy) * 2;
    int error = 0;
    int y = y0;
    for (int x = x0; x <= x1; ++x)
    {
        if (steep)
        {
            DrawPixel(y, x, image, color);
        }
        else
        {
            DrawPixel(x, y, image, color);
        }
        error += derror2;
        if (error > dx)
        {
            y += (y1 > y0 ? 1 : -1);
            error -= dx * 2;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc == 2)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);

    for (int i = 0; i < model->nfaces(); ++i)
    {
        std::vector<int> face = model->face(i);
        for (int j = 0; j < 3; ++j)
        {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j + 1) % 3]);
            int x0 = (v0.x + 1.)*width/2.;
            int y0 = (v0.y + 1.)*height/2.;
            int x1 = (v1.x + 1.)*width/2.;
            int y1 = (v1.y + 1.)*height/2.;
            DrawLine(x0, y0, x1, y1, image, white);
        }
    }
    // image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}
