#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace kaun {
    class Transform {
    private:
        mutable bool mMatrixDirty;

    protected:
        glm::vec3 mPosition, mScale;
        glm::quat mQuaternion;
        mutable glm::mat4 mMatrix;

        inline virtual void dirty() { mMatrixDirty = true; }

        void updateTRSFromMatrix();

    public:
        Transform() : mPosition(0.0f, 0.0f, 0.0f), mScale(1.0f, 1.0f, 1.0f),
            mQuaternion(1.0f, 0.0f, 0.0f, 0.0f), mMatrix(1.0f), mMatrixDirty(true) { }
        Transform(const glm::vec3& pos);
        Transform(const glm::vec3& pos, const glm::quat& quat);
        Transform(const glm::vec3& pos, const glm::quat& quat, const glm::vec3& scale);
        Transform(const glm::vec3& pos, const glm::vec3& angles);
        Transform(const glm::vec3& pos, const glm::vec3& angles, const glm::vec3& scale);

        void setPosition(const glm::vec3& position) {
            mPosition = position;
            dirty();
        }
        glm::vec3 getPosition() const { return mPosition; }

        void setScale(const glm::vec3& scale) {
            mScale = scale;
            dirty();
        }
        glm::vec3 getScale() const { return mScale; }

        void setQuaternion(const glm::quat& quat) {
            mQuaternion = quat;
            dirty();
        }
        glm::quat getQuaternion() const { return mQuaternion; }

        void rotate(const glm::quat& quat) {
            mQuaternion = glm::normalize(quat * mQuaternion);
            dirty();
        }

        void rotate(float angleRadians, const glm::vec3& axis) {
            // this normalization is not necessary mathematically, but technically (because of floating point numbers) it is
            mQuaternion = glm::normalize(glm::angleAxis(angleRadians, axis) * mQuaternion);
            dirty();
        }

        void rotateWorld(float angleRadians, const glm::vec3& worldAxis) {
            rotate(angleRadians, mQuaternion * worldAxis);
        }

        // For FPS camera ust localDirToWorld, then set y = 0 and renormalize
        glm::vec3 localDirToWorld(const glm::vec3& vec) const {
            return glm::conjugate(mQuaternion) * vec;
        }

        glm::vec3 getForward() const {
            return localDirToWorld(glm::vec3(0.0f, 0.0f, -1.0f));
        }

        glm::vec3 getRight() const {
            return localDirToWorld(glm::vec3(1.0f, 0.0f, 0.0f));
        }

        glm::vec3 getUp() const {
            return localDirToWorld(glm::vec3(0.0f, 1.0f, 0.0f));
        }

        glm::mat3 getLocalSystem() const;

        // a little long, but still in the header, because I hope this gets inlined
        glm::mat4 getMatrix() const {
            if(mMatrixDirty) {
                mMatrix = glm::scale(glm::translate(glm::mat4(1.0f), mPosition)
                    * glm::mat4_cast(glm::conjugate(mQuaternion)), mScale);
                mMatrixDirty = false;
            }
            return mMatrix;
        }

        void setMatrix(const glm::mat4& matrix, bool updateTRS = true);

        // this works just as gluLookAt, so may put in "world space up" (this might not work sometimes, but makes everything a lot easier most of the time)
        // it also means, that the negative z-axis is aligned to face "at"
        void lookAtPos(const glm::vec3& pos, const glm::vec3& at, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

        void lookAt(const glm::vec3& at, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f)) {
            lookAtPos(mPosition, at, up);
        }

        Transform& operator=(const Transform& other) {
            mPosition = other.mPosition;
            mScale = other.mScale;
            mQuaternion = other.mQuaternion;
            mMatrixDirty = true;
            return *this;
        }
    };
}
