local kaun = require("kaun")
local shaders = require("shaders")

local cameraTrafo = kaun.newTransform()
cameraTrafo:lookAtPos(0, 0, 4,   0, 0, -1)
kaun.setViewTransform(cameraTrafo)

local shader = kaun.newShader(shaders.frag, shaders.vert)

local mesh = kaun.newBoxMesh(1, 1, 1)
local meshTrafo = kaun.newTransform()

--local texture = kaun.newCheckerTexture(512, 512, 64)
local texture = kaun.newTexture("crate.png")

function love.resize(w, h)
    kaun.setProjection(45, w/h, 0.1, 100.0)
    kaun.setViewport(0, 0, w, h)
end

function love.update(dt)
    meshTrafo:rotate(dt, 0, 1, 0)
end

function love.draw()
    kaun.clear()
    kaun.clearDepth()

    kaun.setModelTransform(meshTrafo)

    kaun.draw(mesh, shader, {
        color = {1, 1, 1, 1},
        baseTexture = texture,
    })
end
