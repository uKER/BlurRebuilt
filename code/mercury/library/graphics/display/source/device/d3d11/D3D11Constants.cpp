#include "Display/Device/D3D11/D3D11Constants.h"

#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <cstring>

namespace Display { namespace D3D11 {

HRESULT C_ShaderConstants::Initialise(ID3D11Device* device, const void* shader, size_t shaderBytes)
{
    if (!device || !shader || !shaderBytes) return E_INVALIDARG;
    m_Buffers.clear();
    m_Bindings.clear();
    Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflection;
    HRESULT result = D3DReflect(shader, shaderBytes, IID_PPV_ARGS(&reflection));
    if (FAILED(result)) return result;
    D3D11_SHADER_DESC shaderDesc = {};
    result = reflection->GetDesc(&shaderDesc);
    if (FAILED(result)) return result;

    for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i)
    {
        ID3D11ShaderReflectionConstantBuffer* reflected = reflection->GetConstantBufferByIndex(i);
        D3D11_SHADER_BUFFER_DESC bufferDesc = {};
        result = reflected->GetDesc(&bufferDesc);
        if (FAILED(result)) return result;
        D3D11_SHADER_INPUT_BIND_DESC bind = {};
        result = reflection->GetResourceBindingDescByName(bufferDesc.Name, &bind);
        if (FAILED(result) || bind.Type != D3D_SIT_CBUFFER) return E_FAIL;

        C_Buffer buffer = {};
        buffer.m_Slot = bind.BindPoint;
        buffer.m_Dirty = true;
        buffer.m_Data.resize((bufferDesc.Size + 15u) & ~15u, 0);
        D3D11_BUFFER_DESC gpuDesc = {};
        gpuDesc.ByteWidth = static_cast<UINT>(buffer.m_Data.size());
        gpuDesc.Usage = D3D11_USAGE_DYNAMIC;
        gpuDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        gpuDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        result = device->CreateBuffer(&gpuDesc, nullptr, &buffer.m_GPU);
        if (FAILED(result)) return result;

        const UINT bufferIndex = static_cast<UINT>(m_Buffers.size());
        for (UINT j = 0; j < bufferDesc.Variables; ++j)
        {
            D3D11_SHADER_VARIABLE_DESC variable = {};
            result = reflected->GetVariableByIndex(j)->GetDesc(&variable);
            if (FAILED(result)) return result;
            if (variable.StartOffset + variable.Size > buffer.m_Data.size() ||
                m_Bindings.count(variable.Name))
                return E_FAIL;
            m_Bindings[variable.Name] = { bufferIndex, variable.StartOffset, variable.Size };
        }
        m_Buffers.push_back(std::move(buffer));
    }
    return S_OK;
}

bool C_ShaderConstants::FindVariable(const char* name, UINT* bufferSlot,
                                      UINT* offset, UINT* bytes) const
{
    if (!name) return false;
    const auto it = m_Bindings.find(name);
    if (it == m_Bindings.end()) return false;
    const C_Binding& binding = it->second;
    if (bufferSlot) *bufferSlot = m_Buffers[binding.m_BufferIndex].m_Slot;
    if (offset) *offset = binding.m_Offset;
    if (bytes) *bytes = binding.m_Bytes;
    return true;
}

HRESULT C_ShaderConstants::SetVariable(const char* name, const void* data, UINT bytes)
{
    if (!name || !data || !bytes) return E_INVALIDARG;
    const auto it = m_Bindings.find(name);
    if (it == m_Bindings.end()) return E_INVALIDARG;
    const C_Binding& binding = it->second;
    if (bytes > binding.m_Bytes) return E_INVALIDARG;
    C_Buffer& buffer = m_Buffers[binding.m_BufferIndex];
    std::memcpy(buffer.m_Data.data() + binding.m_Offset, data, bytes);
    buffer.m_Dirty = true;
    return S_OK;
}

HRESULT C_ShaderConstants::UploadAndBind(ID3D11DeviceContext* context, bool vertex)
{
    if (!context) return E_INVALIDARG;
    for (C_Buffer& buffer : m_Buffers)
    {
        if (buffer.m_Dirty)
        {
            D3D11_MAPPED_SUBRESOURCE mapped = {};
            HRESULT result = context->Map(buffer.m_GPU.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            if (FAILED(result)) return result;
            std::memcpy(mapped.pData, buffer.m_Data.data(), buffer.m_Data.size());
            context->Unmap(buffer.m_GPU.Get(), 0);
            buffer.m_Dirty = false;
        }
        ID3D11Buffer* gpu = buffer.m_GPU.Get();
        if (vertex) context->VSSetConstantBuffers(buffer.m_Slot, 1, &gpu);
        else context->PSSetConstantBuffers(buffer.m_Slot, 1, &gpu);
    }
    return S_OK;
}

HRESULT C_ShaderConstants::UploadAndBindVS(ID3D11DeviceContext* context)
{
    return UploadAndBind(context, true);
}

HRESULT C_ShaderConstants::UploadAndBindPS(ID3D11DeviceContext* context)
{
    return UploadAndBind(context, false);
}

} }
