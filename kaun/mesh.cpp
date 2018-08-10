#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "mesh.hpp"

namespace kaun {
    GLuint Mesh::currentVAO = 0;

    void Mesh::ensureGlState() {
        glBindVertexArray(currentVAO);
    }

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
        currentVAO = 0;

        // VAO stores the last bound ELEMENT_BUFFER state, so as soon as the VAO is unbound, unbind the VBO
        if(mIndexBuffer != nullptr) mIndexBuffer->unbind();
    }

    void Mesh::draw(size_t instanceCount) {
        if(mVAO == 0) {
            compile();
        }

        if(currentVAO != mVAO) {
            glBindVertexArray(mVAO);
            currentVAO = mVAO;
        }

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
        std::pair<glm::vec3, float> bSphere = boundingSphere();
        LOG_DEBUG("Mesh bounding sphere %s, %f", glm::to_string(bSphere.first).c_str(), bSphere.second);
        glm::mat4 t(1.0);
        if(rescale) t = glm::scale(t, glm::vec3(1.0f / bSphere.second));
        t = glm::translate(t, glm::vec3(-bSphere.first));
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
    Mesh* Mesh::box(float width, float height, float depth, const VertexFormat& format) {
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
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,

            // -y
            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f
        };

        static uint8_t indices[6] = {
            0, 1, 2,
            0, 2, 3
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

    // Stacks represents the subdivision on the y axis (excluding the poles)
    Mesh* Mesh::sphere(float radius, int slices, int stacks, bool cubeProjectionTexCoords, const VertexFormat& format) {
        assert(slices > 3 && stacks > 2);
        Mesh* mesh = new Mesh(Mesh::DrawMode::TRIANGLES);
        size_t vertexCount = slices*stacks;
        mesh->addVertexBuffer(format, vertexCount);

        auto position = mesh->getAccessor<glm::vec3>(AttributeType::POSITION);
        auto normal = mesh->getAccessor<glm::vec3>(AttributeType::NORMAL);
        auto texCoord = mesh->getAccessor<glm::vec2>(AttributeType::TEXCOORD0);

        /* This should probably be:
        auto normal = VertexAttributeAccessor();
        if(format.hasAttribute(AttributeType::NORMAL)) {
            normal = (*vData)[AttributeType::NORMAL];
        }

        That way there will not be a debug log message every time you create a sphere mesh,
        but I did it like this to show that it's possible to do it like this without everything going up in flames
         */

        int index = 0;
        for(int stack = 0; stack < stacks; ++stack) {
            float stackAngle = glm::pi<float>() / (stacks - 1) * stack;
            float xzRadius = glm::sin(stackAngle) * radius;
            float y = glm::cos(stackAngle) * radius;
            for(int slice = 0; slice < slices; ++slice) {
                float sliceAngle = 2.0f * glm::pi<float>() / (slices - 1) * slice;
                position.set(index, glm::vec3(glm::cos(sliceAngle) * xzRadius, y, glm::sin(sliceAngle) * xzRadius));
                normal.set(index, glm::normalize(position.get(index)));
                if(cubeProjectionTexCoords) {
                    // http://www.gamedev.net/topic/443878-texture-lookup-in-cube-map/
                    glm::vec3 dir = normal.get(index);
                    glm::vec3 absDir = glm::abs(dir);
                    int majorDirIndex = 0;
                    if(absDir.x >= absDir.y && absDir.x >= absDir.z) majorDirIndex = 0;
                    if(absDir.y >= absDir.x && absDir.y >= absDir.z) majorDirIndex = 1;
                    if(absDir.z >= absDir.x && absDir.z >= absDir.y) majorDirIndex = 2;
                    float majorDirSign = 1.0f;
                    if(dir[majorDirIndex] < 0.0f) majorDirSign = -1.0f;

                    glm::vec3 v;
                    switch(majorDirIndex) {
                        case 0:
                            v = glm::vec3(-majorDirSign * dir.z, -dir.y, dir.x);
                            break;
                        case 1:
                            v = glm::vec3(dir.x, majorDirSign * dir.z, dir.y);
                            break;
                        case 2:
                            v = glm::vec3(majorDirSign * dir.x, -dir.y, dir.z);
                            break;
                        default:
                            assert(false);
                            break;
                    }

                    texCoord.set(index++, glm::vec2((v.x/glm::abs(v.z) + 1.0f) / 2.0f, (v.y/glm::abs(v.z) + 1.0f) / 2.0f));
                } else {
                    texCoord.set(index++, glm::vec2(sliceAngle / 2.0f / glm::pi<float>(), stackAngle / glm::pi<float>()));
                }
            }
        }

        int triangles = 2 * (slices - 1) * (stacks - 1);

        IndexBuffer* iData = mesh->setIndexBuffer(vertexCount, triangles * 3);
        index = 0;
        for(int stack = 0; stack < stacks - 1; ++stack) {
            int firstStackVertex = stack * slices;
            for(int slice = 0; slice < slices - 1; ++slice) {
                int firstFaceVertex = firstStackVertex + slice;
                int nextVertex = firstFaceVertex + 1;
                iData->set(index++, nextVertex + slices);
                iData->set(index++, firstFaceVertex + slices);
                iData->set(index++, firstFaceVertex);

                iData->set(index++, nextVertex);
                iData->set(index++, nextVertex + slices);
                iData->set(index++, firstFaceVertex);
            }
        }

        //if(format.hasAttribute(AttributeType::TANGENT)) vData->calculateTangents();

        return mesh;
    }

    Mesh* Mesh::plane(float width, float height, int segmentsX, int segmentsY, const VertexFormat& format) {
        assert(segmentsX >= 1 && segmentsY >= 1);

        Mesh* mesh = new Mesh(Mesh::DrawMode::TRIANGLES);
        size_t vertexCount = (segmentsX+1)*(segmentsY+1);
        mesh->addVertexBuffer(format, vertexCount);

        auto position = mesh->getAccessor<glm::vec3>(AttributeType::POSITION);
        auto normal = mesh->getAccessor<glm::vec3>(AttributeType::NORMAL);
        auto texCoord = mesh->getAccessor<glm::vec2>(AttributeType::TEXCOORD0);

        int index = 0;
        glm::vec2 size(width, height);
        for(int y = 0; y <= segmentsY; ++y) {
            for(int x = 0; x <= segmentsX; ++x) {
                glm::vec2 pos2D = glm::vec2((float)x / segmentsX, (float)y / segmentsY);
                texCoord.set(index, pos2D);
                pos2D = pos2D * size - 0.5f * size;
                position.set(index, glm::vec3(pos2D.x, 0.0f, pos2D.y));
                normal.set(index++, glm::vec3(0.0f, 1.0f, 0.0f));
            }
        }

        IndexBuffer* iData = mesh->setIndexBuffer(vertexCount, segmentsX*segmentsY*2*3);
        index = 0;
        int perLine = segmentsX + 1;
        for(int y = 0; y < segmentsY; ++y) {
            for(int x = 0; x < segmentsX; ++x) {
                int start = x + y * perLine;
                iData->set(index++, start);
                iData->set(index++, start + perLine);
                iData->set(index++, start + perLine + 1);

                iData->set(index++, start + perLine + 1);
                iData->set(index++, start + 1);
                iData->set(index++, start);
            }
        }

        return mesh;
    }

    struct objVertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    Mesh* Mesh::objFile(const std::string& filename, const VertexFormat& format) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
            filename.c_str(), nullptr, true);

        if (ret) {
            LOG_WARNING("Loading object file: %s", err.c_str());
        } else {
            LOG_ERROR("Error loading object file: %s", err.c_str());
            return nullptr;
        }

        // Per-Face Materials are annoying and I am going to ignore them completely.
        // If I wanted to respect them, I would have to bucket the faces by material, then
        // build vertex/index buffers from them. This is annoying and slow.

        std::vector<objVertex> vertices;
        // loop shapes
        for(size_t s = 0; s < shapes.size(); ++s) {
            // loop faces
            for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
                assert(shapes[s].mesh.num_face_vertices[f] == 3);
                bool missingNormal = true;
                for(size_t v = 0; v < 3; ++v) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[f*3 + v];
                    vertices.emplace_back();
                    objVertex& vertex = vertices.back();
                    vertex.position = glm::vec3(attrib.vertices[3*idx.vertex_index+0],
                                                attrib.vertices[3*idx.vertex_index+1],
                                                attrib.vertices[3*idx.vertex_index+2]);
                    if(idx.normal_index >= 0) {
                        vertex.normal = glm::vec3(attrib.normals[3*idx.normal_index+0],
                                                  attrib.normals[3*idx.normal_index+1],
                                                  attrib.normals[3*idx.normal_index+2]);
                        missingNormal = false;
                    }
                    if(idx.texcoord_index >= 0) {
                        vertex.texCoord = glm::vec2(attrib.texcoords[2*idx.texcoord_index+0],
                                                    attrib.texcoords[2*idx.texcoord_index+1]);

                    }
                }

                if(missingNormal) {
                    glm::vec3& p1 = vertices[vertices.size() - 3].position;
                    glm::vec3& p2 = vertices[vertices.size() - 2].position;
                    glm::vec3& p3 = vertices[vertices.size() - 1].position;
                    glm::vec3 rel21 = p1 - p2;
                    glm::vec3 rel23 = p3 - p2;
                    glm::vec3 normal = glm::normalize(glm::cross(rel23, rel21));
                    for(size_t v = vertices.size() - 3; v < vertices.size(); ++v) {
                        vertices[v].normal = normal;
                    }
                }
            }
        }

        Mesh* mesh = new Mesh(Mesh::DrawMode::TRIANGLES);
        VertexBuffer* vData = mesh->addVertexBuffer(format, vertices.size());

        auto position = mesh->getAccessor<glm::vec3>(AttributeType::POSITION);
        auto normal = mesh->getAccessor<glm::vec3>(AttributeType::NORMAL);
        auto texCoord = mesh->getAccessor<glm::vec2>(AttributeType::TEXCOORD0);
        assert(position.isValid() && normal.isValid());

        for(size_t v = 0; v < vertices.size(); ++v) {
            const objVertex& vertex = vertices[v];
            position.set(v, vertex.position);
            normal.set(v, vertex.normal);
            if(texCoord.isValid()) {
                texCoord.set(v, vertex.texCoord);
            }
        }

        LOG_INFO("Loaded mesh '%s' (%d vertices, %d faces)", filename.c_str(), vertices.size(), vertices.size()/3);

        return mesh;
    }
}
