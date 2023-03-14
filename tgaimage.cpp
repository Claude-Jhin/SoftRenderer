#include "tgaimage.h"

#include <iostream>

TGAImage::TGAImage()
    : data(nullptr),
      width(0),
      height(0),
      bytespp(0)
{
}

TGAImage::TGAImage(int w, int h, int bpp)
    : data(nullptr),
      width(w),
      height(h),
      bytespp(bpp)
{
    unsigned long nbytes = width * height * bytespp;
    data = new unsigned char[nbytes];
    memset(data, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage& other)
{
    width = other.width;
    height = other.height;
    bytespp = other.bytespp;
    unsigned long nbytes = width * height * bytespp;
    data = new unsigned char[nbytes];
    memcpy(data, other.data, nbytes);
}

TGAImage::~TGAImage()
{
    if (data)
    {
        delete[] data;
    }
}

TGAImage& TGAImage::operator=(const TGAImage& other)
{
    if (this == &other)
    {
        return *this;
    }

    width = other.width;
    height = other.height;
    bytespp = other.bytespp;
    unsigned long nbytes = width * height * bytespp;
    data = new unsigned char[nbytes];
    memcpy(data, other.data, nbytes);

    return *this;
}

bool TGAImage::read_tga_file(const char* filename)
{
    if (data)
    {
        delete[] data;
        data = nullptr;
    }

    ifstream in;
    in.open(filename, ios::binary);
    if (!in.is_open())
    {
        cerr << "can't open file " << filename << "\n";
        in.close();
        return false;
    }

    TGA_Header header;
    in.read((char*)&header, sizeof(header));
    if (!in.good())
    {
        in.close();
        cerr << "an error occured while reading the header\n";
        return false;
    }

    width = header.width;
    height = header.height;
    // bit to byte
    bytespp = header.bitsperpixel >> 3;
    if (width <= 0 || height <= 0 || (bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA))
    {
        in.close();
        cerr << "bad bpp or width/height value\n";
        return false;
    }

    unsigned long nbytes = bytespp * width * height;
    data = new unsigned char[nbytes];
    if (3 == header.datatypecode || 2 == header.datatypecode)
    {
        in.read((char))
    }

    return true;
}

TGAColor TGAImage::get(int x, int y)
{
    if (!data || x < 0 || y < 0 || x >= width || y >= height)
    {
        return TGAColor();
    }
    return TGAColor(data + (x + y * width) * bytespp, bytespp);
}

bool TGAImage::set(int x, int y, TGAColor c)
{
    if (!data || x < 0 || y < 0 || x >= width || y >= height)
    {
        return false;
    }
    memcpy(data + (x + y * width) * bytespp, c.raw, bytespp);
    return true;
}

int TGAImage::get_width()
{
    return width;
}

int TGAImage::get_height()
{
    return height;
}

int TGAImage::get_bytespp()
{
    return bytespp;
}

unsigned char* TGAImage::buffer()
{
    return data;
}

void TGAImage::clear()
{
    memset((void*)data, 0, width * height * bytespp);
}
