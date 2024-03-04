#pragma once

#include <cstring>
#include <cstdint>
#include <map>

class Text
{
public:
    Text();
    ~Text();
    void SetFontPath(const char *fontPath);
    void SetFontSize(int fontSize);
    void SetText(const char *text);
    void SetPosition(int offsetX, int offsetY);
    void Draw2Canvas(uint8_t *YPlane, unsigned int width, unsigned int height);

protected:
    struct Bitmap
    {
        unsigned int width;
        unsigned int rows;
        int pitch;
        signed int left;
        signed int top;
        signed long advanceX;
        uint8_t *mem;
    };
    struct TextData
    {
        Bitmap bitmap;
        signed long posX;
    };
    // "static" is used because the variations in characters are usually small,
    // and utilizing caching can reduce the overhead of rendering.
    static std::map<char, Bitmap> cachedBitmaps;

private:
    char *fontPath;
    int fontSize;
    char *text;
    int offsetX;
    int offsetY;
    
    Bitmap *textBitmap;

    void renderBitmap();
};