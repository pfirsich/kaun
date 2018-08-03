#pragma once

#include <expected.hpp>
#include <glm/glm.hpp>

namespace kaun {
    struct ErrorTuple {
        int errorCode;
        std::string errorMessage;

        ErrorTuple(int code) : errorCode(code) {
            char buf[256];
            strerror_s(buf, sizeof(buf), code);
            errorMessage = buf;
        }

        ErrorTuple(int code, const std::string& message) :
                errorCode(code), errorMessage(message) {
        }
    };

    // http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
    tl::expected<std::string, ErrorTuple> readFile(const std::string& filename);

    // for usual model/view matrices this is fine
    inline glm::vec3 transformPoint(const glm::mat4& transform, const glm::vec3& coord) {
        return glm::vec3(transform * glm::vec4(coord, 1.0f));
    }

    // If we are dealing with general, potentially non-affine transformations, use this (e.g. projections)
    // specifically: the last row of the transformation matrix has to be (0, 0, 0, 1), otherwise, use this
    inline glm::vec3 transformPointDivide(const glm::mat4& transform, const glm::vec3& coord) {
        glm::vec4 v = transform * glm::vec4(coord, 1.0f);
        if(v.w < 1e-5) v.w = 1.0f;
        return glm::vec3(v) / v.w;
    }

    // for vectors the last row can be (0, 0, 0, X)
    inline glm::vec3 transformVector(const glm::mat4& transform, const glm::vec3& coord, float w = 1.0f) {
        return glm::vec3(transform * glm::vec4(coord, 0.0f));
    }

    inline glm::vec3 transformVectorDivide(const glm::mat4& transform, const glm::vec3& coord) {
        glm::vec4 v = transform * glm::vec4(coord, 0.0f);
        if(v.w < 1e-5) v.w = 1.0f;
        return glm::vec3(v) / v.w;
    }

    // glm gives an error for glm::abs(mat4());
    inline glm::mat4 absMat4(const glm::mat4& mat) {
        glm::mat4 ret = mat;
        for(int i = 0; i < 3; ++i) {
            for(int j = 0; j < 3; ++j) {
                ret[i][j] = std::fabs(mat[i][j]);
            }
        }
        return ret;
    }
}
