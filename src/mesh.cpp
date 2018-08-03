#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh.hpp"

namespace kaun {
    GLuint Mesh::lastBoundVAO = 0;

    VertexBuffer* Mesh::hasAttribute(AttributeType attrType) const {
        for(auto& vBuffer : mVertexBuffers) {
            if(vBuffer->getVertexFormat().hasAttribute(attrType)) return vBuffer.get();
        }
        return nullptr;
    }

    void Mesh::compile() {
        if(mVAO == 0) glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);

        // Not sure if this should be in VertexFormat
        for(auto& vData : mVertexBuffers) {
            vData->bind();
            const VertexFormat& format = vData->getVertexFormat();
            const auto& attributes = format.getAttributes();
            for(size_t i = 0; i < attributes.size(); ++i) {
                const auto& attr = attributes[i];
                int location = static_cast<int>(attr.type);
                glEnableVertexAttribArray(location);
                glVertexAttribPointer(location, attr.alignedNum, static_cast<GLenum>(attr.dataType),
                                      attr.normalized ? GL_TRUE : GL_FALSE,
                                      format.getStride(), reinterpret_cast<GLvoid*>(attr.offset));
                if(attr.divisor > 0) glVertexAttribDivisor(location, attr.divisor);
            }
        }

        // for ARRAY_BUFFER only the calls to glEnableVertexAttribArray/glEnableVertexPointer are stored
        // so unbind now.
        mVertexBuffers.back()->unbind();

        if(mIndexBuffer != nullptr) mIndexBuffer->bind();

        glBindVertexArray(0);

        // VAO stores the last bound ELEMENT_BUFFER state, so as soon as the VAO is unbound, unbind the VBO
        if(mIndexBuffer != nullptr) mIndexBuffer->unbind();
    }

    void Mesh::draw(size_t instanceCount) {
        if(mVAO == 0) {
            compile();
        }

        if(lastBoundVAO != mVAO) glBindVertexArray(mVAO);

        // A lof of this can go wrong if someone compiles this Mesh without an index buffer attached, then attaches one and compiles it with another
        // shader, while both are in use
        GLenum mode = static_cast<GLenum>(mMode);
        if(mIndexBuffer != nullptr) {
            GLenum indexType = static_cast<GLenum>(mIndexBuffer->getDataType());
            if(instanceCount > 0) {
                glDrawElementsInstanced(mode, mIndexBuffer->getNumIndices(), indexType, nullptr, instanceCount);
            } else {
                glDrawElements(mode, mIndexBuffer->getNumIndices(), indexType, nullptr);
            }
        } else {
            // If someone had the great idea of having multiple VertexBuffer objects attached and changing their size after attaching
            // this might break
            size_t size = 0;
            if(mVertexBuffers.size() > 0) size = mVertexBuffers[0]->getNumVertices();
            if(instanceCount > 0) {
                glDrawArraysInstanced(mode, 0, size, instanceCount);
            } else {
                glDrawArrays(mode, 0, size);
            }
        }
    }

    // Transform positions, normals, tangents and bitangents
    void Mesh::transform(const glm::mat4& transform, const std::vector<AttributeType>& pointAttributes,
                         const std::vector<AttributeType>& vectorAttributes) {
        for(auto attrType : pointAttributes) {
            if(!hasAttribute(attrType)) continue;
            auto attr = getAccessor<glm::vec3>(attrType);
            for(size_t i = 0; i < attr.getCount(); ++i) {
                attr.set(i, glm::vec3(transform * glm::vec4(attr.get(i), 1.0f)));
            }
        }

        for(auto attrType : vectorAttributes) {
            if(!hasAttribute(attrType)) continue;
            auto attr = getAccessor<glm::vec3>(attrType);
            for(size_t i = 0; i < attr.getCount(); ++i) {
                attr.set(i, glm::vec3(transform * glm::vec4(attr.get(i), 0.0f)));
            }
        }
    }

    void Mesh::normalize(bool rescale) {
        std::pair<glm::vec3, float> bSphere;
        glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(-bSphere.first.x, -bSphere.first.y, -bSphere.first.z));
        if(rescale) t = glm::scale(t, glm::vec3(1.0f / bSphere.second));
        transform(t);
    }

    glm::vec3 Mesh::center() const {
        return boundingSphere().first;
    }

    std::pair<glm::vec3, float> Mesh::boundingSphere() const {
        AABoundingBox bBox = boundingBox();
        glm::vec3 center = (bBox.min + bBox.max) * 0.5f;
        float radius = glm::length(center - bBox.min);
        return std::make_pair(center, radius);
    }

    const AABoundingBox& Mesh::boundingBox() const {
        if(mBBoxDirty) {
            auto position = getAccessor<glm::vec3>(AttributeType::POSITION);
            mBoundingBox.min = mBoundingBox.max = position.get(0);
            for(size_t i = 1; i < position.getCount(); ++i)
                mBoundingBox.fitPoint(position.get(i));
            mBBoxDirty = false;
        }
        return mBoundingBox;
    }

    // width, height, depth along x, y, z, center is 0, 0, 0
    Mesh* boxMesh(float width, float height, float depth, const VertexFormat& format) {
        static float vertices[] = {
            // +z
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,

            // -z
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            // +x
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,

            // -x
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,

            // +y
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,

            // -y
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f
        };

        static uint8_t indices[6] = {
            0, 1, 3,
            3, 1, 2
        };

        static float normals[6][3] = {
            { 0.0f,  0.0f,  1.0f},
            { 0.0f,  0.0f, -1.0f},
            { 1.0f,  0.0f,  0.0f},
            {-1.0f,  0.0f,  0.0f},
            { 0.0f,  1.0f,  0.0f},
            { 0.0f, -1.0f,  0.0f},
        };

        glm::vec3 size = glm::vec3(width * 0.5f, height * 0.5f, depth * 0.5f);

        Mesh* mesh = new Mesh(Mesh::DrawMode::TRIANGLES);
        size_t vertexCount = 24;
        VertexBuffer* vData = mesh->addVertexBuffer(format, vertexCount);

        auto position = mesh->getAccessor<glm::vec3>(AttributeType::POSITION);
        auto normal = mesh->getAccessor<glm::vec3>(AttributeType::NORMAL);

        for(size_t i = 0; i < vertexCount; ++i) {
            position.set(i, glm::make_vec3(vertices + i*3) * size);
            int side = i / 4;
            normal.set(i, glm::make_vec3(normals[side]));
        }

        if(format.hasAttribute(AttributeType::TEXCOORD0)) {
            auto texCoord = mesh->getAccessor<glm::vec2>(AttributeType::TEXCOORD0);
            for(int side = 0; side < 6; ++side) {
                texCoord.set(side*4+0, glm::vec2(0.0f, 0.0f));
                texCoord.set(side*4+1, glm::vec2(0.0f, 1.0f));
                texCoord.set(side*4+2, glm::vec2(1.0f, 1.0f));
                texCoord.set(side*4+3, glm::vec2(1.0f, 0.0f));
            }
        }

        IndexBuffer* iData = mesh->setIndexBuffer(vertexCount, 36);
        for(int side = 0; side < 6; ++side) {
            for(int vertex = 0; vertex < 6; ++vertex) {
                iData->set(side*6+vertex, 4*side + indices[vertex]);
            }
        }

        //TODO: if(format.hasAttribute(AttributeType::TANGENT)) vData->calculateTangents();

        return mesh;
    }
}
