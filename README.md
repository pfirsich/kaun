# kaun
kaun is a replacement for [löve](https://love2d.org)'s built-in `love.graphics` module intended for 3D graphics. It is a Lua module you can `require` from a shared library.

Be aware that the code is horrible, because I just wanted to try the idea out in a couple of weeks 2 years ago (at the time of writing this). Now I decided to make a game and I am trying to not trap myself in the perpetual cycle of rewriting. If you want to improve anything (and there is a lot) feel invited to contribute!

## Why?
If you know your way around löve, you might know that additions to löve 11 in particular (esp. [`setDepthMode`](https://love2d.org/wiki/love.graphics.setDepthMode)) have made it easier to do 3D graphics with löve, but for the foreseeable future it will not be primarily intended for 3D graphics.

And if you know your way around löve a bit more, you will also have noticed how a handful of ambitious developers have delved far into 3D graphics with löve and stumbled upon new requirements for features that are hard to fit cleanly into löve as it currently is. So they either take a while to make them work or don't end up there at all.

kaun should be a way to not have to worry about these things and add whatever is necessary or useful in the future (like occlusion queries, uniform buffer objects, geometry/compute shaders, indirect draws etc.).

It is also a way to to make techniques possible that require more CPU load than might be comfortably manageable with LuaJIT (like software rasterization for occlusion culling or performant state sorting).

The goal is still to have a "lövely" API (fairly low-level, but enough to abstract away boring OpenGL details and to leave space for relevant optimizations).

## Examples
Hello Spinning Cube (from here: [main.lua](lovetest/test/spinny_box/main.lua)):

[VIDEO](https://streamable.com/9sx5a)

```lua
kaun = require("kaun")
local shaders = require("shared.shaders")

local cameraTrafo = kaun.newTransform()
cameraTrafo:lookAtPos(0, 0, 4,   0, 0, 0)
kaun.setViewTransform(cameraTrafo)

local shader = kaun.newShader(shaders.defaultVertex, shaders.defaultTexturedLambert)

local mesh = kaun.newBoxMesh(1, 1, 1)
local meshTrafo = kaun.newTransform()

local texture = kaun.newCheckerTexture(512, 512, 64)

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

    kaun.flush()
end
```

Videos of the other Examples:
* [Island Scene](https://streamable.com/bkrr3)
* [Tetris](https://streamable.com/r424g)
