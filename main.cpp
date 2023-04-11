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
Vec3f camera(0, 0, 3);

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

void DrawTriangleWithZBufferAndTexture(Vec3i* pts, Vec2f* uv, int* zbuffer_, TGAImage& image,
                                       float intensity)
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
            if (zbuffer_[p.x + p.y * image.get_width()] > p.z)
            {
                continue;
            }

            zbuffer_[p.x + p.y * image.get_width()] = p.z;
            Vec2f uvp = uv[0] * bc_screen.x + uv[1] * bc_screen.y + uv[2] * bc_screen.z;
            TGAColor color_final = model->diffuse(uvp);
            image.set(p.x, p.y, TGAColor(color_final.r * intensity, color_final.g * intensity,
                                         color_final.b * intensity, 255));
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

Matrix viewport(int x, int y, int w, int h)
{
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = depth / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = depth / 2.f;
    return m;
}

Vec3f m2v(Matrix m)
{
    return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix v2m(Vec3f v)
{
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
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

    zbuffer = new int [width * height];
    for (int i = 0; i < width * height; ++i)
    {
        zbuffer[i] = std::numeric_limits<int>::min();
    }

    TGAImage output(width, height, TGAImage::RGB);
    for (int i = 0; i < model->nfaces(); ++i)
    {
        std::vector<Vec3i> face = model->face(i);
        Vec3i screen_coords[3];
        Vec3f world_coords[3];
        Vec2f uv[3];

        Matrix Projection = Matrix::identity(4);
        Projection[3][2] = -1.f / camera.z;
        Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

        for (int j = 0; j < 3; ++j)
        {
            Vec3f vertice = model->vert(face[j].ivert);
            screen_coords[j] = m2v(ViewPort * Projection * v2m(vertice));
            world_coords[j] = vertice;
            uv[j] = model->uv(face[j].iuv);
        }

        Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0)
        {
            // DrawTriangle(screen_coords, output, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
            DrawTriangleWithZBufferAndTexture(screen_coords, uv, zbuffer, output, intensity);
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
