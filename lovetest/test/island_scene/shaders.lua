local media = require("util").media

local shaderCache = {}
local unitCache = {}
local unitCounter = 0

-- http://lua-users.org/wiki/StringTrim - trim7
local match = string.match
function trim(s)
   return match(s,'^()%s*$') and '' or match(s,'^%s*(.*%S)')
end

local function getShader(name)
    if not shaderCache[name] then
        local str = assert(love.filesystem.read(media("shaders/%s.glsl", name)))

        local unit = unitCounter
        unitCounter = unitCounter + 1
        unitCache[name] = unit
        print(name, "has file number", unit)

        local parts = {
            ("#line 1 %d\n"):format(unit),
        }

        local slash = string.byte("/")
        local asterisk = string.byte("/*")
        local newline = string.byte("\n")
        local pound = string.byte("#")
        local pragmaValid = true
        local lineCounter = 1
        local partStart = 1
        local idx = 1
        while idx < str:len() do
            local cur, nxt = str:byte(idx), str:byte(idx+1)
            -- skip comments
            if cur == slash and nxt == slash then
                -- advance to newline
                local start = idx
                while idx < str:len() and str:byte(idx) ~= newline do
                    idx = idx + 1
                end
                cur, nxt = str:byte(idx), str:byte(idx+1)
            end
            if cur == slash and nxt == asterisk then
                -- advance to '*/'
                while idx < str:len() and (str:byte(idx) ~= asterisk or str:byte(idx+1) ~= slash) do
                    if str:byte(idx) == newline then
                        lineCounter = lineCounter + 1
                    end
                    idx = idx + 1
                end
                cur, nxt = str:byte(idx), str:byte(idx+1)
            end

            -- only parse pragmas if they occur after a newline only with space inbetween
            if cur == newline then
                lineCounter = lineCounter + 1
                pragmaValid = true
            end

            if pragmaValid and cur == pound then -- preprocessor directive
                local from, to = str:find("#pragma include%s+.-\n", idx)
                if from and to then
                    table.insert(parts, str:sub(partStart, to))
                    partStart = to + 1
                    local inclName = trim(str:sub(from, to):match("#pragma include%s+(.-)\n"))
                    table.insert(parts, getShader(inclName))
                    table.insert(parts, ("\n#line %d %d\n"):format(lineCounter+1, unit))
                end
            elseif cur > 32 then -- not whitespace
                pragmaValid = false
            end
            idx = idx + 1
        end
        table.insert(parts, str:sub(partStart))


        shaderCache[name] = table.concat(parts)
    end
        if name == "waterFrag" then
            --print(shaderCache[name])
        end

    return shaderCache[name]
end

return setmetatable({}, {__index = function(t, name)
    return getShader(name)
end})
