#include "io.h"
#include <fstream>

namespace io {
   void readBin(const char* path, std::vector<char>& data) {
	std::fstream strm(path, std::ios::ate | std::ios::in | std::ios::binary);
	if (!strm.is_open()) {
        printf("Could not load file\n");
        return;
	}
	size_t size = (size_t)strm.tellg();
	data.resize(size);
	strm.seekg(0);
	strm.read(&data[0], size);
	strm.close(); 
   }
};


