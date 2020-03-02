#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "render.hpp"
#include "texture.hpp"
#include "utility.hpp"

namespace kaun {
const Texture* Texture::currentBoundTextures[Texture::MAX_UNITS] = { nullptr };

void Texture::ensureGlState()
{
    for (int unit = 0; unit < Texture::MAX_UNITS; ++unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        const Texture* texture = currentBoundTextures[unit];
        if (texture == nullptr) {
            glBindTexture(GL_TEXTURE_2D, 0);
        } else {
            glBindTexture(static_cast<GLenum>(texture->getTarget()), texture->getTextureObject());
        }
    }
}

Texture* Texture::pixel(const glm::vec4& col)
{
    uint32_t data = colToInt(col);
    Texture* temp = new Texture;
    temp->loadFromMemory(reinterpret_cast<unsigned char*>(&data), 1, 1, 4, false);
    return temp;
}

Texture* Texture::checkerBoard(
    int width, int height, int checkerSize, const glm::vec4& colA, const glm::vec4& colB)
{
    uint32_t icolA = colToInt(colA);
    uint32_t icolB = colToInt(colB);
    std::vector<uint32_t> buf(width * height);
    for (int i = 0; i < width * height; ++i) {
        uint32_t col = (i / checkerSize + i / width / checkerSize) % 2 == 0 ? icolA : icolB;
        buf[i] = col;
    }

    Texture* tex = new Texture;
    tex->loadFromMemory(reinterpret_cast<unsigned char*>(&buf[0]), width, height, 4, false);
    // tex->setMagFilter(MagFilter::NEAREST);

    return tex;
}

Texture* Texture::cubeMap(const std::string& posX, const std::string& negX, const std::string& posY,
    const std::string& negY, const std::string& posZ, const std::string& negZ)
{
    Texture* tex = new Texture(Texture::Target::TEX_CUBE_MAP);
    tex->loadFromFile(posX, true, Texture::Target::TEX_CUBE_MAP_POS_X);
    tex->loadFromFile(negX, true, Texture::Target::TEX_CUBE_MAP_NEG_X);
    tex->loadFromFile(posY, true, Texture::Target::TEX_CUBE_MAP_POS_Y);
    tex->loadFromFile(negY, true, Texture::Target::TEX_CUBE_MAP_NEG_Y);
    tex->loadFromFile(posZ, true, Texture::Target::TEX_CUBE_MAP_POS_Z);
    tex->loadFromFile(negZ, true, Texture::Target::TEX_CUBE_MAP_NEG_Z);
    return tex;
}

void Texture::initSampler()
{
    GLenum target = static_cast<GLenum>(mTarget);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, static_cast<GLenum>(mSWrap));
    glTexParameteri(target, GL_TEXTURE_WRAP_T, static_cast<GLenum>(mTWrap));
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(mMagFilter));
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(mMinFilter));
}

const GLint channelsToInternalFormat[4] = { GL_R8, GL_RG8, GL_RGB8, GL_RGBA8 };
const GLint channelsToInternalFormatSrgb[4] = { GL_R8, GL_RG8, GL_SRGB8, GL_SRGB8_ALPHA8 };

void Texture::loadFromMemory(const uint8_t* buffer, int width, int height, int components,
    bool genMipmaps, Target target, bool replace)
{
    assert(components >= 1 && components <= 4);
    if (target == Target::NONE)
        target = mTarget;

    if (mTextureObject == 0)
        glGenTextures(1, &mTextureObject);
    bind(0);

    const GLint* internalFormatMap
        = getSrgbEnabled() ? channelsToInternalFormatSrgb : channelsToInternalFormat;
    GLint internalFormat = internalFormatMap[components - 1];
    GLint formatMap[4] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
    GLint format = formatMap[components - 1];
    if (mImmutable || (replace && width == mWidth && height == mHeight)) {
        if (mImmutable && (width != mWidth || height != mHeight)) {
            LOG_ERROR("Loading texture of size %d, %d that does not fit into an immutable texture "
                      "of size %d, %d",
                width, height, mWidth, mHeight);
            return;
        }
        glTexSubImage2D(
            static_cast<GLenum>(target), 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, buffer);
    } else {
        glTexImage2D(static_cast<GLenum>(target), 0, internalFormat, width, height, 0, format,
            GL_UNSIGNED_BYTE, buffer);
        mPixelFormat = static_cast<PixelFormat>(internalFormat);
        if (genMipmaps)
            mMinFilter = MinFilter::LINEAR_MIPMAP_LINEAR;
        initSampler();
    }

    if (genMipmaps)
        glGenerateMipmap(static_cast<GLenum>(mTarget));

    mWidth = width;
    mHeight = height;
}

bool Texture::loadEncodedFromMemory(
    const uint8_t* encBuffer, int len, bool genMipmaps, Target target)
{
    int w, h, c;
    unsigned char* buf = stbi_load_from_memory(encBuffer, len, &w, &h, &c, 0);
    if (buf == 0) {
        LOG_ERROR("Image could not be loaded from memory.");
        return false;
    }
    loadFromMemory(buf, w, h, c, genMipmaps, target);
    delete[] buf;
    return true;
}

bool Texture::loadFromFile(const std::string& filename, bool genMipmaps, Target target)
{
    int w, h, c;
    unsigned char* buf = stbi_load(filename.c_str(), &w, &h, &c, 0);
    if (buf == 0) {
        LOG_ERROR("Image file '%s' could not be loaded.\n", filename);
        return false;
    }
    loadFromMemory(buf, w, h, c, genMipmaps, target);
    delete[] buf;
    return true;
}

void Texture::attach(GLenum attachmentPoint) const
{
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, attachmentPoint, static_cast<GLenum>(getTarget()), getTextureObject(), 0);
}

void Texture::setStorageMultisample(
    PixelFormat internalFormat, int width, int height, size_t samples, bool fixedSampleLocations)
{
    if (mTextureObject == 0) {
        glGenTextures(1, &mTextureObject);
    } else {
        if (mImmutable) {
            LOG_ERROR("Trying to set storage for an immutable texture!");
            return;
        }
    }
    bind(0);
    mWidth = width;
    mHeight = height;
    mPixelFormat = internalFormat;
    mSamples = samples;

    glTexImage2DMultisample(static_cast<GLenum>(mTarget), samples,
        static_cast<GLenum>(internalFormat), width, height, fixedSampleLocations);
    // initSampler(); -- this just produces a bunch of errors with GL_TEXTURE_2D_MULTISAMPLE
}

void Texture::setStorage(PixelFormat internalFormat, int width, int height, int levels)
{
    if (mTextureObject == 0) {
        glGenTextures(1, &mTextureObject);
    } else {
        if (mImmutable) {
            LOG_ERROR("Trying to set storage for an immutable texture! (you probably called "
                      "setStorage on this texture twice)");
            return;
        }
    }
    bind(0);
    // glTexStorage2D(mTarget, levels, internalFormat, width, height);
    mWidth = width;
    mHeight = height;
    mImmutable = true;
    mPixelFormat = internalFormat;

    /*
    These are all glTexImage2D errors for invalid format combinations:
    GL_INVALID_OPERATION is generated if type is one of GL_UNSIGNED_BYTE_3_3_2,
    GL_UNSIGNED_BYTE_2_3_3_REV, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, or
    GL_UNSIGNED_INT_10F_11F_11F_REV, and format is not GL_RGB. GL_INVALID_OPERATION is generated if
    type is one of GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV,
    GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_INT_8_8_8_8,
    GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, GL_UNSIGNED_INT_2_10_10_10_REV, or
    GL_UNSIGNED_INT_5_9_9_9_REV, and format is neither GL_RGBA nor GL_BGRA. GL_INVALID_OPERATION is
    generated if format is GL_DEPTH_COMPONENT and internalFormat is not GL_DEPTH_COMPONENT,
    GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, or GL_DEPTH_COMPONENT32F. GL_INVALID_OPERATION is
    generated if internalFormat is GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24,
    or GL_DEPTH_COMPONENT32F, and format is not GL_DEPTH_COMPONENT.

    There are way more combinations that don't go well together, but they are not in the docs, so I
    hope they will never make problems. The first two can be eliminated by using GL_FLOAT for type
    (for example), the other two are essentially the same. Ergo I will just just GL_RGBA for format
    and if internalFormat is a depth format, I will use GL_DEPTH_COMPONENT
    */
    GLenum intFormat = static_cast<GLenum>(internalFormat);
    GLenum format = GL_RGBA;
    if (intFormat == GL_DEPTH_COMPONENT || intFormat == GL_DEPTH_COMPONENT16
        || intFormat == GL_DEPTH_COMPONENT24 || intFormat == GL_DEPTH_COMPONENT32F) {
        format = GL_DEPTH_COMPONENT;
    }
    GLenum target = static_cast<GLenum>(mTarget);
    for (int i = 0; i < levels; ++i) {
        glTexImage2D(target, i, intFormat, width, height, 0, format, GL_FLOAT, nullptr);
        width = width / 2;
        if (width < 1)
            width = 1;
        height = height / 2;
        if (height < 1)
            height = 1;
    }
    initSampler();
}

void Texture::updateData(
    GLenum format, GLenum type, const void* data, int level, int width, int height, int x, int y)
{
    if (width < 0)
        width = mWidth;
    if (height < 0)
        height = mHeight;
    if (mTextureObject == 0) {
        LOG_ERROR("Trying to update texture that is not initialized yet!");
        return;
    }
    bind(0);
    glTexSubImage2D(static_cast<GLenum>(mTarget), level, x, y, width, height, format, type, data);
}

void Texture::bindTextures(const std::vector<const Texture*>& textures)
{
    assert(textures.size() <= MAX_UNITS);
    bool unitInUse[MAX_UNITS] = { false };
    std::vector<const Texture*> toBind;
    for (auto tex : textures) {
        int unit = tex->getUnit();
        if (unit >= 0) { // already bound
            unitInUse[unit] = true;
        } else {
            toBind.push_back(tex);
        }
    }
    unsigned int unit;
    for (auto tex : toBind) {
        // get first available unit
        for (unit = 0; unit < MAX_UNITS; ++unit) {
            if (!unitInUse[unit])
                break;
        }
        assert(unit < MAX_UNITS);
        tex->bind(unit);
        unitInUse[unit] = true;
    }
}
}
