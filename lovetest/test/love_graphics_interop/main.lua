kaun = require("kaun")
local shaders = require("shared.shaders")

local cameraTrafo = kaun.newTransform()
cameraTrafo:lookAtPos(0, 0, 4,   0, 0, -1)
kaun.setViewTransform(cameraTrafo)

local shader = kaun.newShader(shaders.defaultVertex, shaders.defaultTexturedLambert)

local mesh = kaun.newBoxMesh(1, 1, 1)
local meshTrafo = kaun.newTransform()

local texture = kaun.newTexture("shared/assets/crate.png")

kaun.beginLoveGraphics()
local loveTex = love.graphics.newImage("shared/assets/crate.png")
kaun.endLoveGraphics()

function love.resize(w, h)
    kaun.setProjection(45, w/h, 0.1, 100.0)
    kaun.setWindowDimensions(w, h)
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

    kaun.beginLoveGraphics() -- calls kaun.flush
    love.graphics.setColor(1, 1, 1)
    love.graphics.draw(loveTex, 0, 0, 0, 0.7)
    love.graphics.setColor(0, 0, 0)
    love.graphics.print("FPS: " .. love.timer.getFPS(), 5, 5)
    -- all draws have to finish in this block! flush batches!
    love.graphics.flushBatch()
    kaun.endLoveGraphics()
end
