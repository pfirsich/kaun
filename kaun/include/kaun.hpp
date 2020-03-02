#pragma once

#include <glad/glad.h>

#include "aabb.hpp"
#include "log.hpp"
#include "mesh.hpp"
#include "render.hpp"
#include "renderstate.hpp"
#include "rendertarget.hpp"
#include "shader.hpp"
#include "signal.hpp"
#include "texture.hpp"
#include "transform.hpp"
#include "utility.hpp"
#include "window.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace kaun {
bool initGl();
void init();
void ensureGlState();
}
