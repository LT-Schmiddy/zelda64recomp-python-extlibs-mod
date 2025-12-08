import modbuildcore as mbc

clang_entry = mbc.downloads.DownloadArchiveEntry(
    "https://github.com/LT-Schmiddy/n64recomp-clang/releases/download/shim-prerelease-0.1.0/N64RecompAndClangEssentials-ClangVersion21.1.6-MipsOnly-Windows-AMD64.zip",
    "./compilers/clangmips"
)

clang_handler = mbc.downloads.DownloadArchiveHandler(clang_entry)
clang_handler.run()

zig_entry = mbc.downloads.DownloadArchiveEntry(
    "https://ziglang.org/download/0.14.1/zig-x86_64-windows-0.14.1.zip",
    "./compilers/zig"
)

zig_handler = mbc.downloads.DownloadArchiveHandler(zig_entry)
zig_handler.run()