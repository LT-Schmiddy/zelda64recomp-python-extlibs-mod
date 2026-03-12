#include <iostream>
#include <string>
#include <thread>

#include "extlib_wrapper.hpp"

#define RDRAM_SIZE 512 * 1024 * 1024

ExtlibWrapper* extlib = NULL;

uint8_t rdram[RDRAM_SIZE]; // The memoryspace is global.
thread_local recomp_context ctx; // But we use thread-local contexts.


int main(int argc, const char** argv) {
	std::cout << "Hello Extlib";
	if (argc < 3 ) {
		std::cerr << "Err: 2 arguments required:\n NativeTest exlib_path mod_dir_path\n";
		return -1;
	}
	
	// Load the extlib:
	fs::path dlpath(argv[1]);
	extlib = new ExtlibWrapper(dlpath);
	fs::path test_path = fs::absolute(fs::path(argv[2]));

	// Initialize the Memory Space:
	printf("%08X\n", RDRAM_FROM_PTR(rdram, &rdram[10]));
	
	_set_arg<0, uint32_t>(rdram, &ctx, 5);
	_set_arg<1, uint32_t>(rdram, &ctx, 1);
	_set_arg_u8string<3>(rdram, &ctx, FROM_PTR(&rdram[0]), test_path.u8string());
	std::u8string arg3 = _arg_u8string<3>(rdram, &ctx);
	printf("%s\n", &rdram);
	printf("%s\n", arg3.c_str());
	extlib->call_function("PythonNative_Init", rdram, &ctx);
	printf("Can't believe this worked\n");

	// Cleanup:
	delete extlib;
	return 0;
}
