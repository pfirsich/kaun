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
    class Texture {
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

    private:
        GLenum mTarget;
        GLuint mTextureObject;
        PixelFormat mPixelFormat;
        int mWidth, mHeight;
        bool mImmutable;
        WrapMode mSWrap, mTWrap;
        MinFilter mMinFilter;
        MagFilter mMagFilter;

        void setParameter(GLenum param, GLenum val) {
            if(mTextureObject != 0) {
                bind(0);
                glTexParameteri(mTarget, param, val);
            }
        }

        void initSampler();

    public:
        static const size_t MAX_UNITS = 16;
        static const Texture* currentBoundTextures[MAX_UNITS];
        static bool currentTextureUnitAvailable[MAX_UNITS];

        // Mipmapping is default, since it's takes a little more ram, but usually it's faster and looks nicer
        Texture(GLenum target = GL_TEXTURE_2D) : mTarget(target), mTextureObject(0), mPixelFormat(PixelFormat::NONE),
                mWidth(-1), mHeight(-1), mImmutable(false), mSWrap(WrapMode::CLAMP_TO_EDGE),
                mTWrap(WrapMode::CLAMP_TO_EDGE), mMinFilter(MinFilter::LINEAR),
                mMagFilter(MagFilter::LINEAR) {}

        // Allocate (immutable!) texture storage wit glTexStorage2D
        Texture(PixelFormat internalFormat, int width, int height, int levels = 1, GLenum target = GL_TEXTURE_2D) : Texture(target) {
            setStorage(internalFormat, width, height, levels);
        }

        Texture(const std::string& filename, bool genMipmaps = true, GLenum target = GL_TEXTURE_2D) : Texture(target) {
            loadFromFile(filename, genMipmaps);
        }

        ~Texture() {
            glDeleteTextures(1, &mTextureObject);
        }

        void loadFromMemory(const uint8_t* buffer, int width, int height, int components, bool genMipmaps = true);
        bool loadEncodedFromMemory(const uint8_t* encBuffer, int len, bool genMipmaps = true);
        bool loadFromFile(const std::string& filename, bool genMipmaps = true);
        void setStorage(PixelFormat format, int width, int height, int levels = 1);
        void updateData(GLenum format, GLenum type, const void* data, int level = 0, int width = -1, int height = -1, int x = 0, int y = 0);
        // if you've set the base level + data, call this. this can also be called on an immutable texture
        void updateMipmaps() {bind(0); glGenerateMipmap(mTarget);}

        void setTarget(GLenum target) { mTarget = target; }
        GLenum getTarget() const { return mTarget; }
        GLuint getTextureObject() const { return mTextureObject; }
        PixelFormat getPixelFormat() const { return mPixelFormat; }
        int getWidth() const { return mWidth; }
        int getHeight() const { return mHeight; }
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
            glTexParameterfv(mTarget, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(col));
        }

        void bind(unsigned int unit) const {
            if(currentBoundTextures[unit] != this && mTextureObject != 0) {
                glActiveTexture(GL_TEXTURE0 + unit);
                glBindTexture(mTarget, mTextureObject);

                int boundTexture;
                glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
                currentBoundTextures[unit] = this;
                currentTextureUnitAvailable[unit] = false;
            }
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

        // These functions should be uses in conjunction.
        // The parameterless bind binds the texture to ANY unit, or does not bind it if it is already bound and returns the index of the unit
        // it was bound to.
        // If there is no unit available (currentTextureUnitAvailable[unit] == false for all units) then this function will return -1
        // To mark all texture units as unused for the next draw call, use markAllUnitsAvailable().
        // bind(unit) "forcibly" binds the texture to the specified unit, even if it is marked as unavailable
        // So if you have textures with a fixed unit you want to bind them to, then bind them first (because bind(unit) also makes sure to mark units as used)
        // to accidental overwriting and bind the rest however you want
        // This mechanism exists to minimize rebinds on textures if they are on different units and also to have multiple uniform blocks bind textures without talking to each other
        // (obviously by introducing more global state)
        // Example:
        // for each draw call:
        //     markAllUnitsAvailable()
        //     fixedUnitTexture.bind(fixedUnit);
        //     setUniform("fixedUnitTexture", fixedUnit);
        //     setUniform("mySampler", someTexture.bind());
        //     setUniform("mySampler2", someTexture2.bind());
        //     draw()

        int bind() const {
            int firstAvailableUnit = -1;
            for(unsigned int i = 0; i < MAX_UNITS; ++i) {
                if(currentTextureUnitAvailable[i] == true && firstAvailableUnit < 0) firstAvailableUnit = i;
                if(currentBoundTextures[i] == this) return i; // already bound, return index
            }
            if(firstAvailableUnit < 0) {
                LOG_ERROR("No units available for binding!");
            } else {
                bind(firstAvailableUnit);
            }
            return firstAvailableUnit;
        }

        static void markAllUnitsAvailable() {
            for(unsigned int i = 0; i < MAX_UNITS; ++i) currentTextureUnitAvailable[i] = true;
        }


        static Texture* pixel(const glm::vec4& col); // col in SRGB
        static Texture* checkerBoard(int width, int height, int checkerSize,
                                     const glm::vec4& colA = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
                                     const glm::vec4& colB = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));
    };
}
