#pragma once

#include <glad/glad.h>

#include "window.hpp"
#include "log.hpp"
#include "signal.hpp"
#include "render.hpp"
#include "transform.hpp"
#include "aabb.hpp"
#include "mesh.hpp"
#include "renderstate.hpp"
#include "shader.hpp"
#include "utility.hpp"
#include "texture.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace kaun {
    extern void init(bool loadGl = false);
    extern void ensureGlState();
}
