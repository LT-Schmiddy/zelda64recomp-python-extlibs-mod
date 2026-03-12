#include "extlib_wrapper.hpp"

ExtlibWrapper::ExtlibWrapper(fs::path dlpath) {
	_path = fs::absolute(dlpath);
	std::string path_str = dlpath.string();
	std::cout << path_str << "\n";
	_handle = dlopen(path_str.c_str(), RTLD_NOW | RTLD_LOCAL);
	std::cout << "Extlib Loaded from " << path_str << "\n";

	uint32_t* recomp_api_version = 0;
	get_dll_symbol(recomp_api_version, "recomp_api_version");
	std::cout << "recomp_api_version: " << *recomp_api_version << "\n";		

}

ExtlibWrapper::~ExtlibWrapper() {
	dlclose(_handle);
	_handle = nullptr;
	std::cout << "Extlib unloaded\n";

}

RecompExtlibFunction ExtlibWrapper::lookup_function(std::string name) {
	// non-cached lookup:
	RecompExtlibFunction retVal = nullptr;
	bool found = get_dll_symbol(retVal, name.c_str());
	if (found) {
		return retVal;	
	} else {
		return nullptr;
	}
}

bool ExtlibWrapper::call_function(std::string name, uint8_t* rdram, recomp_context* ctx) {
	RecompExtlibFunction func = lookup_function(name);
	if (func == nullptr) {
		return false;
	}

	func(rdram, ctx);
	return true;
}
