local util = {}

function util.unpackmat4(mat, index)
    index = index or 1
    if index > 16 then return end
    return mat[index], util.unpackmat4(mat, index + 1)
end

function util.randf(min, max)
    min = min or 1
    if max == nil then
        max = min
        min = -min
    end
    return min + love.math.random() * (max - min)
end

function util.bool2int(b)
    return b and 1 or 0
end

function util.media(str, ...)
    return "island_scene/media/" .. str:format(...)
end

return util
