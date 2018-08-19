local terrain = require("terrain")
local util = require("util")

local bool2int = util.bool2int

local camera = {}

camera.transform = kaun.newTransform()
setmetatable(camera, {__index = function(tbl, key)
    if camera.transform[key] then
        return function(...)
            return camera.transform[key](camera.transform, ...)
        end
    end
end})
camera.position = vec3(0, 0, 0)

function camera.uncollide()
    camera.position.y = math.max(camera.position.y,
        terrain.getHeight(camera.position.x, camera.position.z) + 0.25)
end

function camera.update(dt)
    local lk = love.keyboard
    local speed = 2.0
    if lk.isDown("lshift") then speed = 4.0 end
    speed = speed * dt

    local move = vec3(0, 0, 0)
    move.x = bool2int(lk.isDown("d")) - bool2int(lk.isDown("a"))
    move.y = bool2int(lk.isDown("r")) - bool2int(lk.isDown("f"))
    move.z = bool2int(lk.isDown("s")) - bool2int(lk.isDown("w"))

    if move:len() > 0.5 then
        local moveY = move.y
        move = vec3(camera.localDirToWorld(move.x, move.y, move.z))
        move.y = move.y + moveY -- move up down with r/f in world space
        camera.position = camera.position + move:normalize() * speed
    end

    camera.uncollide()
end

function camera.mouseLook(dx, dy, sensitity)
    camera.rotateWorld(sensitity * dx, 0, 1, 0)
    camera.rotate(sensitity * dy, 1, 0, 0)
end

function camera.getTransform()
    camera.transform:setPosition(camera.position:unpack())
    return camera.transform
end

return camera
