#include "transform.hpp"

namespace kaun {
    glm::mat3 Transform::getLocalSystem() const {
        glm::mat3 ret;
        ret[0] = getRight();
        ret[1] = getUp();
        ret[2] = getForward();
        return ret;
    }

    void Transform::setMatrix(const glm::mat4& matrix, bool updateTRS) {
        mMatrix = matrix;
        mMatrixDirty = false;
        if(updateTRS) updateTRSFromMatrix();
    }

    void Transform::updateTRSFromMatrix() {
        mPosition = glm::vec3(mMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        mQuaternion = glm::normalize(glm::quat_cast(mMatrix));
        // remove the translation and put the scale factors in the columns
        glm::mat3 temp = glm::transpose(glm::mat3(mMatrix));
        mScale = glm::vec3(glm::length(temp[0]), glm::length(temp[1]), glm::length(temp[2]));
    }

    void Transform::lookAtPos(const glm::vec3& pos, const glm::vec3& at, const glm::vec3& up) {
        mPosition = pos;
        mQuaternion = glm::normalize(glm::quat_cast(glm::lookAt(pos, at, up)));
        dirty();
    }
}
