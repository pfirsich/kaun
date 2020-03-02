#pragma once

#include <memory>
#include <utility>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "aabb.hpp"
#include "log.hpp"
#include "mesh_buffers.hpp"
#include "mesh_vertexaccessor.hpp"

namespace kaun {
class Mesh {
public:
    enum class DrawMode : GLenum {
        POINTS = GL_POINTS,
        LINES = GL_LINES,
        LINE_LOOP = GL_LINE_LOOP,
        LINE_STRIP = GL_LINE_STRIP,
        TRIANGLES = GL_TRIANGLES,
        TRIANGLE_FAN = GL_TRIANGLE_FAN,
        TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
    };

private:
    DrawMode mMode;
    GLuint mVAO;
    std::vector<std::unique_ptr<VertexBuffer>> mVertexBuffers;
    std::unique_ptr<IndexBuffer> mIndexBuffer;

    mutable AABoundingBox mBoundingBox;
    mutable bool mBBoxDirty;

    static GLuint currentVAO;

public:
    static void ensureGlState();

    Mesh(DrawMode mode)
        : mMode(mode)
        , mVAO(0)
        , mIndexBuffer(nullptr)
        , mBBoxDirty(true)
    {
    }

    // I'm not really sure what I want these to do
    Mesh(const Mesh& other) = delete;
    Mesh& operator=(const Mesh& other) = delete;

    template <typename... Ts>
    VertexBuffer* addVertexBuffer(Ts&&... args)
    {
        VertexBuffer* vBuf = new VertexBuffer(std::forward<Ts>(args)...);
        for (auto& attr : vBuf->getVertexFormat().getAttributes()) {
            if (hasAttribute(attr.type) != nullptr) {
                LOG_ERROR("You are trying to add a vertex buffer to a mesh that contains vertex "
                          "attributes that are already present in another vertex buffer already in "
                          "that mesh.");
                return nullptr;
            }
        }
        mVertexBuffers.emplace_back(vBuf);
        return vBuf;
    }

    std::vector<VertexBuffer*> getVertexBuffers()
    {
        std::vector<VertexBuffer*> buffers;
        for (auto& buffer : mVertexBuffers)
            buffers.push_back(buffer.get());
        return buffers;
    }

    // returns nullptr if the given attribute is not present in any vertexbuffer
    VertexBuffer* hasAttribute(AttributeType attrType) const;

    template <typename T>
    VertexAttributeAccessor<T> getAccessor(AttributeType attrType) const
    {
        VertexBuffer* vBuf = hasAttribute(attrType);
        if (vBuf != nullptr) {
            return VertexAttributeAccessor<T>(*vBuf, attrType);
        } else {
            return VertexAttributeAccessor<T>();
        }
    }

    // add might be confusing since every Mesh object can only hold a single instance of IndexBuffer
    // if another one is added, the old one is detached and free'd
    template <typename... Ts>
    IndexBuffer* setIndexBuffer(Ts&&... args)
    {
        IndexBuffer* iData = new IndexBuffer(std::forward<Ts>(args)...);
        mIndexBuffer.reset(iData);
        return iData;
    }

    void compile();

    // instanceCount = 0 means, that the draw commands will not be instanced
    void draw(size_t instanceCount = 0);

    // ---- geometry manipulation
    // these functions are here (and not in VertexBuffer), because some of them have to
    // for example read positions and write normals or read normals and write tangents, which might
    // reside in different buffers

    /*TODO*/ void calculateVertexNormals(bool faceAreaWeighted = true);
    // sets normals so that in the fragment shader the normals can be interpolated using a "flat"
    // varying - https://www.opengl.org/wiki/Type_Qualifier_(GLSL)
    /*TODO*/ void calculateFaceNormals(bool lastVertexConvention = true);
    /*TODO*/ void calculateTangents();

    // moves center to 0, 0, 0 and radius to 1.0 if rescale = true
    void normalize(bool rescale = false);

    void transform(const glm::mat4& transform,
        const std::vector<AttributeType>& pointAttributes = { AttributeType::POSITION },
        const std::vector<AttributeType>& vectorAttributes
        = { AttributeType::NORMAL, AttributeType::TANGENT, AttributeType::BITANGENT });

    const AABoundingBox& boundingBox() const;

    // Centroid of the bounding box
    glm::vec3 center() const;

    // position and radius
    std::pair<glm::vec3, float> boundingSphere() const;

    // width, height, depth along x, y, z, center is 0, 0, 0
    static Mesh* box(float width, float height, float depth, const VertexFormat& format);

    // Stacks represents the number of elements on the y axis
    static Mesh* sphere(float radius, int slices, int stacks, bool cubeProjectionTexCoords,
        const VertexFormat& format);

    // Make sure this can be used to make a "line mesh"?
    static Mesh* plane(
        float width, float height, int segmentsX, int segmentsY, const VertexFormat& format);

    static Mesh* objFile(const std::string& filename, const VertexFormat& format);
    static Mesh* objFile(const uint8_t* buffer, size_t size, const VertexFormat& format);

    ///////////////////////////////////////////////////////////////////////////
    /*
    circleMesh(int radius, int segments, const VertexFormat&format = defaultFormat);
    cylinderMesh(radiusTop, radiusBottom) -> Cylinder
    subdivide(Mesh, iterations) //
    http://answers.unity3d.com/questions/259127/does-anyone-have-any-code-to-subdivide-a-mesh-and.html
    like this normalsMesh - generates normals from another VertexBuffer - maybe GL_LINES or actual
    arrows? gridMesh frustumMesh - from camera/any perspective matrix, inverts is and converts ndc -
    corners coordinateSystem

    // A Helper class or something for ribbons/extruded geometry (takes a shape, that's appended and
    maybe a radius (animated over time))
    */
};

}
