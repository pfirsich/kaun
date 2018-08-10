template <typename T>
T luax_check(lua_State* L, int index);

template <>
float luax_check<float>(lua_State* L, int index) {
    return static_cast<float>(luaL_checknumber(L, index));
}

template <>
glm::vec2 luax_check<glm::vec2>(lua_State* L, int index) {
    return glm::vec2(luax_check<float>(L, index + 0), luax_check<float>(L, index + 1));
}

template <>
glm::vec3 luax_check<glm::vec3>(lua_State* L, int index) {
    return glm::vec3(luax_check<float>(L, index + 0), luax_check<float>(L, index + 1),
                     luax_check<float>(L, index + 2));
}

template <>
glm::vec4 luax_check<glm::vec4>(lua_State* L, int index) {
    return glm::vec4(luax_check<float>(L, index + 0), luax_check<float>(L, index + 1),
                     luax_check<float>(L, index + 2), luax_check<float>(L, index + 3));
}

// checks if stack[index] is a table with num numbers and pushes them on to the stack
// true on success, false if malformed
bool luax_getnumtable(lua_State* L, int index, int num) {
    if(index < 0) index = lua_gettop(L) + index + 1;
    if(lua_istable(L, index)) {
        int len = lua_objlen(L, index);
        if(len == num) {
            luaL_checkstack(L, num, "Cannot grow stack to appropriate size.");
            for(int i = 1; i <= num; ++i) {
                lua_rawgeti(L, index, i);
                if(!lua_isnumber(L, -1)) {
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
T luax_checkvectable(lua_State* L, int index) {
    int len = T::length();
    if(luax_getnumtable(L, index, len)) {
        T v = luax_check<T>(L, -len);
        lua_pop(L, len);
        return v;
    }
    return T();
}

void luax_pushvec3(lua_State* L, const glm::vec3& v) {
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
}

void luax_pushvec4(lua_State* L, const glm::vec4& v) {
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
    lua_pushnumber(L, v.w);
}
