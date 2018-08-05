#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "texture.hpp"
#include "utility.hpp"

namespace kaun {
    const Texture* Texture::currentBoundTextures[Texture::MAX_UNITS] = {nullptr};
    bool Texture::currentTextureUnitAvailable[Texture::MAX_UNITS] = {true};

    Texture* Texture::pixel(const glm::vec4& col) {
        uint32_t data = colToInt(col);
        Texture* temp = new Texture;
        temp->loadFromMemory(reinterpret_cast<unsigned char*>(&data), 1, 1, 4, false);
        return temp;
    }

    Texture* Texture::checkerBoard(int width, int height, int checkerSize, const glm::vec4& colA, const glm::vec4& colB) {
        uint32_t icolA = colToInt(colA);
        uint32_t icolB = colToInt(colB);
        std::vector<uint32_t> buf(width*height);
        for(int i = 0; i < width*height; ++i) {
            uint32_t col = (i/checkerSize + i/width/checkerSize) % 2 == 0 ? icolA : icolB;
            LOG_DEBUG("col: %X", col);
            buf[i] = col;
        }

        Texture* tex = new Texture;
        tex->loadFromMemory(reinterpret_cast<unsigned char*>(&buf[0]), width, height, 4, false);
        tex->setMagFilter(MagFilter::NEAREST);

        return tex;
    }

    void Texture::initSampler() {
        glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, static_cast<GLenum>(mSWrap));
        glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, static_cast<GLenum>(mTWrap));
        glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(mMagFilter));
        glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(mMinFilter));
    }

    void Texture::loadFromMemory(unsigned char* buffer, int width, int height, int components, bool genMipmaps) {
        assert(components >= 1 && components <= 4);

        if(mTextureObject == 0) glGenTextures(1, &mTextureObject);
        bind(0);

        GLint formatMap[4] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
        GLint format = formatMap[components-1];
        if(mImmutable || (width == mWidth && height == mHeight)) {
            if(mImmutable && (width != mWidth || height != mHeight)) {
                LOG_ERROR("Loading texture of size %d, %d that does not fit into an immutable texture of size %d, %d", width, height, mWidth, mHeight);
                return;
            }
            glTexSubImage2D(mTarget, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, buffer);
        } else {
            glTexImage2D(mTarget, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, buffer);
            if(genMipmaps) mMinFilter = MinFilter::LINEAR_MIPMAP_LINEAR;
            initSampler();
        }

        if(genMipmaps) glGenerateMipmap(mTarget);

        mWidth = width;
        mHeight = height;
    }

    bool Texture::loadEncodedFromMemory(unsigned char* encBuffer, int len, bool genMipmaps) {
        int w, h, c;
        unsigned char* buf = stbi_load_from_memory(encBuffer, len, &w, &h, &c, 0);
        if(buf == 0) {
            LOG_ERROR("Image could not be loaded from memory.");
            return false;
        }
        loadFromMemory(buf, w, h, c, genMipmaps);
        delete[] buf;
        return true;
    }

    bool Texture::loadFromFile(const std::string& filename, bool genMipmaps) {
        int w, h, c;
        unsigned char* buf = stbi_load(filename.c_str(), &w, &h, &c, 0);
        if(buf == 0) {
            LOG_ERROR("Image file '%s' could not be loaded.\n", filename);
            return false;
        }
        loadFromMemory(buf, w, h, c);
        delete[] buf;
        return true;
    }

    void Texture::setStorage(PixelFormat internalFormat, int width, int height, int levels) {
        if(mTextureObject == 0) {
            glGenTextures(1, &mTextureObject);
        } else {
            if(mImmutable) {
                LOG_ERROR("Trying to set storage for an immutable texture! (you probably called setStorage on this texture twice)");
                return;
            }
        }
        bind(0);
        //glTexStorage2D(mTarget, levels, internalFormat, width, height);
        mWidth = width;
        mHeight = height;
        mImmutable = true;

        /*
        These are all glTexImage2D errors for invalid format combinations:
        GL_INVALID_OPERATION is generated if type is one of GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, or GL_UNSIGNED_INT_10F_11F_11F_REV, and format is not GL_RGB.
        GL_INVALID_OPERATION is generated if type is one of GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_INT_8_8_8_8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, GL_UNSIGNED_INT_2_10_10_10_REV, or GL_UNSIGNED_INT_5_9_9_9_REV, and format is neither GL_RGBA nor GL_BGRA.
        GL_INVALID_OPERATION is generated if format is GL_DEPTH_COMPONENT and internalFormat is not GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, or GL_DEPTH_COMPONENT32F.
        GL_INVALID_OPERATION is generated if internalFormat is GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, or GL_DEPTH_COMPONENT32F, and format is not GL_DEPTH_COMPONENT.

        There are way more combinations that don't go well together, but they are not in the docs, so I hope they will never make problems.
        The first two can be eliminated by using GL_FLOAT for type (for example), the other two are essentially the same.
        Ergo I will just just GL_RGBA for format and if internalFormat is a depth format, I will use GL_DEPTH_COMPONENT
        */
        GLenum intFormat = static_cast<GLenum>(internalFormat);
        GLenum format = GL_RGBA;
        if(intFormat == GL_DEPTH_COMPONENT || intFormat == GL_DEPTH_COMPONENT16 ||
           intFormat == GL_DEPTH_COMPONENT24 || intFormat == GL_DEPTH_COMPONENT32F) {
            format = GL_DEPTH_COMPONENT;
        }
        for(int i = 0; i < levels; ++i) {
            glTexImage2D(mTarget, i, intFormat, width, height, 0, format, GL_FLOAT, nullptr);
            width = width / 2;
            if(width < 1) width = 1;
            height = height / 2;
            if(height < 1) height = 1;
        }
        initSampler();
    }

    void Texture::updateData(GLenum format, GLenum type, const void* data, int level, int width, int height, int x, int y) {
        if(width < 0) width = mWidth;
        if(height < 0) height = mHeight;
        if(mTextureObject == 0) {
            LOG_ERROR("Trying to update texture that is not initialized yet!");
            return;
        }
        bind(0);
        glTexSubImage2D(mTarget, level, x, y, width, height, format, type, data);
    }
}
