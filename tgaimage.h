#pragma once

#include <fstream>
using namespace std;

#pragma pack(push,1)
/**
 * \brief The header information in a TGA file includes various parameters that describe the image,
 * such as its dimensions, color depth, and pixel format.
 * This information is needed by image processing software to correctly read and interpret the image data.
 */
struct TGA_Header
{
    char idlength;
    char colormaptype;
    char datatypecode;
    short colormaporigin;
    short colormaplength;
    char colormapdepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char bitsperpixel;
    char imagedescriptor;
};
#pragma pack(pop)

struct TGAColor
{
    TGAColor(): val(0), bytespp(1)
    {
    }

    TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A)
        : b(B),
          g(G),
          r(R),
          a(A),
          bytespp(4)
    {
    }

    TGAColor(unsigned int v, int bpp)
        : val(v),
          bytespp(bpp)
    {
    }

    TGAColor(const TGAColor& other)
        : val(other.val),
          bytespp(other.bytespp)
    {
    }

    TGAColor(const unsigned char* p, int bpp)
        : val(0),
          bytespp(bpp)
    {
        for (int i = 0; i < bpp; ++i)
        {
            raw[i] = p[i];
        }
    }


    TGAColor& operator=(const TGAColor& other)
    {
        if (this == &other)
            return *this;
        val = other.val;
        bytespp = other.bytespp;
        return *this;
    }


    /**
     * These variables share the same memory block,
     * which means when I update the one of them(for example val), the rest two would change the value(struct rgba and raw[4]) 
     */
    union
    {
        struct
        {
            unsigned char b, g, r, a;
        };

        unsigned char raw[4];
        unsigned int val;
    };
    
    /**
     * \brief Bytes assigned per pixel
     */
    int bytespp;
};


class TGAImage
{
public:
    enum Format
    {
        GRAYSCALE = 1,
        RGB = 3,
        RGBA = 4
    };

    TGAImage();

    TGAImage(int w, int h, int bpp);

    TGAImage(const TGAImage& other);

    ~TGAImage();

    TGAImage& operator=(const TGAImage& other);

    bool read_tga_file(const char* filename);

    bool write_tga_file(const char* filename, bool rle = true);

    /**
     * \brief Horizontal Mirror Flip
     * \return If the operation is successful
     */
    bool flip_horizontally();

    /**
     * \brief Vertical Miro Flip
     * \return If the operation is successful
     */
    bool flip_vertically();

    bool scale(int w, int h);

    TGAColor get(int x, int y);

    bool set(int x, int y, TGAColor c);

    int get_width();

    int get_height();

    int get_bytespp();

    unsigned char* buffer();

    void clear();

protected:
    // TODO: Dont understand
    bool load_rle_data(std::ifstream& in);
    bool unload_rle_data(std::ofstream& out);

    unsigned char* data;
    int width;
    int height;
    int bytespp;
};
