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
    this->fontPath = nullptr;
    this->fontSize = 0;
    this->text = nullptr;
    this->offsetX = 0;
    this->offsetX = 0;
    textBitmap = nullptr;

    if (FT_Init_FreeType(&library))
    {
        std::cerr << "Unable to initialize FreeType library" << std::endl;
        return;
    }
}

Text::~Text()
{
    if (this->fontPath)
        delete[] this->fontPath;
    if (this->text)
        delete[] this->text;
    if (this->textBitmap)
    {
        delete[] this->textBitmap->mem;
        delete this->textBitmap;
    }

    for (auto &pair : this->cachedBitmaps)
    {
        delete[] pair.second.mem;
    }
    this->cachedBitmaps.clear();

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

void Text::SetPosition(int offsetX, int offsetY)
{
    this->offsetX = offsetX;
    this->offsetY = offsetY;
}

void Text::renderBitmap()
{
    std::vector<Text::TextData> textArray;
    int maxAscender = 0;
    int maxDescender = 0;
    signed long penX = 0;
    uint8_t *buffer;
    for (size_t i = 0; i < strlen(this->text); i++)
    {
        char t = this->text[i];
        if (cachedBitmaps.find(t) != cachedBitmaps.end())
        {
            const Text::Bitmap &cachedBitmap = cachedBitmaps[t];
            penX += cachedBitmap.left;
            textArray.push_back({cachedBitmap, penX});
            maxAscender = std::max(maxAscender, cachedBitmap.top);
            maxDescender = std::max(maxDescender, std::abs(static_cast<int>(cachedBitmap.rows - cachedBitmap.top)));
            penX += (cachedBitmap.advanceX >> 6);
            continue;
        }

        if (FT_Load_Char(face, t, FT_LOAD_RENDER))
            std::cout << "load char faild" << std::endl;

        maxAscender = std::max(maxAscender, face->glyph->bitmap_top);
        maxDescender = std::max(maxDescender, std::abs(static_cast<int>(face->glyph->bitmap.rows - face->glyph->bitmap_top)));

        int bufferSize = face->glyph->bitmap.pitch * face->glyph->bitmap.rows;
        buffer = new uint8_t[bufferSize];
        memcpy(buffer, face->glyph->bitmap.buffer, bufferSize);

        penX += face->glyph->bitmap_left;
        textArray.push_back({{face->glyph->bitmap.width,
                              face->glyph->bitmap.rows,
                              face->glyph->bitmap.pitch,
                              face->glyph->bitmap_left,
                              face->glyph->bitmap_top,
                              face->glyph->advance.x,
                              buffer},
                             penX});
        penX += (face->glyph->advance.x >> 6);

        cachedBitmaps.insert({t, textArray.back().bitmap});
    }

    size_t maxHeight = maxAscender + maxDescender;
    // Allocate memory for the bitmap, assume 1 byte per pixel (width = pitch)
    uint8_t *bitmap = new uint8_t[penX * maxHeight];
    std::memset(bitmap, 0, penX * maxHeight);

    for (size_t i = 0; i < maxHeight; i++)
    {
        for (size_t j = 0; j < strlen(this->text); j++)
        {
            const Text::TextData &td = textArray[j];
            // maxAscender is our baseline, and "firstRow" means
            // which row in the combined bitmap is the start of the current character
            size_t firstRow = maxAscender - td.bitmap.top;
            size_t lastRow = firstRow + td.bitmap.rows;
            if (firstRow > i || i >= lastRow)
            {
                continue;
            }
            memcpy(bitmap + i * penX + td.posX, td.bitmap.mem + td.bitmap.pitch * (i - firstRow), td.bitmap.pitch);
        }
    }

    if (this->textBitmap)
    {
        delete[] this->textBitmap->mem;
        delete this->textBitmap;
    }

    this->textBitmap = new Bitmap{static_cast<unsigned int>(penX), maxHeight, penX, 0, maxAscender, penX, bitmap};
}

void Text::Draw2Canvas(uint8_t *YPlane, unsigned int width, unsigned int height)
{
    renderBitmap();

    unsigned int textWidth = this->textBitmap->width;
    unsigned int textHeight = this->textBitmap->rows;
    uint8_t *mem = this->textBitmap->mem;

    uint8_t threshold = 128;

    for (unsigned int y = 0; y < textHeight; ++y)
    {
        for (unsigned int x = 0; x < textWidth; ++x)
        {
            if (y + this->offsetY >= height || x + this->offsetX >= width)
                continue;

            uint8_t pixelValue = mem[y * textWidth + x];

            if (pixelValue > threshold)
            {
                YPlane[(y + this->offsetY) * width + (x + this->offsetX)] = pixelValue;
            }
        }
    }
}
