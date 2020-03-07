#include <cerrno>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include "log.hpp"
#include "utility.hpp"

namespace kaun {
// http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
tl::expected<std::string, ErrorTuple> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::in);
    if (file) {
        std::string contents;
        file.seekg(0, std::ios::end);
        contents.resize(static_cast<uint32_t>(file.tellg()));
        file.seekg(0, std::ios::beg);
        file.read(&contents[0], contents.size());
        file.close();
        return contents;
    } else {
        return tl::make_unexpected<ErrorTuple>(ErrorTuple(errno));
    }
}

std::string toHexStream(const uint8_t* buffer, size_t size)
{
    std::stringstream ss;
    for (size_t i = 0; i < size; ++i) {
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buffer[i]);
        if (i < size - 1)
            ss << " ";
    }
    return ss.str();
}
}
