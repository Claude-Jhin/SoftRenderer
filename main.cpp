#include "model.h"
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

Model* model = nullptr;

const int width = 800;
const int height = 600;
const int depth = 255;

int* zbuffer = nullptr;

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

Vec3f barycentric(Vec3i* pts, Vec3i p)
{
    // (1 - u - v) * A + u * B + v * C = P
    // (1 - u - v) * A + u * B + v * C - A = P - A
    // u * (B - A) + v * (C - A) = P - A
    // u * AB + v * AC + PA = 0
    const Vec3i A = pts[0];
    const Vec3i B = pts[1];
    const Vec3i C = pts[2];

    const Vec3i AB = B - A;
    const Vec3i AC = C - A;
    const Vec3i PA = A - p;

    Vec3f x(AB.x, AC.x, PA.x);
    Vec3f y(AB.y, AC.y, PA.y);

    Vec3f res = x ^ y;

    if (abs(res.z) < 1)
    {
        return Vec3f(-1, 1, 1);
    }

    float u = res.x / res.z;
    float v = res.y / res.z;
    float w = 1.f - (res.x + res.y) / res.z;

    return Vec3f(w, u, v);
}

Vec3f barycentric(Vec2i* pts, Vec2i p)
{
    // (1 - u - v) * A + u * B + v * C = P
    // (1 - u - v) * A + u * B + v * C - A = P - A
    // u * (B - A) + v * (C - A) = P - A
    // u * AB + v * AC + PA = 0
    const Vec2i A = pts[0];
    const Vec2i B = pts[1];
    const Vec2i C = pts[2];

    const Vec2i AB = B - A;
    const Vec2i AC = C - A;
    const Vec2i PA = A - p;

    Vec3f x((float)AB.x, (float)AC.x, (float)PA.x);
    Vec3f y((float)AB.y, (float)AC.y, (float)PA.y);

    Vec3f res = x ^ y;

    if (abs(res.z) < 1)
    {
        return Vec3f(-1, 1, 1);
    }

    float u = res.x / res.z;
    float v = res.y / res.z;
    float w = 1.f - (res.x + res.y) / res.z;

    return Vec3f(w, u, v);
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
        auto u = barycentric(pts, p);

        if (u.x < 0 || u.y < 0 || u.z < 0)
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

void DrawTriangleWithZBuffer(Vec3i* pts, int* zbuffer, TGAImage& image, const TGAColor& color)
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

    Vec3i p;
    for (p.x = aabbboxmin.x; p.x < aabbboxmax.x; ++p.x)
    {
        for (p.y = aabbboxmin.y; p.y < aabbboxmax.y; ++p.y)
        {
            auto bc_screen = barycentric(pts, p);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
            {
                continue;
            }

            p.z = 0;
            p.z += pts[0].z * bc_screen.x;
            p.z += pts[1].z * bc_screen.y;
            p.z += pts[2].z * bc_screen.z;
            if (zbuffer[p.x + p.y * image.get_width()] > p.z)
            {
                continue;
            }

            zbuffer[p.x + p.y * image.get_width()] = p.z;
            image.set(p.x, p.y, color);
        }
    }
}

void DrawTriangleWithZBufferAndTexture(Vec3i* pts, Vec2i* uv, int* zbuffer, TGAImage texture_, TGAImage& image,
                                       const TGAColor& color)
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

    Vec3i p;
    for (p.x = aabbboxmin.x; p.x < aabbboxmax.x; ++p.x)
    {
        for (p.y = aabbboxmin.y; p.y < aabbboxmax.y; ++p.y)
        {
            auto bc_screen = barycentric(pts, p);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
            {
                continue;
            }

            p.z = 0;
            p.z += pts[0].z * bc_screen.x;
            p.z += pts[1].z * bc_screen.y;
            p.z += pts[2].z * bc_screen.z;
            if (zbuffer[p.x + p.y * image.get_width()] > p.z)
            {
                continue;
            }

            zbuffer[p.x + p.y * image.get_width()] = p.z;
            TGAColor color = texture_.get(uv[0].x * bc_screen.x + uv[1].x * bc_screen.y + uv[2].x * bc_screen.z,
                                          uv[0].y * bc_screen.x + uv[1].y * bc_screen.y + uv[2].y * bc_screen.z);
            image.set(p.x, p.y, color);
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
    TGAImage texture;
    if (argc == 2)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("obj/african_head.obj");
        texture.read_tga_file("obj/african_head_diffuse.tga");
        texture.flip_vertically();
    }

    zbuffer = new int [width * height];
    for (int i = 0; i < width * height; ++i)
    {
        zbuffer[i] = std::numeric_limits<int>::min();
    }

    TGAImage output(width, height, TGAImage::RGB);
    for (int i = 0; i < model->nfaces(); ++i)
    {
        std::vector<int> face = model->face(i);
        Vec3i screen_coords[3];
        Vec3f world_coords[3];
        Vec2i uv[3];
        for (int j = 0; j < 3; ++j)
        {
            Vec3f vertice = model->vert(face[j]);
            screen_coords[j] = Vec3i((vertice.x + 1.) * width / 2., (vertice.y + 1.) * height / 2.,
                                     (vertice.z + 1.) * depth / 2);
            world_coords[j] = vertice;
            Vec2f uv_f = model->uv(face[j]);
            uv[j] = Vec2i(uv_f.x * texture.get_width(), uv_f.y * texture.get_height());
        }

        Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0)
        {
            // DrawTriangle(screen_coords, output, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
            DrawTriangleWithZBufferAndTexture(screen_coords, uv, zbuffer, texture, output,
                                              TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }

    output.write_tga_file("output.tga");

    TGAImage depth_image(width, height, TGAImage::RGB);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            depth_image.set(x, y, TGAColor(max(zbuffer[x + y * width] * 255 / depth, 0),
                                           max(zbuffer[x + y * width] * 255 / depth, 0),
                                           max(zbuffer[x + y * width] * 255 / depth, 0), 255));
        }
    }

    depth_image.write_tga_file("depth.tga");

    delete model;
    return 0;
}
