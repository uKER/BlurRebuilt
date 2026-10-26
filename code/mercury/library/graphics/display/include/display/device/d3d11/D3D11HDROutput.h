// [Blur2PC][HDR] Display-aware scRGB presentation; no simulation state.
#pragma once
#include <d3d11.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include "D3D11HDRShaders.h"

namespace Display { namespace D3D11 {
class C_HDROutput
{
    template<class T> using Ptr = Microsoft::WRL::ComPtr<T>;
    Ptr<ID3D11Texture2D> m_Frame;
    Ptr<ID3D11ShaderResourceView> m_FrameView;
    Ptr<ID3D11RenderTargetView> m_Output;
    Ptr<ID3D11VertexShader> m_VS;
    Ptr<ID3D11PixelShader> m_PS;
    Ptr<ID3D11Buffer> m_Constants;
    Ptr<ID3D11RasterizerState> m_Raster;
    Ptr<ID3D11DepthStencilState> m_Depth;
    bool m_Float, m_Active, m_Test;
    HWND m_Window;
    FILE* m_Log = nullptr; // executable-local status for ordinary play.bat launches
    ULONGLONG m_NextCheck;
    float m_Values[4]; // active, paper-white/80, peak/paper-white, exposure
    float m_PeakOverride, m_WhiteOverride;
    HMONITOR m_Monitor;
    bool m_Enabled = true;
    float m_LivePeak = 0, m_LiveWhite = 0;

    static float Setting(const char* name)
    {
        char value[64] = {}; char* end = nullptr;
        if (!GetEnvironmentVariableA(name, value, sizeof(value))) return 0;
        const float parsed = strtof(value, &end);
        return end != value && *end == '\0' && std::isfinite(parsed) && parsed > 0 ? parsed : 0;
    }
    static float SDRWhite(const DXGI_OUTPUT_DESC1& output)
    {
        UINT paths = 0, modes = 0;
        if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &paths, &modes) != ERROR_SUCCESS) return 80;
        std::vector<DISPLAYCONFIG_PATH_INFO> pathInfo(paths);
        std::vector<DISPLAYCONFIG_MODE_INFO> modeInfo(modes);
        if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &paths, pathInfo.data(), &modes, modeInfo.data(), nullptr) != ERROR_SUCCESS) return 80;
        for (UINT i = 0; i < paths; ++i)
        {
            DISPLAYCONFIG_SOURCE_DEVICE_NAME source = {};
            source.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
            source.header.size = sizeof(source); source.header.adapterId = pathInfo[i].sourceInfo.adapterId;
            source.header.id = pathInfo[i].sourceInfo.id;
            if (DisplayConfigGetDeviceInfo(&source.header) != ERROR_SUCCESS || wcscmp(source.viewGdiDeviceName, output.DeviceName)) continue;
            DISPLAYCONFIG_SDR_WHITE_LEVEL white = {};
            white.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL;
            white.header.size = sizeof(white); white.header.adapterId = pathInfo[i].targetInfo.adapterId;
            white.header.id = pathInfo[i].targetInfo.id;
            if (DisplayConfigGetDeviceInfo(&white.header) == ERROR_SUCCESS && white.SDRWhiteLevel)
                return 80.0f * white.SDRWhiteLevel / 1000.0f;
        }
        return 80; // documented nominal SDR white when the OS query is unavailable
    }
public:
    explicit C_HDROutput(HWND window) : m_Float(true), m_Active(false), m_Window(window), m_NextCheck(0), m_Monitor(nullptr)
    {
        // [Blur2PC][HDR] Automation logging is disabled in normal play.
        // Keep the selected display/calibration beside the actual executable.
        wchar_t path[MAX_PATH] = {};
        const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
        wchar_t* name = length && length < MAX_PATH ? wcsrchr(path, L'\\') : nullptr;
        if (name && (name - path) + 16 < MAX_PATH)
        {
            wcscpy_s(name + 1, MAX_PATH - (name + 1 - path), L"hdr_output.log");
            _wfopen_s(&m_Log, path, L"w");
        }
        char mode[16] = {}; GetEnvironmentVariableA("BLUR_DX11_HDR", mode, sizeof(mode));
        m_Float = _stricmp(mode, "off") != 0 && strcmp(mode, "0") != 0;
        m_Test = Setting("BLUR_DX11_HDR_TEST") == 1;
        m_PeakOverride = Setting("BLUR_DX11_HDR_PEAK_NITS");
        m_WhiteOverride = Setting("BLUR_DX11_HDR_PAPER_WHITE_NITS");
        m_Values[0] = 0; m_Values[1] = 1; m_Values[2] = 1; m_Values[3] = 1;
    }
    ~C_HDROutput() { if (m_Log) fclose(m_Log); }
    bool FloatSwapChain() const { return m_Float; }
    bool Active() const { return m_Active; }
    float PaperWhiteScale() const { return m_Values[1]; }
    void Calibrate(bool enabled, float peak, float white)
    {
        peak = std::isfinite(peak) && peak > 0 ? peak : 0;
        white = std::isfinite(white) && white > 0 ? white : 0;
        if (enabled != m_Enabled || peak != m_LivePeak || white != m_LiveWhite) m_NextCheck = 0;
        m_Enabled = enabled; m_LivePeak = peak; m_LiveWhite = white;
    }
    void FallBack() { m_Float = m_Active = false; m_Values[0] = 0; ReleaseTargets(); }
    void ReleaseTargets() { m_Output.Reset(); m_FrameView.Reset(); m_Frame.Reset(); }
    void Refresh(ID3D11Device* device, bool force = false)
    {
        const ULONGLONG now = GetTickCount64();
        const HMONITOR monitor = MonitorFromWindow(m_Window, MONITOR_DEFAULTTONEAREST);
        if (!force && now < m_NextCheck && monitor == m_Monitor) return;
        m_NextCheck = now + 1000; m_Monitor = monitor;
        DXGI_OUTPUT_DESC1 desc = {}; bool found = false;
        // Use the actual window's monitor, including displays on a second GPU.
        Ptr<IDXGIFactory1> factory;
        if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
            for (UINT a = 0; !found; ++a)
            {
                Ptr<IDXGIAdapter1> adapter;
                if (factory->EnumAdapters1(a, &adapter) == DXGI_ERROR_NOT_FOUND) break;
                if (!adapter) break;
                for (UINT o = 0; !found; ++o)
                {
                    Ptr<IDXGIOutput> output; Ptr<IDXGIOutput6> advanced;
                    if (adapter->EnumOutputs(o, &output) == DXGI_ERROR_NOT_FOUND) break;
                    if (!output) break;
                    if (SUCCEEDED(output.As(&advanced)) && SUCCEEDED(advanced->GetDesc1(&desc)) && desc.Monitor == monitor) found = true;
                }
            }
        const bool displayHDR = found && desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
        const float white = m_LiveWhite > 0 ? m_LiveWhite : m_WhiteOverride > 0 ? m_WhiteOverride : displayHDR ? SDRWhite(desc) : 80;
        const float peak = m_LivePeak > 0 ? m_LivePeak : m_PeakOverride > 0 ? m_PeakOverride : found ? desc.MaxLuminance : 0;
        const bool active = m_Enabled && m_Float && (displayHDR || m_Test) && std::isfinite(peak) && peak > 0;
        const float paper = active ? (std::min)(white, peak) : displayHDR ? SDRWhite(desc) : 80;
        const float values[4] = {active ? 1.0f : 0.0f, paper / 80, active ? peak / paper : 1.0f, 1.0f};
        if (force || memcmp(values, m_Values, sizeof(values)))
        {
            char status[512] = {};
            sprintf_s(status, "HDR output active=%u displayHDR=%u test=%u float=%u peak=%.2f paperWhite=%.2f fullFrame=%.2f min=%.4f colourSpace=%u monitor=%p",
                active, displayHDR, m_Test, m_Float, peak, paper, desc.MaxFullFrameLuminance, desc.MinLuminance, desc.ColorSpace, monitor);
            Blur2PCAutomation::LogF("%s", status);
            if (m_Log) { fprintf(m_Log, "%s\n", status); fflush(m_Log); }
        }
        m_Active = active; memcpy(m_Values, values, sizeof(values));
        if (m_Constants)
        {
            Ptr<ID3D11DeviceContext> context; device->GetImmediateContext(&context);
            context->UpdateSubresource(m_Constants.Get(), 0, nullptr, m_Values, 0, 0);
        }
    }
    HRESULT CreateTargets(ID3D11Device* device, IDXGISwapChain* chain, ID3D11Texture2D** frame)
    {
        ReleaseTargets();
        HRESULT hr = S_OK;
        if (!m_Constants)
        {
            D3D11_BUFFER_DESC cb = {}; cb.ByteWidth = sizeof(m_Values); cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            D3D11_SUBRESOURCE_DATA initial = {}; initial.pSysMem = m_Values;
            if (FAILED(hr = device->CreateBuffer(&cb, &initial, &m_Constants))) return hr;
        }
        if (!m_Float)
        {
            Ptr<IDXGISwapChain3> sdr;
            if (SUCCEEDED(chain->QueryInterface(IID_PPV_ARGS(&sdr)))) sdr->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);
            return chain->GetBuffer(0, IID_PPV_ARGS(frame));
        }
        Ptr<IDXGISwapChain3> advanced;
        hr = chain->QueryInterface(IID_PPV_ARGS(&advanced));
        UINT support = 0;
        if (FAILED(hr) || FAILED(hr = advanced->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709, &support))) return hr;
        if (!(support & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)) return DXGI_ERROR_UNSUPPORTED;
        if (FAILED(hr = advanced->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709))) return hr;
        Ptr<ID3D11Texture2D> back;
        if (FAILED(hr = chain->GetBuffer(0, IID_PPV_ARGS(&back)))) return hr;
        if (FAILED(hr = device->CreateRenderTargetView(back.Get(), nullptr, &m_Output))) return hr;
        D3D11_TEXTURE2D_DESC desc = {}; back->GetDesc(&desc);
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(hr = device->CreateTexture2D(&desc, nullptr, &m_Frame))) return hr;
        if (FAILED(hr = device->CreateShaderResourceView(m_Frame.Get(), nullptr, &m_FrameView))) return hr;
        if (!m_VS)
        {
            Ptr<ID3DBlob> vs, ps, errors;
            if (FAILED(hr = D3DCompile(HDRPresentShader, strlen(HDRPresentShader), "HDRPresent", nullptr, nullptr, "VS", "vs_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &vs, &errors))) return hr;
            if (FAILED(hr = D3DCompile(HDRPresentShader, strlen(HDRPresentShader), "HDRPresent", nullptr, nullptr, "PS", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &ps, &errors))) return hr;
            if (FAILED(hr = device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &m_VS))) return hr;
            if (FAILED(hr = device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &m_PS))) return hr;
            D3D11_RASTERIZER_DESC raster = {}; raster.FillMode = D3D11_FILL_SOLID; raster.CullMode = D3D11_CULL_NONE; raster.DepthClipEnable = TRUE;
            if (FAILED(hr = device->CreateRasterizerState(&raster, &m_Raster))) return hr;
            D3D11_DEPTH_STENCIL_DESC depth = {}; depth.DepthFunc = D3D11_COMPARISON_ALWAYS;
            if (FAILED(hr = device->CreateDepthStencilState(&depth, &m_Depth))) return hr;
        }
        Refresh(device, true);
        *frame = m_Frame.Get(); (*frame)->AddRef();
        return S_OK;
    }
    void Bind(ID3D11DeviceContext* context)
    {
        ID3D11Buffer* buffer = m_Constants.Get(); context->PSSetConstantBuffers(12, 1, &buffer);
        context->VSSetConstantBuffers(12, 1, &buffer); // [Blur2PC][HDR] Vehicle SH is evaluated in VS.
    }
    void Compose(ID3D11Device* device)
    {
        if (!m_Float || !m_Output || !m_Frame) return;
        Ptr<ID3D11DeviceContext> context; device->GetImmediateContext(&context);
        Ptr<ID3D11VertexShader> vs; Ptr<ID3D11PixelShader> ps; Ptr<ID3D11GeometryShader> gs;
        Ptr<ID3D11InputLayout> layout; Ptr<ID3D11RasterizerState> raster;
        Ptr<ID3D11BlendState> blend; Ptr<ID3D11DepthStencilState> depth;
        Ptr<ID3D11ShaderResourceView> view; Ptr<ID3D11Buffer> constants, vertexConstants;
        ID3D11RenderTargetView* targets[8] = {}; ID3D11DepthStencilView* dsv = nullptr;
        UINT stencil = 0, mask = 0; FLOAT factors[4] = {};
        D3D11_PRIMITIVE_TOPOLOGY topology; D3D11_VIEWPORT ports[16]; UINT portCount = 16;
        context->OMGetRenderTargets(8, targets, &dsv);
        // FX11 uses dynamic linkage: retain each shader's class instances too.
        ID3D11ClassInstance* vsClasses[256] = {}; ID3D11ClassInstance* psClasses[256] = {}; ID3D11ClassInstance* gsClasses[256] = {};
        UINT vsCount = 0, psCount = 0, gsCount = 0;
        context->VSGetShader(&vs, nullptr, &vsCount); context->PSGetShader(&ps, nullptr, &psCount); context->GSGetShader(&gs, nullptr, &gsCount);
        context->VSGetShader(&vs, vsClasses, &vsCount); context->PSGetShader(&ps, psClasses, &psCount); context->GSGetShader(&gs, gsClasses, &gsCount);
        context->IAGetInputLayout(&layout); context->IAGetPrimitiveTopology(&topology);
        context->RSGetState(&raster); context->RSGetViewports(&portCount, ports);
        context->OMGetBlendState(&blend, factors, &mask); context->OMGetDepthStencilState(&depth, &stencil);
        context->PSGetShaderResources(0, 1, &view); context->PSGetConstantBuffers(12, 1, &constants);
        context->VSGetConstantBuffers(12, 1, &vertexConstants); // [Blur2PC][HDR]
        // Drop the frame's RTV before reading it; output uses the actual swap-chain buffer.
        ID3D11RenderTargetView* output = m_Output.Get(); context->OMSetRenderTargets(1, &output, nullptr);
        D3D11_TEXTURE2D_DESC desc = {}; m_Frame->GetDesc(&desc);
        D3D11_VIEWPORT viewport = {0, 0, static_cast<float>(desc.Width), static_cast<float>(desc.Height), 0, 1};
        context->RSSetViewports(1, &viewport); context->RSSetState(m_Raster.Get());
        context->OMSetBlendState(nullptr, nullptr, ~0U); context->OMSetDepthStencilState(m_Depth.Get(), 0);
        context->IASetInputLayout(nullptr); context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(m_VS.Get(), nullptr, 0); context->PSSetShader(m_PS.Get(), nullptr, 0); context->GSSetShader(nullptr, nullptr, 0);
        Bind(context.Get()); ID3D11ShaderResourceView* frame = m_FrameView.Get(); context->PSSetShaderResources(0, 1, &frame);
        context->Draw(3, 0);
        ID3D11ShaderResourceView* oldView = view.Get(); context->PSSetShaderResources(0, 1, &oldView);
        ID3D11Buffer* oldConstants = constants.Get(); context->PSSetConstantBuffers(12, 1, &oldConstants);
        ID3D11Buffer* oldVertexConstants = vertexConstants.Get(); context->VSSetConstantBuffers(12, 1, &oldVertexConstants); // [Blur2PC][HDR]
        context->VSSetShader(vs.Get(), vsClasses, vsCount); context->PSSetShader(ps.Get(), psClasses, psCount); context->GSSetShader(gs.Get(), gsClasses, gsCount);
        for (UINT i = 0; i < vsCount; ++i) if (vsClasses[i]) vsClasses[i]->Release();
        for (UINT i = 0; i < psCount; ++i) if (psClasses[i]) psClasses[i]->Release();
        for (UINT i = 0; i < gsCount; ++i) if (gsClasses[i]) gsClasses[i]->Release();
        context->IASetInputLayout(layout.Get()); context->IASetPrimitiveTopology(topology);
        context->RSSetState(raster.Get()); context->RSSetViewports(portCount, ports);
        context->OMSetBlendState(blend.Get(), factors, mask); context->OMSetDepthStencilState(depth.Get(), stencil);
        context->OMSetRenderTargets(8, targets, dsv);
        for (auto target : targets) if (target) target->Release(); if (dsv) dsv->Release();
    }
};
}}
