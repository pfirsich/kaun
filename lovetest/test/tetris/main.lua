kaun = require("kaun")
local shaders = require("shared.shaders")

local cameraTrafo = kaun.newTransform()
cameraTrafo:lookAtPos(0, -5, 17,   0, 7, 0)
kaun.setViewTransform(cameraTrafo)

local mesh = kaun.newBoxMesh(1, 1, 1)
local meshTrafo = kaun.newTransform()

local shader = kaun.newShader(shaders.defaultVertex, shaders.defaultTexturedLambert)
local texture = kaun.newTexture("shared/assets/crate.png")

-- game - http://tetris.wikia.com/wiki/Tetris_Guideline
local blockLetters = {"I", "O", "T", "S", "Z", "J", "L"}

local blockColors = {
    I = {0.0, 1.0, 1.0, 1.0},
    O = {1.0, 1.0, 0.0, 1.0},
    T = {1.0, 0.0, 1.0, 1.0},
    S = {0.0, 1.0, 0.0, 1.0},
    Z = {1.0, 0.0, 0.0, 1.0},
    J = {0.0, 0.0, 1.0, 1.0},
    L = {1.0, 0.5, 0.25, 1.0},
}
borderColor = {0.2, 0.2, 0.2, 1.0}

-- these describe square regions, the center of which is the rotation pivot point
local blockLayout = {
    I = {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}},
    O = {{1, 1}, {1, 1}},
    T = {{0, 0, 0}, {1, 1, 1}, {0, 1, 0}},
    S = {{0, 0, 0}, {1, 1, 0}, {0, 1, 1}},
    Z = {{0, 0, 0}, {0, 1, 1}, {1, 1, 0}},
    J = {{0, 0, 0}, {1, 1, 1}, {1, 0, 0}},
    L = {{0, 0, 0}, {1, 1, 1}, {0, 0, 1}},
}

local gridW, gridH = 10, 22
local showGridH = 20
local grid = {} -- a 2d array of colors, nil = empty
for y = 1, gridH do
    grid[y] = {}
end

local curBlock = nil -- a table containing color, letter and geometry of currently falling block
local nextBlockStep = 0
local blockStepInterval = 0.5
local clearedLines = 0

function love.resize(w, h)
    kaun.setProjection(45, w/h, 0.1, 100.0)
    kaun.setWindowDimensions(w, h)
end

local function spawnBlock()
    local letter = blockLetters[love.math.random(1, #blockLetters)]
    local layout = blockLayout[letter]
    local width = #layout[1]
    curBlock = {
        offset = {math.floor(gridW/2 - width/2), gridH},
        color = blockColors[letter],
        grid = layout, -- when we rotate the block, a copy is created
    }
end

function love.load()
    spawnBlock()
end

local function checkBlock(block)
    block = block or curBlock
    for y = 1, #block.grid do
        for x = 1, #block.grid[y] do
            local gridX, gridY = block.offset[1] + x, block.offset[2] + y
            if block.grid[y][x] > 0 and ((grid[gridY] and grid[gridY][gridX]) or
                    gridX < 1 or gridX > gridW or gridY < 1) then
                return false
            end
        end
    end
    return true
end

local function moveBlock(dx, dy)
    local oldPos = curBlock.offset
    curBlock.offset = {curBlock.offset[1] + dx, curBlock.offset[2] + dy}
    if checkBlock() then
        return true
    else
        curBlock.offset = oldPos
        return false
    end
end

function love.update(dt)
    nextBlockStep = nextBlockStep - (love.keyboard.isDown("down") and 5.0 or 1.0) * dt
    if nextBlockStep < 0.0 then
        nextBlockStep = blockStepInterval

        if not moveBlock(0, -1) then
            -- commit to grid
            for y = 1, #curBlock.grid do
                for x = 1, #curBlock.grid[y] do
                    if curBlock.grid[y][x] > 0 then
                        grid[y + curBlock.offset[2]][x + curBlock.offset[1]] = curBlock.color
                    end
                end
            end

            -- remove cleared lines
            local oldClearedLines = clearedLines
            local y = 1
            while y < showGridH do
                local fullLine = true
                for x = 1, gridW do
                    if not grid[y][x] then
                        fullLine = false
                        break
                    end
                end

                if fullLine then
                    -- move all lines above down and keep y the same
                    for iy = y, gridH do
                        grid[iy] = grid[iy+1] or {}
                    end
                    clearedLines = clearedLines + 1
                else
                    y = y + 1
                end
            end

            -- check if too high
            for x = 1, gridW do
                if grid[showGridH+1][x] then
                    error("You lose! Cleared lines: " .. tostring(clearedLines))
                end
            end

            if clearedLines ~= oldClearedLines then
                blockStepInterval = 0.9 * blockStepInterval
            end

            spawnBlock()
        end
    end
end

local function rotatedGrid(grid, dir)
    local w, h = #grid[1], #grid
    local cx, cy = (w - 1) / 2 + 1, (h - 1) / 2 + 1 -- rotation center
    local rotated = {}
    for y = 1, h do
        rotated[y] = {}
        for x = 1, w do
            -- counterclockwise rotation is negative of clockwise rotation
            local rotX, rotY = dir * -(y - cy), dir * (x - cx) -- rotation matrix for 90°
            rotX, rotY = math.floor(rotX + cx + 0.5), math.floor(rotY + cy + 0.5) -- reverse center translation
            rotated[y][x] = grid[rotY][rotX]
        end
    end
    return rotated
end

local function rotateBlock()
    local oldGrid = curBlock.grid
    curBlock.grid = rotatedGrid(curBlock.grid, 1)
    -- check if it fits, if not, wiggle left and right to see if it fits
    local startOffset = curBlock.offset[1]
    for dx = 0, 2 do -- how much we wiggle (I know this checks dx = 0 twice, but I don't mind)
        for dir = -1, 1, 2 do -- wiggle direction
            curBlock.offset[1] = startOffset + dir * dx
            if checkBlock() then
                return true
            end
        end
    end
    -- nothing fits => undo everything (just don't do any rotation)
    curBlock.offset[1] = startOffset
    curBlock.grid = oldGrid
    return false
end

function love.keypressed(key)
    if key == "left" then
        moveBlock(-1, 0)
    elseif key == "right" then
        moveBlock(1, 0)
    elseif key == "up" then
        rotateBlock()
    elseif key == "down" then
        moveBlock(0, -1)
    end
end

local function drawBlock(x, y, color)
    meshTrafo:setPosition(-gridW/2 + (x-1), (y-1), 0.0)
    kaun.setModelTransform(meshTrafo)
    kaun.draw(mesh, shader, {
        color = color,
        baseTexture = texture,
    })
end

function love.draw()
    kaun.clear()
    kaun.clearDepth()

    for y = 1, showGridH do
        for x = 1, gridW do
            if grid[y][x] then drawBlock(x, y, grid[y][x]) end
        end
        -- draw left and right border
        drawBlock(0, y, borderColor)
        drawBlock(gridW + 1, y, borderColor)
    end
    -- draw bottom border
    for x = 0, gridW + 1 do
        drawBlock(x, 0, borderColor)
    end

    -- draw the currently falling block
    for y = 1, #curBlock.grid do
        for x = 1, #curBlock.grid[y] do
            if curBlock.grid[y][x] > 0 then
                drawBlock(curBlock.offset[1] + x, curBlock.offset[2] + y, curBlock.color)
            end
        end
    end
end
