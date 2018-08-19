kaun = require("kaun")
local shaders = require("shared.shaders")

-- I don't have to set/update the projection matrix or view matrix here, since
-- the full screen quad vertex shader does not use them
-- It's built like that on purpose so view/projection doesn't have to be changed to render an fs quad

local fullScreenQuadFormat = kaun.newVertexFormat({"POSITION", 2, "F32"})
local fullScreenQuad = kaun.newMesh("triangle_strip", fullScreenQuadFormat,
                                    {{-1, -1}, { 1, -1}, {-1,  1}, { 1,  1}})
local fullScreenQuadShader = kaun.newShader(shaders.fsQuadVert, shaders.fsQuadFrag)
local fullScreenQuadState = kaun.newRenderState()
fullScreenQuadState:setDepthTest("disabled")

local texture = kaun.newTexture("shared/assets/test.png")

function love.resize(w, h)
    kaun.setWindowDimensions(w, h)
end

function love.draw()
    kaun.draw(fullScreenQuad, fullScreenQuadShader, {
        tex = texture,
        flipY = true,
    }, fullScreenQuadState)
end
