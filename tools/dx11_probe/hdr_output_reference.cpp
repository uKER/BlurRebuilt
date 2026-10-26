// [Blur2PC][HDR] GPU checks of the production presenter, calibration and reset.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <d3d11sdklayers.h>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <DirectXPackedVector.h>
namespace Blur2PCAutomation {
void LogF(const char* format, ...) { va_list args; va_start(args, format); vprintf(format, args); puts(""); va_end(args); }
}
#include "Display/Device/D3D11/D3D11HDROutput.h"
using Microsoft::WRL::ComPtr;
using Display::D3D11::C_HDROutput;

static bool g_CheckLinkage = false; // WARP linkage fixture; hardware also covers real FX11 bindings in the game.
static float Decode(float v) { return v <= 0.04045f ? v / 12.92f : powf((v + 0.055f) / 1.055f, 2.4f); }
static bool Check(ID3D11Device* device, IDXGISwapChain* chain, C_HDROutput& output, float peak, float paper, bool hdr = true)
{
    ComPtr<ID3D11Texture2D> frame;
    const HRESULT created = output.CreateTargets(device, chain, &frame);
    if (FAILED(created)) { printf("HDR target creation failed %08x\n", (unsigned)created); return false; }
    D3D11_TEXTURE2D_DESC td = {}; frame->GetDesc(&td);
    const float inputs[] = {0, 0.001f, 0.04045f, 0.5f, 1, 1.01f, 2, 4, 16, 32};
    std::vector<unsigned short> data(td.Width * td.Height * 4);
    for (UINT y = 0; y < td.Height; ++y) for (UINT x = 0; x < td.Width; ++x)
        for (UINT c = 0; c < 4; ++c)
            data[(y * td.Width + x) * 4 + c] = DirectX::PackedVector::XMConvertFloatToHalf(c == 3 ? 1 : inputs[x % 10] * (c == 0 ? 1 : c == 1 ? 0.5f : 0.25f));
    ComPtr<ID3D11DeviceContext> context; device->GetImmediateContext(&context);
    ComPtr<ID3D11PixelShader> linkedPS; ComPtr<ID3D11ClassInstance> instance;
    if (g_CheckLinkage) {
        // Exercise FX11-style class-instance restoration, not only fixed shaders.
        const char* linkedSource = "interface IColour { float4 Get(); }; class CColour : IColour { float4 colour; float4 Get() { return colour; } }; CColour stored; IColour selected; float4 main() : SV_Target { return selected.Get(); }";
        ComPtr<ID3DBlob> bytecode, errors; ComPtr<ID3D11ClassLinkage> linkage;
        if (FAILED(D3DCompile(linkedSource, strlen(linkedSource), "HDRState", nullptr, nullptr, "main", "ps_5_0", 0, 0, &bytecode, &errors)) ||
            FAILED(device->CreateClassLinkage(&linkage)) ||
            FAILED(device->CreatePixelShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), linkage.Get(), &linkedPS)) ||
            FAILED(linkage->GetClassInstance("stored", 0, &instance))) return false;
        ComPtr<ID3D11Buffer> linkedConstants; D3D11_BUFFER_DESC linkedDesc = {}; linkedDesc.ByteWidth = 16; linkedDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        const float classColour[4] = {1,0,0,1}; D3D11_SUBRESOURCE_DATA linkedData = {}; linkedData.pSysMem = classColour;
        if (FAILED(device->CreateBuffer(&linkedDesc, &linkedData, &linkedConstants))) return false;
        ComPtr<ID3DBlob> vertexCode; ComPtr<ID3D11VertexShader> linkedVS;
        if (FAILED(D3DCompile(Display::D3D11::HDRPresentShader, strlen(Display::D3D11::HDRPresentShader), "HDRStateVS", nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vertexCode, &errors)) ||
            FAILED(device->CreateVertexShader(vertexCode->GetBufferPointer(), vertexCode->GetBufferSize(), nullptr, &linkedVS))) return false;
        context->VSSetShader(linkedVS.Get(), nullptr, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ID3D11Buffer* linkedBuffer = linkedConstants.Get(); context->PSSetConstantBuffers(0, 1, &linkedBuffer);
        ID3D11ClassInstance* boundInstance = instance.Get(); context->PSSetShader(linkedPS.Get(), &boundInstance, 1);
        ComPtr<ID3D11PixelShader> beforePS; context->PSGetShader(&beforePS, nullptr, nullptr);
        if (beforePS.Get() != linkedPS.Get()) { ComPtr<ID3D11InfoQueue> queue; device->QueryInterface(IID_PPV_ARGS(&queue));
            if (queue) for (UINT64 i = 0; i < queue->GetNumStoredMessages(); ++i) { SIZE_T size=0; queue->GetMessage(i,nullptr,&size); std::vector<char> bytes(size); auto msg=reinterpret_cast<D3D11_MESSAGE*>(bytes.data()); queue->GetMessage(i,msg,&size); printf("%s\n",msg->pDescription); }
            puts("HDR fixture shader binding invalid"); return false; }
    }
    context->UpdateSubresource(frame.Get(), 0, nullptr, data.data(), td.Width * 8, 0);
    ComPtr<ID3D11RenderTargetView> originalTarget;
    if (FAILED(device->CreateRenderTargetView(frame.Get(), nullptr, &originalTarget))) return false;
    ID3D11RenderTargetView* original = originalTarget.Get(); context->OMSetRenderTargets(1, &original, nullptr);
    // [Blur2PC][HDR] Compose must restore both material shader stages' b12.
    ComPtr<ID3D11Buffer> vertexState, pixelState;
    D3D11_BUFFER_DESC stateDesc = {}; stateDesc.ByteWidth = 16; stateDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (FAILED(device->CreateBuffer(&stateDesc, nullptr, &vertexState)) || FAILED(device->CreateBuffer(&stateDesc, nullptr, &pixelState))) return false;
    ID3D11Buffer* vertexRaw = vertexState.Get(); context->VSSetConstantBuffers(12, 1, &vertexRaw);
    ID3D11Buffer* pixelRaw = pixelState.Get(); context->PSSetConstantBuffers(12, 1, &pixelRaw);
    D3D11_VIEWPORT vp = {3, 4, 12, 13, 0, 1}; context->RSSetViewports(1, &vp);
    output.Compose(device);
    ComPtr<ID3D11Buffer> restoredVertexState, restoredPixelState;
    context->VSGetConstantBuffers(12, 1, &restoredVertexState); context->PSGetConstantBuffers(12, 1, &restoredPixelState);
    if (restoredVertexState.Get() != vertexState.Get() || restoredPixelState.Get() != pixelState.Get()) {
        puts("HDR Compose failed to restore VS/PS b12 independently"); return false;
    }
    if (g_CheckLinkage) {
        ComPtr<ID3D11PixelShader> restoredPS; ComPtr<ID3D11ClassInstance> restoredInstance; UINT restoredCount = 1;
        context->PSGetShader(&restoredPS, &restoredInstance, &restoredCount);
        if (restoredPS.Get() != linkedPS.Get() || restoredCount != 1 || restoredInstance.Get() != instance.Get())
        { printf("HDR dynamic linkage restoration failed PS=%p/%p count=%u instance=%p/%p\n", restoredPS.Get(), linkedPS.Get(), restoredCount, restoredInstance.Get(), instance.Get()); return false; }
    }
    ComPtr<ID3D11RenderTargetView> restored; context->OMGetRenderTargets(1, &restored, nullptr);
    D3D11_VIEWPORT restoredVP = {}; UINT vpCount = 1; context->RSGetViewports(&vpCount, &restoredVP);
    if (restored.Get() != original || memcmp(&vp, &restoredVP, sizeof(vp)))
    { printf("HDR state restore failed target=%p/%p viewport=%g,%g,%g,%g expected=%g,%g,%g,%g\n", restored.Get(), original, restoredVP.TopLeftX, restoredVP.TopLeftY, restoredVP.Width, restoredVP.Height, vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height); return false; }
    ComPtr<ID3D11Texture2D> back, staging;
    if (FAILED(chain->GetBuffer(0, IID_PPV_ARGS(&back)))) return false;
    back->GetDesc(&td); td.Usage = D3D11_USAGE_STAGING; td.BindFlags = td.MiscFlags = 0; td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device->CreateTexture2D(&td, nullptr, &staging))) return false;
    context->CopyResource(staging.Get(), back.Get());
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped))) return false;
    bool okay = true; float previous = -1;
    for (UINT x = 0; x < 10; ++x)
    {
        const unsigned short* pixel = static_cast<const unsigned short*>(mapped.pData) + x * 4;
        float linear[3];
        for (UINT c = 0; c < 3; ++c) linear[c] = Decode(DirectX::PackedVector::XMConvertHalfToFloat(data[x * 4 + c]));
        const float intensity = linear[0], headroom = peak / paper - 1;
        const float target = !hdr ? (std::min)(intensity, 1.0f) : intensity > 1 ? (headroom > 0 ? 1 + headroom * (1 - expf(-(intensity - 1) / headroom)) : 1) : intensity;
        for (UINT c = 0; c < 3; ++c)
        {
            const float expected = (hdr ? linear[c] * (intensity > 0 ? target / intensity : 1) : (std::min)(linear[c], 1.0f)) * paper / 80;
            const float actual = DirectX::PackedVector::XMConvertHalfToFloat(pixel[c]);
            if (!std::isfinite(actual) || fabsf(expected - actual) > (std::max)(0.002f, fabsf(expected) * 0.0015f))
            { printf("HDR mismatch x=%u channel=%u expected=%g actual=%g\n", x, c, expected, actual); okay = false; }
        }
        const float actual = DirectX::PackedVector::XMConvertHalfToFloat(pixel[0]);
        if (actual + (std::max)(0.002f, previous * 0.0015f) < previous || actual > peak / 80 + 0.05f)
        { printf("HDR monotonic/peak check failed x=%u actual=%g previous=%g peak=%g\n", x, actual, previous, peak/80); okay = false; }
        previous = actual;
    }
    context->Unmap(staging.Get(), 0);
    context->OMSetRenderTargets(0, nullptr, nullptr);
    return okay;
}
#include "hdr_scene_reference.h"
#include "hdr_range_reference.h"

int main(int argc, char** argv)
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    WNDCLASSA wc = {}; wc.lpfnWndProc = DefWindowProcA; wc.hInstance = GetModuleHandleA(nullptr); wc.lpszClassName = "BlurHDRProbe";
    RegisterClassA(&wc); HWND window = CreateWindowA(wc.lpszClassName, "Blur HDR GPU reference", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
    ShowWindow(window, SW_SHOWMINNOACTIVE);
    DXGI_SWAP_CHAIN_DESC sd = {}; sd.BufferDesc.Width = 64; sd.BufferDesc.Height = 32; sd.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    sd.SampleDesc.Count = 1; sd.BufferCount = 2; sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; sd.OutputWindow = window; sd.Windowed = TRUE; sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    ComPtr<ID3D11Device> device; ComPtr<ID3D11DeviceContext> context; ComPtr<IDXGISwapChain> chain;
    const D3D_DRIVER_TYPE driver = argc > 1 && !strcmp(argv[1], "--warp") ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE;
    g_CheckLinkage = driver == D3D_DRIVER_TYPE_WARP;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, driver, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
        nullptr, 0, D3D11_SDK_VERSION, &sd, &chain, &device, nullptr, &context);
    if (FAILED(hr)) { printf("HDR create failed %08x\n", (unsigned)hr); return 1; }
    // Report the real display BEFORE injecting diagnostic calibration.
    C_HDROutput detected(window); detected.Refresh(device.Get(), true);
    SetEnvironmentVariableA("BLUR_DX11_HDR_TEST", "1");
    const float peaks[] = {80, 200, 400, 600, 1000, 2000, 4000, 10000};
    for (float peak : peaks)
    {
        char text[32]; sprintf_s(text, "%g", peak); SetEnvironmentVariableA("BLUR_DX11_HDR_PEAK_NITS", text);
        const float paper = (std::min)(peak, 203.0f); sprintf_s(text, "%g", paper); SetEnvironmentVariableA("BLUR_DX11_HDR_PAPER_WHITE_NITS", text);
        C_HDROutput output(window);
        if (!Check(device.Get(), chain.Get(), output, peak, paper)) return 2;
        output.ReleaseTargets(); context->ClearState(); context->Flush();
        if (FAILED(chain->ResizeBuffers(2, 80, 40, DXGI_FORMAT_R16G16B16A16_FLOAT, 0))) return 3;
        if (!Check(device.Get(), chain.Get(), output, peak, paper)) return 4;
        output.ReleaseTargets(); context->ClearState(); context->Flush();
        printf("HDR GPU %g-nit peak, %g-nit paper white: black, SDR, highlights, hue, peak, state restoration and resize pass\n", peak, paper);
    }
    C_HDROutput live(window); live.Calibrate(true, 1733.33f, 251.2f);
    if (!Check(device.Get(), chain.Get(), live, 1733.33f, 251.2f)) return 6;
    live.Calibrate(false, 0, 0); live.Refresh(device.Get(), true);
    const float sdrWhite = live.PaperWhiteScale() * 80;
    if (live.Active() || !Check(device.Get(), chain.Get(), live, sdrWhite, sdrWhite, false)) return 7;
    live.ReleaseTargets(); context->ClearState(); context->Flush();
    printf("HDR live fractional calibration and disable pass; synthetic linkage=%s\n", g_CheckLinkage ? "verified" : "WARP-only");
    SetEnvironmentVariableA("BLUR_DX11_HDR", "off"); C_HDROutput disabled(window);
    if (disabled.FloatSwapChain() || disabled.Active()) return 5;
    SetEnvironmentVariableA("BLUR_DX11_HDR", "auto");
    SetEnvironmentVariableA("BLUR_DX11_HDR_TEST", nullptr); SetEnvironmentVariableA("BLUR_DX11_HDR_PEAK_NITS", "nan");
    SetEnvironmentVariableA("BLUR_DX11_HDR_PAPER_WHITE_NITS", "-1"); C_HDROutput invalid(window); invalid.Refresh(device.Get(), true);
    if (argc > 2 && !CheckSceneRange(device.Get(), chain.Get(), window, argv[2])) return 8;
    if (argc > 3 && !CheckEmissiveRange(device.Get(), chain.Get(), window, argv[3])) return 9;
    if (argc > 4 && !CheckIntermediateRange(device.Get(), chain.Get(), window, argv[2], argv[4])) return 10;
    DestroyWindow(window); puts("HDR production presenter GPU references passed"); return 0;
}
