#pragma once

#include <utility>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderstate.hpp"
#include "renderattachment.hpp"
#include "log.hpp"

namespace kaun {
    class Texture : public RenderAttachment {
    public:
        enum class WrapMode : GLenum {
            CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
            CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
            MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
            REPEAT = GL_REPEAT
        };

        // Excellent tutorial on texture filtering: https://paroj.github.io/gltut/Texturing/Tutorial%2015.html
        enum class MinFilter : GLenum {
            // I added the common names for these techniques, though they are not very good names
            NEAREST = GL_NEAREST,
            LINEAR = GL_LINEAR, // "Bilinear"
            NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
            LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST, // "Bilinear"
            NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR, // "Trilinear", but not "Bilinear" - you probably don't want to use this
            LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR // "Trilinear"
        };

        enum class MagFilter : GLenum {
            NEAREST = GL_NEAREST,
            LINEAR = GL_LINEAR
        };

        enum class Target : GLenum {
            NONE = 0,
            TEX_1D = GL_TEXTURE_1D,
            TEX_2D = GL_TEXTURE_2D,
            TEX_3D = GL_TEXTURE_3D,
            TEX_1D_ARRAY = GL_TEXTURE_1D_ARRAY,
            TEX_2D_ARRAY = GL_TEXTURE_2D_ARRAY,
            TEX_RECTANGLE = GL_TEXTURE_RECTANGLE,
            TEX_CUBE_MAP = GL_TEXTURE_CUBE_MAP,
            TEX_BUFFER = GL_TEXTURE_BUFFER,
            TEX_2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE,
            TEX_2D_MULTISAMPLE_ARRAY = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
            TEX_CUBE_MAP_POS_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            TEX_CUBE_MAP_NEG_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            TEX_CUBE_MAP_POS_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            TEX_CUBE_MAP_NEG_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            TEX_CUBE_MAP_POS_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            TEX_CUBE_MAP_NEG_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
        };

    private:
        Target mTarget;
        GLuint mTextureObject;
        PixelFormat mPixelFormat;
        int mWidth, mHeight;
        size_t mSamples;
        bool mImmutable;
        WrapMode mSWrap, mTWrap;
        MinFilter mMinFilter;
        MagFilter mMagFilter;

        void setParameter(GLenum param, GLenum val) {
            if(mTextureObject != 0) {
                bind(0);
                glTexParameteri(static_cast<GLenum>(mTarget), param, val);
            }
        }

        void initSampler();

    public:
        static const size_t MAX_UNITS = 16;
        static const Texture* currentBoundTextures[MAX_UNITS];

        static void ensureGlState();

        // Mipmapping is default, since it's takes a little more ram, but usually it's faster and looks nicer
        Texture(Target target = Target::TEX_2D)
                : mTarget(target), mTextureObject(0), mPixelFormat(PixelFormat::NONE),
                mWidth(-1), mHeight(-1), mSamples(0), mImmutable(false),
                mSWrap(WrapMode::CLAMP_TO_EDGE), mTWrap(WrapMode::CLAMP_TO_EDGE),
                mMinFilter(MinFilter::LINEAR), mMagFilter(MagFilter::LINEAR) {}

        Texture(PixelFormat internalFormat, int width, int height, size_t samples, bool fixedSampleLocations = false)
                : Texture(Target::TEX_2D_MULTISAMPLE) {
            setStorageMultisample(internalFormat, width, height, samples, fixedSampleLocations);
        }

        // Allocate (immutable!) texture storage with glTexStorage2D
        Texture(PixelFormat internalFormat, int width, int height, Target target = Target::TEX_2D) : Texture(target) {
            setStorage(internalFormat, width, height);
        }

        Texture(const std::string& filename, bool genMipmaps = true, Target target = Target::TEX_2D) : Texture(target) {
            loadFromFile(filename, genMipmaps);
        }

        ~Texture() {
            glDeleteTextures(1, &mTextureObject);
        }

        void loadFromMemory(const uint8_t* buffer, int width, int height, int components, bool genMipmaps = true, Target target = Target::NONE, bool replace = false);
        bool loadEncodedFromMemory(const uint8_t* encBuffer, int len, bool genMipmaps = true, Target target = Target::NONE);
        bool loadFromFile(const std::string& filename, bool genMipmaps = true, Target target = Target::NONE);

        void setStorage(PixelFormat format, int width, int height, int levels = 1);
        void setStorageMultisample(PixelFormat format, int width, int height, size_t samples, bool fixedSampleLocations = false);

        void updateData(GLenum format, GLenum type, const void* data, int level = 0, int width = -1, int height = -1, int x = 0, int y = 0);
        // if you've set the base level + data, call this. this can also be called on an immutable texture
        void updateMipmaps() {bind(0); glGenerateMipmap(static_cast<GLenum>(mTarget));}

        void setTarget(Target target) { mTarget = target; }
        Target getTarget() const { return mTarget; }
        GLuint getTextureObject() const { return mTextureObject; }
        PixelFormat getPixelFormat() const { return mPixelFormat; }
        int getWidth() const { return mWidth; }
        int getHeight() const { return mHeight; }
        size_t getSamples() const { return mSamples; }
        bool isValid() const { return mTextureObject != 0; }

        void attach(GLenum attachmentPoint) const;

        void setWrap(WrapMode u, WrapMode v) {
            setParameter(GL_TEXTURE_WRAP_S, static_cast<GLenum>(mSWrap = u));
            setParameter(GL_TEXTURE_WRAP_T, static_cast<GLenum>(mTWrap = v));
        }
        std::pair<WrapMode, WrapMode> getWrap() const { return std::make_pair(mSWrap, mTWrap); }

        void setMinFilter(MinFilter filter) { setParameter(GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(mMinFilter = filter)); }
        MinFilter getMinFilter() const { return mMinFilter; }
        void setMagFilter(MagFilter filter) { setParameter(GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(mMagFilter = filter)); }
        MagFilter getMagFilter() const { return mMagFilter; }

        void setCompareFunc(RenderState::DepthFunc func = RenderState::DepthFunc::LESS) {
            setParameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            setParameter(GL_TEXTURE_COMPARE_FUNC, static_cast<GLenum>(func));
        }

        void setBorderColor(const glm::vec4& col) {
            bind(0);
            glTexParameterfv(static_cast<GLenum>(mTarget),
                             GL_TEXTURE_BORDER_COLOR, glm::value_ptr(col));
        }

        void bind(unsigned int unit) const {
            if(currentBoundTextures[unit] != this && mTextureObject != 0) {
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(static_cast<GLenum>(mTarget), mTextureObject);
                currentBoundTextures[unit] = this;
            }
        }

        int getUnit() const {
            for(int unit = 0; unit < MAX_UNITS; ++unit) {
                if(currentBoundTextures[unit] == this) return unit;
            }
            return -1;
        }

        static void unbind(unsigned int unit) {
            if(currentBoundTextures[unit] != nullptr) {
                glActiveTexture(GL_TEXTURE0 + unit);
                // There can only be one texture bound to a single unit (independent of type), so i can unbind with whatever I want
                // Bonus: unbind can be static!
                glBindTexture(GL_TEXTURE_2D, 0);
                currentBoundTextures[unit] = nullptr;
            }
        }

        static void bindTextures(const std::vector<const Texture*>& textures);

        static Texture* pixel(const glm::vec4& col); // col in SRGB
        static Texture* checkerBoard(int width, int height, int checkerSize,
                                     const glm::vec4& colA = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                                     const glm::vec4& colB = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));
        static Texture* cubeMap(const std::string& posX, const std::string& negX,
                                const std::string& posY, const std::string& negY,
                                const std::string& posZ, const std::string& negZ);
    };
}
