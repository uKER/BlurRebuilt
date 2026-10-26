// [Blur2PC][Blur1PCDX11] Compiles shader entry points with the D3D9-era HLSL
// compiler that built the retail PC effects (D3DCompiler_42, 9.27.952.3022),
// using the PC exporter's options (pcfx.cpp: fxc /Zpr /T<profile> /E<entry>).
// Must be built for x86: the legacy compiler DLLs exist only as 32-bit.
//
// Usage: legacy_compile <jobs.txt>
// Each job line: <preprocessed source>\t<entry>\t<profile>\t<output .bin>
// Writes <output>.log with compiler messages when a job fails.
#include <windows.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct ID3DBlobLegacy : IUnknown
{
    virtual LPVOID STDMETHODCALLTYPE GetBufferPointer() = 0;
    virtual SIZE_T STDMETHODCALLTYPE GetBufferSize() = 0;
};
typedef HRESULT(WINAPI* CompileFn)(LPCVOID, SIZE_T, LPCSTR, const void*, void*, LPCSTR, LPCSTR,
                                    UINT, UINT, ID3DBlobLegacy**, ID3DBlobLegacy**);
constexpr UINT PackMatrixRowMajor = 1u << 3;

static std::vector<std::string> Split(const std::string& line)
{
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, '\t')) fields.push_back(field);
    return fields;
}

int main(int argc, char** argv)
{
    if (argc != 2) { fprintf(stderr, "usage: legacy_compile <jobs.txt>\n"); return 2; }
    HMODULE module = LoadLibraryA("D3DCompiler_42.dll");
    CompileFn compile = module ? reinterpret_cast<CompileFn>(GetProcAddress(module, "D3DCompile")) : nullptr;
    if (!compile) { fprintf(stderr, "D3DCompiler_42.dll D3DCompile unavailable\n"); return 3; }
    std::ifstream jobs(argv[1]);
    std::string line, cachedPath, cachedSource;
    int failures = 0, total = 0;
    while (std::getline(jobs, line))
    {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const std::vector<std::string> job = Split(line);
        if (job.size() != 4) continue;
        ++total;
        if (job[0] != cachedPath)
        {
            std::ifstream source(job[0], std::ios::binary);
            cachedSource.assign(std::istreambuf_iterator<char>(source), std::istreambuf_iterator<char>());
            cachedPath = job[0];
        }
        ID3DBlobLegacy* code = nullptr;
        ID3DBlobLegacy* errors = nullptr;
        const HRESULT hr = compile(cachedSource.data(), cachedSource.size(), job[0].c_str(), nullptr, nullptr,
                                   job[1].c_str(), job[2].c_str(), PackMatrixRowMajor, 0, &code, &errors);
        if (SUCCEEDED(hr) && code)
        {
            std::ofstream out(job[3], std::ios::binary);
            out.write(static_cast<const char*>(code->GetBufferPointer()), code->GetBufferSize());
        }
        else
        {
            ++failures;
            std::ofstream log(job[3] + ".log", std::ios::binary);
            if (errors) log.write(static_cast<const char*>(errors->GetBufferPointer()), errors->GetBufferSize());
            else log << "D3DCompile failed " << std::hex << hr << "\n";
        }
        if (code) code->Release();
        if (errors) errors->Release();
    }
    printf("legacy_compile: %d jobs, %d failed\n", total, failures);
    return failures ? 1 : 0;
}
