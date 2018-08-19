local terrain = {}

local vertexFormat = kaun.newVertexFormat({"POSITION", 3, "F32"},
                                          {"NORMAL", 3, "F32"},
                                          {"TEXCOORD0", 2, "F32"})

local function hullCurve(nx, ny, hullRadius)
    local relX, relY = nx - 0.5, ny - 0.5
    local dist = math.sqrt(relX*relX + relY*relY)
    if dist < hullRadius then
        return 1
    else
        return math.max(0.0, 1.0 - (dist - hullRadius) / (0.5 - hullRadius))
    end
end

local function generateHeightmap(subDiv, baseFreq, intervalFactor, weights, redistribution, hullRadius)
    local weightSum = 0
    for _, weight in ipairs(weights) do
        weightSum = weightSum + weight
    end

    local heightmap = {}
    local nx, ny -- normalized coordinates
    for y = 1, subDiv do
        heightmap[y] = {}
        ny = (y-1) / subDiv
        for x = 1, subDiv do
            nx = (x-1) / subDiv

            local height = 0
            local freq = baseFreq
            for _, weight in ipairs(weights) do
                local noise = love.math.noise(nx * freq, ny * freq)
                height = height + weight / weightSum * noise
                freq = freq * intervalFactor
            end
            if hullRadius then
                height = height * hullCurve(nx, ny, hullRadius)
            end
            if redistribution then
                height = redistribution(height)
            end
            heightmap[y][x] = height
        end
    end

    return heightmap
end

local function createMesh(heightmap, sizeXZ, height)
    local subDiv = #heightmap
    local scaleXZ = sizeXZ / subDiv
    local vertices = {}

    function pushVert(x, y)
        table.insert(vertices, {
            (x-1) * scaleXZ, heightmap[y][x] * height, (y-1) * scaleXZ,
            0, 1, 0,
            (x-1) / (subDiv-1), (y-1) / (subDiv-1),
        })
    end

    function calcNormal()
        local p1 = vec3(unpack(vertices[#vertices-2]))
        local p2 = vec3(unpack(vertices[#vertices-1]))
        local p3 = vec3(unpack(vertices[#vertices-0]))
        local rel21 = p1 - p2
        local rel23 = p3 - p2
        local normal = vec3.normalize(vec3.cross(rel23, rel21))
        for i = 0, 2 do
            vertices[#vertices-i][4] = normal.x
            vertices[#vertices-i][5] = normal.y
            vertices[#vertices-i][6] = normal.z
        end
    end

    for y = 1, subDiv - 1 do
        for x = 1, subDiv - 1 do
            pushVert(x + 0, y + 0)
            pushVert(x + 0, y + 1)
            pushVert(x + 1, y + 1)
            calcNormal()

            pushVert(x + 0, y + 0)
            pushVert(x + 1, y + 1)
            pushVert(x + 1, y + 0)
            calcNormal()
        end
    end

    return kaun.newMesh("triangles", vertexFormat, vertices)
end

-- hx, hy in heightmap coordinates
local function getHeightmapHeight(heightmap, hx, hy)
    local subDiv = #heightmap
    if hx <= 1 or hx >= subDiv or hy <= 1 or hy >= subDiv then
        return 0
    else
        -- integer
        local ix = math.floor(hx)
        local iy = math.floor(hy)
        -- fractional
        local fx = hx - math.floor(hx)
        local fy = hy - math.floor(hy)

        -- 0------------1
        -- |T         . |
        -- |        .   |
        -- |  q   .     |
        -- |    .       |
        -- |  .         |
        -- |.          B|
        -- 2------------3
        --
        -- q is at hx, hy
        -- 0 is at ix, iy
        -- 1 is at ix + 1, iy
        -- 2 is at ix, iy + 1
        -- 3 is at ix + 1, iy + 1

        -- let p0, p2, p2 be the corner points of the triangle containing q
        -- and d1, d2 the directions between p0 and p1, p2

        -- we know that there exist s, t:
        -- (p0 + s * d1 + t * d2).xy = q.xy
        -- => p0.x + s * d1.x + t * d2.x = q.x
        --    p0.y + s * d1.y + t * d2.y = q.y

        if fx + fy < 1 then -- p is in T
            -- choose p0 = 0, p1 = 1, p2 = 2

            -- d1.x = 1, d1.y = 0
            -- d2.x = 0, d2.y = 1
            --  => p0.x + s = q.x
            --     p0.y + t = q.y
            -- <=> s = q.x - p0.x = fx
            --     t = q.y - p0.y = fy

            -- => q.h = p0.h +  s * d1.h +  t * d2.h
            --  = q.h = p0.h + fx * d1.h + fy * d2.h
            local p0h = heightmap[iy][ix]
            return p0h + fx * (heightmap[iy][ix+1] - p0h)
                       + fy * (heightmap[iy+1][ix] - p0h)
        else -- p is in B
            -- choose p0 = 3, p1 = 2, p2 = 1

            -- d1.x = -1, d1.y = 0
            -- d2.x = 0, d2.y = -1
            --  => p0.x - s = q.x
            --     p0.y - t = q.y
            -- <=> s = p0.x - q.x = 1 - fx
            --     t = p0.y - q.y = 1 - fy

            -- => q.h = p0.h +      s * d1.h +      t * d2.h
            --  = q.h = p0.h + (1-fx) * d1.h + (1-fy) * d2.h
            local p0h = heightmap[iy+1][ix+1]
            return p0h + (1-fx) * (heightmap[iy+1][ix] - p0h)
                       + (1-fy) * (heightmap[iy][ix+1] - p0h)
        end
    end
end

function terrain.setup(sizeXZ, height, subDiv, baseFreq, intervalFactor, weights, hullRadius, redistribution)
    terrain.size = sizeXZ
    terrain.height = height
    terrain.subDiv = subDiv

    terrain.heightmap = generateHeightmap(subDiv, baseFreq, intervalFactor, weights, hullRadius, redistribution)
    terrain.mesh = createMesh(terrain.heightmap, sizeXZ, height)

    terrain.transform = kaun.newTransform()
    terrain.transform:setPosition(-sizeXZ/2, 0, -sizeXZ/2)
end

-- wx, wz in world space
function terrain.getHeight(wx, wz)
    local terrainX, terrainY, terrainZ = terrain.transform:getPosition()
    local relX, relZ = wx - terrainX, wz - terrainZ
    local tx = relX / terrain.size * terrain.subDiv + 1
    local ty = relZ / terrain.size * terrain.subDiv + 1
    return getHeightmapHeight(terrain.heightmap, tx, ty) * terrain.height + terrainY
end

return terrain
