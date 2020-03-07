R"luastring"--(
-- ^^^^^^^^ DO NOT REMOVE ^^^^^^^^

_kaun = {}

function _kaun.getFileData(path)
    local f, err = love.filesystem.newFileData(path)
    if not f then
        error(err)
    end
    -- TODO: getPointer will be deprecated soon. Return tostring(f:getFFIPointer()) instead
    -- return FileData so it's on the Lua stack and won't be garbage collected
    return f, f:getPointer(), f:getSize()
end

function love.run()
    local width, height, flags = love.window.getMode()
    kaun.setWindowDimensions(width, height)
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
            -- pump puts all events in the queue and processes them
            -- SDL_WINDOWEVENT_RESIZED or SDL_WINDOWEVENT_SIZE_CHANGED
            -- can result in changes do viewport and scissor state
            -- the latter might trigger a flushing of batched draws, but if you don't do
            -- anything weird, no batched draws will be queued after the last
            -- love.graphics.present(), which also flushed the batched draws
            kaun.beginLoveEventPump()
            love.event.pump()
            kaun.endLoveEventPump()

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
            -- This does not modify GL state IFF the render queue is empty
            kaun.flush()
            -- if no batched draws are pending, this should only swap buffers and not modify state
            -- but this might have to be guarded with begin/endLoveGraphics too eventually
            love.graphics.present()
        end
    end
end

local origSetMode = love.window.setMode
function love.window.setMode()
    error("setMode recreates the OpenGL context and kaun can not recover from this. Please do all setMode calls before importing kaun.")
end

-- apparently in 11.1 love has it's own errorhandler at love.errhand,
-- but you are supposed to define love.errorhandler
function love.errorhandler(msg)
    kaun.beginLoveGraphics()
    love.window.setMode = origSetMode
    return love.errhand(msg)
end

-- vvvvvvvv DO NOT REMOVE vvvvvvvv
--)luastring"--"
