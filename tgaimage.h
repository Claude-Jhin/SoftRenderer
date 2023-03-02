#include <fstream>
using namespace std;

#pragma pack(push,1)
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

    TGAColor(unsigned v, int bpp)
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


    union
    {
        struct
        {
            unsigned char b, g, r, a;
        };

        unsigned char raw[4];
        unsigned int val;
    };

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

    bool write_tga_file(const char* filename);

    bool flip_horizontally();
    
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

    bool load_rle_data(std::ifstream &in);
    bool unload_rle_data(std::ofstream &out);
    
    unsigned char* data;
    int width;
    int height;
    int bytespp;
};
