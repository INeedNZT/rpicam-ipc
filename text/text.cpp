#include "text.hpp"

#include <ft2build.h>
#include <iostream>
#include <vector>
#include FT_FREETYPE_H

static FT_Library library;

std::map<char, Text::TextData> Text::cachedBitmaps = {};

Text::Text()
{
    fontPath = nullptr;
    text = nullptr;
    x = 0;
    y = 0;
}

Text::~Text()
{
    if (fontPath)
        delete[] fontPath;
    if (text)
        delete[] text;
    if (bitmap)
    {
        delete[] bitmap->bitmap;
        delete bitmap;
    }
}

void Text::SetFontPath(const char *fontPath)
{
    if (this->fontPath)
        delete[] this->fontPath;
    this->fontPath = new char[strlen(fontPath) + 1];
    strcpy(this->fontPath, fontPath);
}

void Text::SetText(const char *text)
{
    if (this->text)
        delete[] this->text;
    this->text = new char[strlen(text) + 1];
    strcpy(this->text, text);
}

void Text::SetPosition(int x, int y)
{
    this->x = x;
    this->y = y;
}

void Text::renderBitmap()
{
    FT_Face face;
    FT_Error error;

    error = FT_Init_FreeType(&library);
    if (error)
    {
        std::cerr << "Unable to initialize FreeType library" << std::endl;
        return;
    }

    error = FT_New_Face(library, this->fontPath, 0, &face);
    if (error)
    {
        std::cerr << "Unable to load font file" << std::endl;
        return;
    }

    error = FT_Set_Pixel_Sizes(face, 0, 16);

    if (error)
    {
        std::cerr << "Unable to set pixel size" << std::endl;
        return;
    }

    std::vector<Text::TextData> textArray;
    unsigned int maxHeight = 0;
    signed long advanceX = 0;
    uint8_t *bufferCopy;
    for (size_t i = 0; i < strlen(this->text); i++)
    {
        char t = this->text[i];
        error = FT_Load_Char(face, t, FT_LOAD_RENDER);
        if (error)
            std::cout << "load char faild" << std::endl;
        maxHeight = std::max(maxHeight, face->glyph->bitmap.rows);
        int bufferSize = face->glyph->bitmap.pitch * face->glyph->bitmap.rows;
        bufferCopy = new uint8_t[bufferSize];
        memcpy(bufferCopy, face->glyph->bitmap.buffer, bufferSize);
        textArray.push_back({face->glyph->bitmap.width,
                             face->glyph->bitmap.rows,
                             face->glyph->bitmap.pitch,
                             advanceX,
                             face->glyph->advance.y,
                             bufferCopy});
        int s = face->glyph->advance.x >> 6;
        advanceX += s;
    }

    // Allocate memory for the bitmap, assume 1 byte per pixel (width = pitch)
    uint8_t *bitmap = new uint8_t[advanceX * maxHeight];
    std::memset(bitmap, 0, advanceX * maxHeight);

    for (size_t i = 0; i < maxHeight; i++)
    {
        for (size_t j = 0; j < strlen(this->text); j++)
        {
            // char t = this->text[j];
            const Text::TextData &cachedBitmap = textArray[j];
            size_t asent = maxHeight - cachedBitmap.rows;
            if (asent > i)
            {
                continue;
            }
            memcpy(bitmap + i * advanceX + cachedBitmap.advanceX, cachedBitmap.bitmap + cachedBitmap.pitch * (i - asent), cachedBitmap.pitch);
        }
    }

    this->bitmap = new TextData{static_cast<unsigned int>(advanceX), maxHeight, static_cast<int>(advanceX), 0, 0, bitmap};

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

void Text::Draw2Canvas(uint8_t *YPlane, unsigned int width, unsigned int height)
{
    renderBitmap();

    unsigned int offsetX = 50;
    unsigned int offsetY = 20;

    unsigned int textWidth = this->bitmap->width;
    unsigned int textHeight = this->bitmap->rows;
    uint8_t *textBitmap = this->bitmap->bitmap;

    uint8_t threshold = 128;

    for (unsigned int y = 0; y < textHeight; ++y) {
        for (unsigned int x = 0; x < textWidth; ++x) {
            if (y + offsetY >= height || x + offsetX >= width) continue;
            
            uint8_t pixelValue = textBitmap[y * textWidth + x];

            if (pixelValue > threshold) {
                YPlane[(y + offsetY) * width + (x + offsetX)] = pixelValue;
            }
        }
    }
}