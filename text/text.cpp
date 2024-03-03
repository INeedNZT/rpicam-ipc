#include "text.hpp"

#include <ft2build.h>
#include <iostream>
#include <vector>
#include FT_FREETYPE_H

static FT_Library library;
static FT_Face face;

std::map<char, Text::Bitmap> Text::cachedBitmaps = {};

Text::Text()
{
    fontPath = nullptr;
    fontSize = 0;
    text = nullptr;
    x = 0;
    y = 0;

    if (FT_Init_FreeType(&library))
    {
        std::cerr << "Unable to initialize FreeType library" << std::endl;
        return;
    }
}

Text::~Text()
{
    if (fontPath)
        delete[] fontPath;
    if (text)
        delete[] text;
    if (textData)
    {
        delete[] textData->bitmap.bitmap;
        delete textData;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

void Text::SetFontPath(const char *fontPath)
{
    if (this->fontPath)
        delete[] this->fontPath;
    this->fontPath = new char[strlen(fontPath) + 1];
    strcpy(this->fontPath, fontPath);
    if (FT_New_Face(library, this->fontPath, 0, &face))
    {
        std::cerr << "Unable to load font file" << std::endl;
        return;
    }
}

void Text::SetFontSize(int fontSize)
{
    this->fontSize = fontSize;

    if (FT_Set_Pixel_Sizes(face, 0, this->fontSize))
    {
        std::cerr << "Unable to set pixel size" << std::endl;
        return;
    }
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
    FT_Error error;

    std::vector<Text::TextData> textArray;
    unsigned int maxHeight = 0;
    signed long currentX = 0;
    uint8_t *bufferCopy;
    for (size_t i = 0; i < strlen(this->text); i++)
    {
        char t = this->text[i];
        if (cachedBitmaps.find(t) != cachedBitmaps.end())
        {
            const Text::Bitmap &cachedBitmap = cachedBitmaps[t];
            textArray.push_back({cachedBitmap, currentX});
            maxHeight = std::max(maxHeight, cachedBitmap.rows);
            currentX += (cachedBitmap.advanceX >> 6);
            continue;
        }

        error = FT_Load_Char(face, t, FT_LOAD_RENDER);
        if (error)
            std::cout << "load char faild" << std::endl;
        maxHeight = std::max(maxHeight, face->glyph->bitmap.rows);
        int bufferSize = face->glyph->bitmap.pitch * face->glyph->bitmap.rows;
        bufferCopy = new uint8_t[bufferSize];
        memcpy(bufferCopy, face->glyph->bitmap.buffer, bufferSize);
        textArray.push_back({{face->glyph->bitmap.width,
                              face->glyph->bitmap.rows,
                              face->glyph->bitmap.pitch,
                              face->glyph->advance.x,
                              bufferCopy},
                             currentX});
        currentX += (face->glyph->advance.x >> 6);

        cachedBitmaps.insert({t, textArray.back().bitmap});
    }

    // Allocate memory for the bitmap, assume 1 byte per pixel (width = pitch)
    uint8_t *bitmap = new uint8_t[currentX * maxHeight];
    std::memset(bitmap, 0, currentX * maxHeight);

    for (size_t i = 0; i < maxHeight; i++)
    {
        for (size_t j = 0; j < strlen(this->text); j++)
        {
            // char t = this->text[j];
            const Text::TextData &td = textArray[j];
            size_t asent = maxHeight - td.bitmap.rows;
            if (asent > i)
            {
                continue;
            }
            memcpy(bitmap + i * currentX + td.posX, td.bitmap.bitmap + td.bitmap.pitch * (i - asent), td.bitmap.pitch);
        }
    }

    this->textData = new TextData{{static_cast<unsigned int>(currentX), maxHeight, static_cast<int>(currentX), 0, bitmap}, static_cast<int>(currentX)};
}

void Text::Draw2Canvas(uint8_t *YPlane, unsigned int width, unsigned int height)
{
    renderBitmap();

    unsigned int offsetX = 50;
    unsigned int offsetY = 20;

    unsigned int textWidth = this->textData->bitmap.width;
    unsigned int textHeight = this->textData->bitmap.rows;
    uint8_t *textBitmap = this->textData->bitmap.bitmap;

    uint8_t threshold = 128;

    for (unsigned int y = 0; y < textHeight; ++y)
    {
        for (unsigned int x = 0; x < textWidth; ++x)
        {
            if (y + offsetY >= height || x + offsetX >= width)
                continue;

            uint8_t pixelValue = textBitmap[y * textWidth + x];

            if (pixelValue > threshold)
            {
                YPlane[(y + offsetY) * width + (x + offsetX)] = pixelValue;
            }
        }
    }
}