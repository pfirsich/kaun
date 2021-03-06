enum class LuaType {
    NIL = LUA_TNIL,
    NUMBER = LUA_TNUMBER,
    BOOLEAN = LUA_TBOOLEAN,
    STRING = LUA_TSTRING,
    TABLE = LUA_TTABLE,
    FUNCTION = LUA_TFUNCTION,
    USERDATA = LUA_TUSERDATA,
    THREAD = LUA_TTHREAD,
    LIGHTUSERDATA = LUA_TLIGHTUSERDATA,
};

template <typename T>
class LuaEnum {
private:
    std::string name;
    std::unordered_map<std::string, T> valueMap;

public:
    LuaEnum(const std::string& name, const std::unordered_map<std::string, T>& valueMap)
        : name(name)
        , valueMap(valueMap)
    {
    }

    std::string getName() const
    {
        return name;
    }
    const std::unordered_map<std::string, T> getValueMap() const
    {
        return valueMap;
    }

    std::vector<std::string> getValues() const
    {
        std::vector<std::string> values;
        for (auto value : valueMap) {
            values.push_back(value.first);
        }
        return values;
    }

    std::string getValuesString() const
    {
        std::stringstream s;
        bool first = true;
        for (auto value : valueMap) {
            s << (first ? "'" : ", '") << value.first << "'";
            first = false;
        }
        return s.str();
    }

    bool is(lua_State* L, int idx)
    {
        return lua_isstring(L, idx) && valueMap.find(lua_tostring(L, idx)) != valueMap.end();
    }

    T check(lua_State* L, int idx)
    {
        if (!lua_isstring(L, idx)) {
            luaL_error(
                L, "Invalid %s, expected one of: %s", name.c_str(), getValuesString().c_str());
            return T();
        }
        const char* value = luaL_checklstring(L, idx, nullptr);
        auto it = valueMap.find(value);
        if (it == valueMap.end()) {
            luaL_error(L, "Invalid %s '%s': expected one of %s", name.c_str(), value,
                getValuesString().c_str());
            return T();
        } else {
            return it->second;
        }
    }

    std::string getValue(T t)
    {
        for (auto value : valueMap) {
            if (value.second == t)
                return value.first;
        }
        return "";
    }

    void push(lua_State* L, T t)
    {
        lua_pushstring(L, getValue(t).c_str());
    }
};

template <typename T>
T luax_check(lua_State* L, int index);

template <>
float luax_check<float>(lua_State* L, int index)
{
    return static_cast<float>(luaL_checknumber(L, index));
}

template <>
bool luax_check<bool>(lua_State* L, int index)
{
    luaL_checktype(L, index, LUA_TBOOLEAN);
    return lua_toboolean(L, index) != 0;
}

template <>
glm::vec2 luax_check<glm::vec2>(lua_State* L, int index)
{
    return glm::vec2(luax_check<float>(L, index + 0), luax_check<float>(L, index + 1));
}

template <>
glm::vec3 luax_check<glm::vec3>(lua_State* L, int index)
{
    return glm::vec3(luax_check<float>(L, index + 0), luax_check<float>(L, index + 1),
        luax_check<float>(L, index + 2));
}

template <>
glm::vec4 luax_check<glm::vec4>(lua_State* L, int index)
{
    return glm::vec4(luax_check<float>(L, index + 0), luax_check<float>(L, index + 1),
        luax_check<float>(L, index + 2), luax_check<float>(L, index + 3));
}

template <>
glm::mat4 luax_check<glm::mat4>(lua_State* L, int index)
{
    glm::mat4 mat;
    for (int i = 0; i < 16; ++i)
        mat[i / 4][i % 4] = luax_check<float>(L, index + i);
    return mat;
}

// checks if stack[index] is a table with num numbers and pushes them on to the stack
// true on success, false if malformed
bool luax_getnumtable(lua_State* L, int index, int num)
{
    if (index < 0)
        index = lua_gettop(L) + index + 1;
    if (lua_istable(L, index)) {
        int len = lua_objlen(L, index);
        if (len == num) {
            luaL_checkstack(L, num, "Cannot grow stack to appropriate size.");
            for (int i = 1; i <= num; ++i) {
                lua_rawgeti(L, index, i);
                if (!lua_isnumber(L, -1)) {
                    luaL_error(L, "Expected number for element #%d in table.", i);
                    lua_pop(L, i); // clean up pushed elements
                    return false;
                }
            }
            return true;
        } else {
            luaL_error(L, "Expecting table of length %d", num);
            return false;
        }
    } else {
        luaL_typerror(L, index, "table");
        return false;
    }
}

template <typename T>
T luax_checkvectable(lua_State* L, int index)
{
    int len = T::length();
    if (luax_getnumtable(L, index, len)) {
        T v = luax_check<T>(L, -len);
        lua_pop(L, len);
        return v;
    }
    return T();
}

int luax_pushvec3(lua_State* L, const glm::vec3& v)
{
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
    return 3;
}

int luax_pushvec4(lua_State* L, const glm::vec4& v)
{
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
    lua_pushnumber(L, v.w);
    return 4;
}

int luax_pushmat4(lua_State* L, const glm::mat4& mat)
{
    for (int i = 0; i < 16; ++i)
        lua_pushnumber(L, mat[i / 4][i % 4]);
    return 16;
}

std::string luax_debugStackElement(lua_State* L, int idx)
{
    const auto type = static_cast<LuaType>(lua_type(L, idx));
    switch (type) {
    case LuaType::NIL:
        return "nil"s;
    case LuaType::NUMBER:
        return "number: " + std::to_string(lua_tonumber(L, idx));
    case LuaType::BOOLEAN:
        return "boolean: " + std::to_string(lua_toboolean(L, idx) > 0);
    case LuaType::STRING:
        return "string: "s + lua_tostring(L, idx);
    case LuaType::TABLE:
        return "table: "s + kaun::sprintfFmt("%p", lua_topointer(L, idx));
    case LuaType::FUNCTION:
        return "function: "s + kaun::sprintfFmt("%p", lua_topointer(L, idx));
    case LuaType::USERDATA:
        return "userdata: "s + kaun::sprintfFmt("%p", lua_touserdata(L, idx));
    case LuaType::THREAD:
        return "thread: "s + kaun::sprintfFmt("%p", lua_topointer(L, idx));
    case LuaType::LIGHTUSERDATA:
        return "lightuserdata: "s + kaun::sprintfFmt("%p", lua_touserdata(L, idx));
    default:
        return "unknown"s;
    }
}

void luax_printStack(lua_State* L)
{
    const int count = lua_gettop(L);
    LOG_DEBUG("%d elements on the stack", count);
    for (int i = 1; i <= count; ++i) {
        LOG_DEBUG("stack element %d: %s", i, luax_debugStackElement(L, i).c_str());
    }
}
