#pragma once

#include <glad/glad.h>

#include "utility.hpp"

namespace kaun {
    // AA = axis aligned
    struct AABoundingBox {
        glm::vec3 min, max;

        AABoundingBox() : min(0.0f, 0.0f, 0.0f), max(0.0f, 0.0f, 0.0f) {}

        inline void fitPoint(const glm::vec3& point) {
            min = glm::min(min, point);
            max = glm::max(max, point);
        }

        inline void fitAABB(const AABoundingBox& other) {
            if(other.empty()) return;
            if(empty()) {
                min = other.min;
                max = other.max;
            } else {
                min = glm::min(min, other.min);
                max = glm::max(max, other.max);
            }
        }

        //http://zeuxcg.org/2010/10/17/aabb-from-obb-with-component-wise-abs/
        inline void transform(const glm::mat4& mat) {
            glm::vec3 center = (min + max) / 2.0f;
            glm::vec3 extent = (max - min) / 2.0f;

            // choosing the divide version in case these boxes are projected (for frustum culling maybe)
            center = transformPointDivide(mat, center);
            extent = transformVectorDivide(absMat4(mat), extent);

            min = center - extent;
            max = center + extent;
        }

        inline bool pointInside(const glm::vec3& point) const {
            return glm::all(glm::greaterThanEqual(point, min)) && glm::all(glm::lessThanEqual(point, max));
        }

        inline bool overlaps(const AABoundingBox& other) const {
            return glm::all(glm::lessThanEqual(min, other.max)) && glm::all(glm::lessThanEqual(other.min, max));
        }

        inline bool empty() const {
            return glm::length(max - min) < 1e-6;
        }
    };
}
