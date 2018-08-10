R"luastring"--(
-- ^^^^^^^^ DO NOT REMOVE ^^^^^^^^

_kaun = {}

function _kaun.getFileData(path)
    local f, err = love.filesystem.newFileData(path)
    if not f then
        error(err)
    end
    return f:getPointer(), f:getSize()
end

function love.run()
    local width, height, flags = love.window.getMode()
    love.resize(width, height)
    assert(flags.depth > 0, "Window needs a depth buffer! Add 't.window.depth = 24' to conf.lua.")


    if love.load then love.load(love.arg.parseGameArguments(arg), arg) end

    -- We don't want the first frame's dt to include time taken by love.load.
    if love.timer then love.timer.step() end

    local dt = 0

    -- Main loop time.
    return function()
        -- Process events.
        if love.event then
            love.event.pump()
            for name, a,b,c,d,e,f in love.event.poll() do
                if name == "quit" then
                    if not love.quit or not love.quit() then
                        return a or 0
                    end
                end
                love.handlers[name](a,b,c,d,e,f)
            end
        end

        -- Update dt, as we'll be passing it to update
        if love.timer then dt = love.timer.step() end

        -- Call update and draw
        if love.update then love.update(dt) end -- will pass 0 if love.timer is disabled

        if love.graphics.isActive() then
            if love.draw then love.draw() end
            kaun.flush()
            love.graphics.present()
        end
    end
end

-- vvvvvvvv DO NOT REMOVE vvvvvvvv
--)luastring"--"
