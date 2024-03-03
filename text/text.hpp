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
    void SetText(const char *text);
    void SetPosition(int x, int y);
    void Draw2Canvas(uint8_t *YPlane, unsigned int width, unsigned int height);
    struct TextData
    {
        unsigned int width;
        unsigned int rows;
        int pitch;
        signed long advanceX;
        signed long advanceY;
        uint8_t *bitmap;
    };

    TextData *bitmap;

protected:
    // Static is used because the variations in characters are usually small, 
    // and utilizing caching can reduce the overhead of rendering.
    static std::map<char, TextData> cachedBitmaps;

private:
    char *fontPath;
    char *text;
    int x;
    int y;

    void renderBitmap();
};