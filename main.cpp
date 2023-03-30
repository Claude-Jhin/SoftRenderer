#include "model.h"
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

Model* model = nullptr;
const int width = 800;
const int height = 600;

// define light direction
Vec3f light_dir(0, 0, -1);

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
    auto getVector = [](Vec2i from, Vec2i to) -> Vec2i
    {
        return {to.x - from.x, to.y - from.y};
    };

    Vec2i A = {pts[0].x, pts[0].y};
    Vec2i B = {pts[1].x, pts[1].y};
    Vec2i C = {pts[2].x, pts[2].y};

    auto AB = getVector(A, B);
    auto BC = getVector(B, C);
    auto CA = getVector(C, A);

    auto AP = getVector(A, p);
    auto BP = getVector(B, p);
    auto CP = getVector(C, p);

    // Method 1. Use cross product
    {
        // auto d1 = AB ^ AP;
        // auto d2 = BC ^ BP;
        // auto d3 = CA ^ CP;
        //
        // return (d1 >= 0 && d2 >= 0 && d3 >= 0)
        //     || (d1 <= 0 && d2 <= 0 && d3 <= 0);
    }

    // Method 2. Use area formula
    {
        // auto getArea = [](auto a, auto b, auto c)
        // {
        //     auto p = (a + b + c) / 2;
        //     return std::sqrt(p * (p - a) * (p - b) * (p - c));
        // };
        //
        // auto length_AB = AB.norm();
        // auto length_BC = BC.norm();
        // auto length_CA = CA.norm();
        //
        // auto s = getArea(length_AB, length_BC, length_CA);
        //
        // auto length_AP = AP.norm();
        // auto length_BP = BP.norm();
        // auto length_CP = CP.norm();
        //
        // auto s1 = getArea(length_AB, length_AP, length_BP);
        // auto s2 = getArea(length_BC, length_BP, length_CP);
        // auto s3 = getArea(length_CA, length_CP, length_AP);
        //
        // return !(s < s1 + s2 + s3);
    }

    // Method 3. Use barycentric
    {
        // u * AB + v * AC + 1 * PA = 0
        Vec3f x(AB.x, CA.x * -1, AP.x * -1);
        Vec3f y(AB.y, CA.y * -1, AP.y * -1);

        Vec3f res = x ^ y;

        if (abs(res.z) < 1)
        {
            return false;
        }

        float u = res.x / res.z;
        float v = res.y / res.z;

        if (u < 0 || v < 0 || 1 - u - v < 0)
        {
            return false;
        }

        return true;
    }
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

void rasterize(Vec2i p0, Vec2i p1, TGAImage& tga_image, const TGAColor& color, int* ybuffer, int& ymax)
{
    if (p0.x > p1.x)
    {
        std::swap(p0, p1);
    }

    for (int x = p0.x; x < p1.x; ++x)
    {
        float t = (x - p0.x) / (float)(p1.x - p0.x);
        int y = p0.y * (1. - t) + p1.y * t;
        if (ybuffer[x] < y)
        {
            ybuffer[x] = y;
            for (int i = 0; i < 16; ++i)
            {
                tga_image.set(x, i, color);
            }
            ymax = std::max(ymax, y);
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

    TGAImage scene(width, height, TGAImage::RGB);

    // scene "2d mesh"
    DrawLine(Vec2i(20, 34), Vec2i(744, 400), scene, red);
    DrawLine(Vec2i(120, 434), Vec2i(444, 400), scene, green);
    DrawLine(Vec2i(330, 463), Vec2i(594, 200), scene, blue);

    // screen line
    DrawLine(Vec2i(10, 10), Vec2i(790, 10), scene, white);

    // image.flip_vertically();
    scene.write_tga_file("scene.tga");

    TGAImage render(width, 16, TGAImage::RGB);
    int ybuffer[width];
    int ymax = std::numeric_limits<int>::min();
    for (int i = 0; i < width; ++i)
    {
        ybuffer[i] = std::numeric_limits<int>::min();
    }

    rasterize(Vec2i(20, 34), Vec2i(744, 400), render, red, ybuffer, ymax);
    rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer, ymax);
    rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue, ybuffer, ymax);
    render.write_tga_file("render.tga");

    TGAImage ybufferImage(width, 16, TGAImage::GRAYSCALE);
    for (int i = 0; i < width; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            int greyscale = (float)ybuffer[i] / ymax * 255;
            if (greyscale > 0)
            {
                ybufferImage.set(i, j, TGAColor(greyscale, 1));
            }
        }
    }
    ybufferImage.write_tga_file("ybuffer.tga");

    delete model;
    return 0;
}
