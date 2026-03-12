#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <format>
#include <stdio.h>
#include <dlfcn.h>

#include "lib_recomp.hpp"

namespace fs = std::filesystem;

class ExtlibWrapper {
public:
	ExtlibWrapper(fs::path dlpath);
	~ExtlibWrapper();


	RecompExtlibFunction lookup_function(std::string name);
	bool call_function(std::string name, uint8_t* rdram, recomp_context* ctx);
	
private:
	fs::path _path;
	void* _handle = nullptr;
	// std::unordered_map<std::string, RecompExtlibFunction> functions;

	template <typename T>
	inline bool get_dll_symbol(T& out, const char* name) const {
		out = (T)dlsym(_handle, name);
		if (out == nullptr) {
        		return false;
		}
		return true;
	}
};

