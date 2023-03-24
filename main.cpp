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

Vec3f barycentric(Vec2i* pts, Vec2i p)
{
    return Vec3f();
}

bool IsInTriangle(Vec2i* pts, Vec2i p)
{
    // Use cross product
    Vec2i A = {pts[0].x, pts[0].y};
    Vec2i B = {pts[1].x, pts[1].y};
    Vec2i C = {pts[2].x, pts[2].y};

    auto getVector = [](Vec2i from, Vec2i to)-> Vec2i
    {
        return {to.x - from.x, to.y - from.y};
    };

    auto AB = getVector(A, B); 
    auto BC = getVector(B, C);
    auto CA = getVector(C,A);

    auto AP = getVector(A,p);
    auto BP = getVector(B,p);
    auto CP = getVector(C,p);

    auto d1 = AB ^ AP;
    auto d2 = BC ^ BP;
    auto d3 = CA ^ CP;

    return (d1 >= 0 && d2 >= 0 && d3 >= 0)
        || (d1 <= 0 && d2 <= 0 && d3 <= 0);
}

void DrawTriangle(Vec2i* pts, TGAImage& image, TGAColor color)
{
    Vec2i aabbboxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2i aabbboxmax(0, 0);

    Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; ++i)
    {
        aabbboxmin.x = std::max(0, std::min(pts[i].x, aabbboxmin.x));
        aabbboxmin.y = std::max(0, std::min(pts[i].y, aabbboxmin.y));
        aabbboxmax.x = std::min(clamp.x, std::max(pts[i].x, aabbboxmax.x));
        aabbboxmax.y = std::min(clamp.y, std::max(pts[i].y, aabbboxmax.y));
    }

    Vec2i p;
    for (p.x = aabbboxmin.x; p.x < aabbboxmax.x; ++p.x)
    {
        for (p.y = aabbboxmin.y; p.y < aabbboxmax.y; ++p.y)
        {
            if (IsInTriangle(pts, p))
            {
                image.set(p.x, p.y, color);
            }
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

    Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

    DrawTriangle(t0, image, red);
    DrawTriangle(t1, image, white);
    DrawTriangle(t2, image, green);

    // image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}
