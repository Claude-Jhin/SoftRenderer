#include "model.h"
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

Model* model = nullptr;
const int width = 200;
const int height = 200;

void DrawPixel(int x, int y, TGAImage& image, TGAColor color)
{
    image.set(x, y, color);
}

void DrawLine(Vec2i t0, Vec2i t1, TGAImage& image, TGAColor color)
{
    bool steep = false;
    if (abs(t1.x - t0.x) < abs(t1.y - t0.y))
    {
        swap(t0.x, t0.y);
        swap(t1.x, t1.y);
        steep = true;
    }

    if (t0.x > t1.x)
    {
        swap(t0.x, t1.x);
        swap(t0.y, t1.y);
    }
    int dx = t1.x - t0.x;
    int dy = t1.y - t0.y;
    int derror2 = abs(dy) * 2;
    int error = 0;
    int y = t0.y;
    for (int x = t0.x; x <= t1.x; ++x)
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
            y += (t1.y > t0.y ? 1 : -1);
            error -= dx * 2;
        }
    }
}

void DrawTriangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)
{
    if (t0.y > t1.y)
    {
        std::swap(t0, t1);
    }
    if (t0.y > t2.y)
    {
        std::swap(t0, t2);
    }
    if (t1.y > t2.y)
    {
        std::swap(t1, t2);
    }
    int total_height = t2.y - t0.y;
    for (int y = t0.y; y <= t1.y; y++)
    {
        int segment_height = t1.y - t0.y + 1;
        float alpha = (float)(y - t0.y) / total_height;
        float beta = (float)(y - t0.y) / segment_height; // be careful with divisions by zero 
        Vec2i A = t0 + (t2 - t0) * alpha;
        Vec2i B = t0 + (t1 - t0) * beta;
        image.set(A.x, y, red);
        image.set(B.x, y, green);
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

    Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

    DrawTriangle(t0[0], t0[1], t0[2], image, red);
    DrawTriangle(t1[0], t1[1], t1[2], image, red);
    DrawTriangle(t2[0], t2[1], t2[2], image, red);

    // image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}
