#include <fstream>
#include <string>
#include <cerrno>

#include "utility.hpp"
#include "log.hpp"

namespace kaun {
	// http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html// http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
    tl::expected<std::string, ErrorTuple> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::in);
		if(file) {
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
}
