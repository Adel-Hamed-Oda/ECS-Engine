#pragma once

#include "core/private_imports.h"

class Renderer;

class Font {
    friend class Renderer;

public:
    struct Character {
        unsigned int TextureID = 0; // ID handle of the glyph texture
        glm::ivec2   Size{ 0 };     // Size of glyph
        glm::ivec2   Bearing{ 0 };  // Offset from baseline to left/top of glyph
        long         Advance = 0;   // Offset to advance to next glyph
    };

    explicit Font(const std::string& fontPath = "fonts/arial.ttf", float pixelSize = 48.0f)
        : fontPath(fontPath), pixelSize(pixelSize)
    {
        Load();
    }

    static std::shared_ptr<Font> Create(const std::string& fontPath = "fonts/arial.ttf", float pixelSize = 48.0f)
    {
        return std::make_shared<Font>(fontPath, pixelSize);
    }

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    Font(Font&& other) noexcept
        : fontPath(std::move(other.fontPath)),
          pixelSize(other.pixelSize),
          loaded(other.loaded),
          characters(std::move(other.characters))
    {
        other.loaded = false;
        other.characters.clear();
    }

    Font& operator=(Font&& other) noexcept
    {
        if (this != &other)
        {
            Cleanup();

            fontPath = std::move(other.fontPath);
            pixelSize = other.pixelSize;
            loaded = other.loaded;
            characters = std::move(other.characters);

            other.loaded = false;
            other.characters.clear();
        }
        return *this;
    }

    ~Font() { Cleanup(); }

    bool IsValid() const { return loaded && !characters.empty(); }
    const std::string& GetFontPath() const { return fontPath; }
    float GetPixelSize() const { return pixelSize; }

    const Character* GetCharacter(char c) const
    {
        auto it = characters.find(c);
        return (it != characters.end()) ? &it->second : nullptr;
    }

    const std::map<char, Character>& GetCharacters() const { return characters; }

private:
    inline static FT_Library ft = nullptr;

    std::string fontPath;
    float pixelSize = 48.0f;
    bool loaded = false;
    std::map<char, Character> characters;

    void Cleanup()
    {
        for (const auto& [character, ch] : characters)
        {
            if (ch.TextureID != 0)
                glDeleteTextures(1, &ch.TextureID);
        }
        characters.clear();
        loaded = false;
    }

    void Load()
    {
        if (fontPath.empty())
        {
            std::cerr << "ERROR::FREETYPE: Font path is empty." << std::endl;
            return;
        }

        if (!ft && FT_Init_FreeType(&ft) != 0)
        {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return;
        }

        FT_Face face = nullptr;
        if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load font: " << fontPath << std::endl;
            return;
        }

        const FT_UInt resolvedSize = (pixelSize > 0.0f) ? static_cast<FT_UInt>(pixelSize) : 48u;
        FT_Set_Pixel_Sizes(face, 0, resolvedSize);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

        for (unsigned char c = 0; c < 128; c++)
        {
            // load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYPE: Failed to load Glyph: " << c << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture = 0;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                face->glyph->advance.x
            };
            characters.emplace(static_cast<char>(c), character);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        FT_Done_Face(face);
        loaded = true;
    }
};
