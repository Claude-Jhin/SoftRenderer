#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

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
    float derror = abs(dy / float(dx));
    float error = 0;
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
        error += derror;
        if (error > 0.5f)
        {
            y += (y1 > y0 ? 1 : -1);
            error -= 1.;
        }
    }
}

int main(int argc, char* argv[])
{
    TGAImage image(100, 100, TGAImage::RGB);
    DrawLine(13, 20, 80, 40, image, white);
    DrawLine(20, 13, 40, 80, image, red);
    DrawLine(80, 40, 13, 20, image, red);
    // image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}
