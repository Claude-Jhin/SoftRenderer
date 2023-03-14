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
        in.read((char*)data, nbytes);
        if (!in.good())
        {
            in.close();
            cerr << "an error occured while reading the data\n";
            return false;
        }
    }
    else if (10 == header.datatypecode || 11 == header.datatypecode)
    {
        if (!load_rle_data(in))
        {
            in.close();
            cerr << "an error occured while reading the data\n";
            return false;
        }
    }
    else
    {
        in.close();
        cerr << "unknown file format " << (int)header.datatypecode << "\n";
        return false;
    }

    // TODO: Dont understand
    if (!(header.imagedescriptor & 0x20))
    {
        flip_vertically();
    }

    if (!(header.imagedescriptor & 0x10))
    {
        flip_horizontally();
    }
    std::cerr << width << "x" << height << "/" << bytespp * 8 << "\n";
    in.close();
    return true;
}

bool TGAImage::flip_horizontally()
{
    if (!data)
    {
        return false;
    }

    int half = width >> 1;
    for (int i = 0; i < half; ++i)
    {
        for (int j = 0; j < height; ++j)
        {
            TGAColor c1 = get(i, j);
            TGAColor c2 = get(width - 1 - i, j);
            set(i, j, c2);
            set(width - 1 - i, j, c1);
        }
    }
    return true;
}

bool TGAImage::flip_vertically()
{
    if (!data)
    {
        return false;
    }

    unsigned long bytes_per_line = width * bytespp;
    unsigned char* line = new unsigned char[bytes_per_line];
    int half = height >> 1;
    for (int j = 0; j < half; ++j)
    {
        unsigned long l1 = j * bytes_per_line;
        unsigned long l2 = (height - 1 - j) * bytes_per_line;
        memmove((void*)line, (void*)(data + l1), bytes_per_line);
        memmove((void*)(data + l1), (void*)(data + l2), bytes_per_line);
        memmove((void*)(data + l2), (void*)line, bytes_per_line);
    }
    delete[] line;
    return true;
}

bool TGAImage::scale(int w, int h)
{
    if (w <= 0 || h <= 0 || !data)
    {
        return false;
    }

    unsigned char* tdata = new unsigned char[w * h * bytespp];
    int nscanline = 0;
    int oscanline = 0;
    int erry = 0;
    unsigned long nlinebytes = w * bytespp;
    unsigned long olinebytes = width * bytespp;
    for (int j = 0; j < height; ++j)
    {
        int errx = width - w;
        int nx = -bytespp;
        int ox = -bytespp;
        for (int i = 0; i < width; ++i)
        {
            ox += bytespp;
            errx += w;
            while (errx >= (int)width)
            {
                errx -= width;
                nx += bytespp;
                memcpy(tdata + nscanline + nx, data + oscanline + ox, bytespp);
            }
        }
        erry += h;
        oscanline += olinebytes;
        while (erry >= (int)height)
        {
            if (erry >= (int)height << 1)
            {
                memcpy(tdata + nscanline + nlinebytes, tdata + nscanline, nlinebytes);
            }
            erry -= height;
            nscanline += nlinebytes;
        }
    }
    delete[] data;
    data = tdata;
    width = w;
    height = h;
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
