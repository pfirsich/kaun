local loaded = false
local function load(name)
    if loaded then error("Only load one example at a time, please!") end
    love.filesystem.setRequirePath(love.filesystem.getRequirePath() .. (";%s/?.lua"):format(name))
    require(name .. ".main")
end

--load("spinny_box")
load("island_scene")
--load("love_graphics_interop")
--load("fullscreen_quad")
