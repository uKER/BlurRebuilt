#include "Display/Device/D3D11/D3D11Effect.h"
#include "Display/Device/D3D11/D3D11EffectNames.h"
#include "Display/Device/D3D11/D3D11Formats.h"

#include <cstdio>
#include <algorithm>
#include <vector>

namespace Display { namespace D3D11 {

HRESULT C_D3D11Effect::LoadMemory(ID3D11Device* device, const void* bytes, SIZE_T size)
{
    if (!device || !bytes || !size) return E_INVALIDARG;
    m_Variables.clear();
    m_VariablesComplete = false;
    m_Effect.Reset();
    const HRESULT result = D3DX11CreateEffectFromMemory(bytes, size, 0, device, &m_Effect);
    D3DX11_EFFECT_DESC effect = {};
    if (SUCCEEDED(result) && SUCCEEDED(m_Effect->GetDesc(&effect)))
    {
        m_VariablesComplete = true;
        m_Variables.reserve(effect.GlobalVariables);
        for (UINT i = 0; i < effect.GlobalVariables; ++i)
        {
            ID3DX11EffectVariable* variable = m_Effect->GetVariableByIndex(i);
            D3DX11_EFFECT_VARIABLE_DESC desc = {};
            if (variable && variable->IsValid() && SUCCEEDED(variable->GetDesc(&desc)) && desc.Name)
                m_Variables.emplace(desc.Name, variable);
            else
                m_VariablesComplete = false;
        }
    }
    return result;
}

ID3DX11EffectVariable* C_D3D11Effect::FindVariable(const char* name) const
{
    if (!m_Effect || !name) return nullptr;
    name = NativeEffectParameterName(name);
    const auto found = m_Variables.find(name);
    if (found != m_Variables.end()) return found->second;
    return m_VariablesComplete ? nullptr : m_Effect->GetVariableByName(name);
}

HRESULT C_D3D11Effect::LoadFile(ID3D11Device* device, const wchar_t* path)
{
    if (!path) return E_INVALIDARG;
    FILE* file = nullptr;
    if (_wfopen_s(&file, path, L"rb") != 0 || !file) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    if (std::fseek(file, 0, SEEK_END) != 0) { std::fclose(file); return E_FAIL; }
    const long length = std::ftell(file);
    if (length <= 0 || std::fseek(file, 0, SEEK_SET) != 0)
    { std::fclose(file); return E_FAIL; }
    std::vector<unsigned char> data(static_cast<size_t>(length));
    const size_t read = std::fread(data.data(), 1, data.size(), file);
    std::fclose(file);
    if (read != data.size()) return E_FAIL;
    return LoadMemory(device, data.data(), data.size());
}

bool C_D3D11Effect::HasParameter(const char* name) const
{
    if (!m_Effect || !name) return false;
    ID3DX11EffectVariable* variable = FindVariable(name);
    return variable && variable->IsValid();
}

HRESULT C_D3D11Effect::SetRawParameter(const char* name, const void* data, UINT bytes)
{
    if (!m_Effect || !name || !data || !bytes) return E_INVALIDARG;
    ID3DX11EffectVariable* variable = FindVariable(name);
    if (!variable || !variable->IsValid()) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    D3DX11_EFFECT_TYPE_DESC type = {};
    if (FAILED(variable->GetType()->GetDesc(&type))) return E_FAIL;
    return variable->SetRawValue(data, 0, bytes < type.UnpackedSize ? bytes : type.UnpackedSize);
}

HRESULT C_D3D11Effect::SetTexture(const char* name, ID3D11ShaderResourceView* view)
{
    if (!m_Effect || !name) return E_INVALIDARG;
    ID3DX11EffectVariable* variable = FindVariable(name);
    if (!variable || !variable->IsValid()) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    ID3DX11EffectShaderResourceVariable* resource = variable->AsShaderResource();
    if (!resource || !resource->IsValid()) return E_INVALIDARG;
    if (view)
    {
        D3DX11_EFFECT_TYPE_DESC type = {};
        D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
        view->GetDesc(&desc);
        // [Blur1PCDX11] PC cube sampling a bound 2D texture returns RGBA zero
        // (including nonzero texels). An unbound native SRV preserves that
        // measured result without the D3D11 draw-time dimension error.
        if (SUCCEEDED(variable->GetType()->GetDesc(&type)) &&
            type.Type == D3D_SVT_TEXTURECUBE && desc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D)
            view = nullptr;
    }
    return resource->SetResource(view);
}

HRESULT C_D3D11Effect::CreateTextureView(const char* name, ID3D11Device* device,
                                      ID3D11Resource* texture, bool srgb,
                                      ID3D11ShaderResourceView** view) const
{
    if (!view) return E_INVALIDARG;
    *view = nullptr;
    if (!m_Effect || !name) return E_INVALIDARG;
    ID3DX11EffectVariable* variable = FindVariable(name);
    if (!variable || !variable->IsValid()) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    ID3DX11EffectShaderResourceVariable* resource = variable->AsShaderResource();
    if (!resource || !resource->IsValid()) return E_INVALIDARG;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> previous;
    // GetResourceArray supports an initially unbound scalar slot as well.
    const HRESULT result = resource->GetResourceArray(&previous, 0, 1);
    if (FAILED(result)) return result;
    return Display::D3D11::CreateTextureView(device, texture, srgb, view, previous.Get());
}

HRESULT C_D3D11Effect::SetSampler(const char* name, ID3D11SamplerState* sampler)
{
    if (!m_Effect || !name) return E_INVALIDARG;
    ID3DX11EffectVariable* variable = FindVariable(name);
    if (!variable || !variable->IsValid()) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    ID3DX11EffectSamplerVariable* state = variable->AsSampler();
    if (!state || !state->IsValid()) return E_INVALIDARG;
    return state->SetSampler(0, sampler);
}

ID3DX11EffectPass* C_D3D11Effect::FindPass(const char* technique, UINT pass) const
{
    if (!m_Effect || !technique) return nullptr;
    ID3DX11EffectTechnique* found = m_Effect->GetTechniqueByName(technique);
    if (!found || !found->IsValid()) return nullptr;
    D3DX11_TECHNIQUE_DESC desc = {};
    if (FAILED(found->GetDesc(&desc)) || pass >= desc.Passes) return nullptr;
    ID3DX11EffectPass* result = found->GetPassByIndex(pass);
    return result && result->IsValid() ? result : nullptr;
}

HRESULT C_D3D11Effect::GetPassInputSignature(const char* technique, UINT pass,
                                              const void** signature, SIZE_T* bytes) const
{
    if (!signature || !bytes) return E_INVALIDARG;
    *signature = nullptr;
    *bytes = 0;
    ID3DX11EffectPass* found = FindPass(technique, pass);
    if (!found) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    D3DX11_PASS_DESC desc = {};
    HRESULT result = found->GetDesc(&desc);
    if (FAILED(result)) return result;
    *signature = desc.pIAInputSignature;
    *bytes = desc.IAInputSignatureSize;
    return S_OK;
}

HRESULT C_D3D11Effect::GetPassPixelShaderBytecode(const char* technique, UINT pass,
                                                   const void** bytecode, SIZE_T* bytes) const
{
    if (!bytecode || !bytes) return E_INVALIDARG;
    *bytecode = nullptr;
    *bytes = 0;
    ID3DX11EffectPass* found = FindPass(technique, pass);
    if (!found) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    D3DX11_PASS_SHADER_DESC passShader = {};
    HRESULT result = found->GetPixelShaderDesc(&passShader);
    if (FAILED(result) || !passShader.pShaderVariable) return E_FAIL;
    D3DX11_EFFECT_SHADER_DESC shader = {};
    result = passShader.pShaderVariable->GetShaderDesc(passShader.ShaderIndex, &shader);
    if (FAILED(result)) return result;
    *bytecode = shader.pBytecode;
    *bytes = shader.BytecodeLength;
    return *bytecode && *bytes ? S_OK : E_FAIL;
}

HRESULT C_D3D11Effect::GetPassVertexShaderBytecode(const char* technique, UINT pass,
                                                    const void** bytecode, SIZE_T* bytes) const
{
    if (!bytecode || !bytes) return E_INVALIDARG;
    *bytecode = nullptr;
    *bytes = 0;
    ID3DX11EffectPass* found = FindPass(technique, pass);
    if (!found) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    D3DX11_PASS_SHADER_DESC passShader = {};
    HRESULT result = found->GetVertexShaderDesc(&passShader);
    if (FAILED(result) || !passShader.pShaderVariable) return E_FAIL;
    D3DX11_EFFECT_SHADER_DESC shader = {};
    result = passShader.pShaderVariable->GetShaderDesc(passShader.ShaderIndex, &shader);
    if (FAILED(result)) return result;
    *bytecode = shader.pBytecode;
    *bytes = shader.BytecodeLength;
    return *bytecode && *bytes ? S_OK : E_FAIL;
}

HRESULT C_D3D11Effect::Apply(const char* technique, UINT pass,
                              ID3D11DeviceContext* context) const
{
    if (!context) return E_INVALIDARG;
    ID3DX11EffectPass* found = FindPass(technique, pass);
    return found ? found->Apply(0, context) : HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

} }
