#include "Display/Device/D3D11/D3D11InputLayout.h"

#include <vector>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <cstring>
#include <cstdio>

namespace Display { namespace D3D11 {

namespace { thread_local char g_inputLayoutDiagnostic[256] = {}; }
const char* LastGameInputLayoutDiagnostic() { return g_inputLayoutDiagnostic; }

namespace {
DXGI_FORMAT Format(BYTE type)
{
    switch (type)
    {
    case D3DDECLTYPE_FLOAT1: return DXGI_FORMAT_R32_FLOAT;
    case D3DDECLTYPE_FLOAT2: return DXGI_FORMAT_R32G32_FLOAT;
    case D3DDECLTYPE_FLOAT3: return DXGI_FORMAT_R32G32B32_FLOAT;
    case D3DDECLTYPE_FLOAT4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case D3DDECLTYPE_D3DCOLOR: return DXGI_FORMAT_B8G8R8A8_UNORM;
    case D3DDECLTYPE_UBYTE4: return DXGI_FORMAT_R8G8B8A8_UINT;
    case D3DDECLTYPE_SHORT2: return DXGI_FORMAT_R16G16_SINT;
    case D3DDECLTYPE_SHORT4: return DXGI_FORMAT_R16G16B16A16_SINT;
    case D3DDECLTYPE_UBYTE4N: return DXGI_FORMAT_R8G8B8A8_UNORM;
    case D3DDECLTYPE_SHORT2N: return DXGI_FORMAT_R16G16_SNORM;
    case D3DDECLTYPE_SHORT4N: return DXGI_FORMAT_R16G16B16A16_SNORM;
    case D3DDECLTYPE_USHORT2N: return DXGI_FORMAT_R16G16_UNORM;
    case D3DDECLTYPE_USHORT4N: return DXGI_FORMAT_R16G16B16A16_UNORM;
    case D3DDECLTYPE_FLOAT16_2: return DXGI_FORMAT_R16G16_FLOAT;
    case D3DDECLTYPE_FLOAT16_4: return DXGI_FORMAT_R16G16B16A16_FLOAT;
    default: return DXGI_FORMAT_UNKNOWN;
    }
}

const char* Semantic(BYTE usage)
{
    switch (usage)
    {
    case D3DDECLUSAGE_POSITION: return "POSITION";
    case D3DDECLUSAGE_BLENDWEIGHT: return "BLENDWEIGHT";
    case D3DDECLUSAGE_BLENDINDICES: return "BLENDINDICES";
    case D3DDECLUSAGE_NORMAL: return "NORMAL";
    case D3DDECLUSAGE_PSIZE: return "PSIZE";
    case D3DDECLUSAGE_TEXCOORD: return "TEXCOORD";
    case D3DDECLUSAGE_TANGENT: return "TANGENT";
    case D3DDECLUSAGE_BINORMAL: return "BINORMAL";
    case D3DDECLUSAGE_COLOR: return "COLOR";
    case D3DDECLUSAGE_FOG: return "FOG";
    case D3DDECLUSAGE_DEPTH: return "DEPTH";
    case D3DDECLUSAGE_SAMPLE: return "SAMPLE";
    default: return nullptr;
    }
}
}

HRESULT CreateGameInputLayout(ID3D11Device* device,
                              const D3DVERTEXELEMENT9* elements, UINT count,
                              const void* signature, SIZE_T signatureBytes,
                              Microsoft::WRL::ComPtr<ID3D11InputLayout>& layout)
{
    g_inputLayoutDiagnostic[0] = 0;
    if (!device || !elements || !count || !signature || !signatureBytes)
        return E_INVALIDARG;
    std::vector<D3D11_INPUT_ELEMENT_DESC> native;
    native.reserve(count);
    Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflection;
    HRESULT result = D3DReflect(signature, signatureBytes, IID_ID3D11ShaderReflection,
                                 reinterpret_cast<void**>(reflection.GetAddressOf()));
    if (FAILED(result)) return result;
    D3D11_SHADER_DESC shaderDesc = {};
    result = reflection->GetDesc(&shaderDesc);
    if (FAILED(result)) return result;
    for (UINT parameter = 0; parameter < shaderDesc.InputParameters; ++parameter)
    {
        D3D11_SIGNATURE_PARAMETER_DESC input = {};
        if (FAILED(reflection->GetInputParameterDesc(parameter, &input))) return E_FAIL;
        if (input.SystemValueType != D3D_NAME_UNDEFINED) continue;
        const D3DVERTEXELEMENT9* match = nullptr;
        for (UINT i = 0; i < count; ++i)
        {
            const char* semantic = Semantic(elements[i].Usage);
            if (semantic && _stricmp(semantic, input.SemanticName) == 0 &&
                elements[i].UsageIndex == input.SemanticIndex)
            { match = &elements[i]; break; }
        }
        if (!match)
        {
            // [Blur1PCDX11] D3D9 vertex declarations can omit inputs used by
            // a shader. Bind a constant zero vertex stream for those semantics.
            // The native device supplies (0,0,0,1) at slots 29-31.
            D3D11_INPUT_ELEMENT_DESC missing = {};
            missing.SemanticName = input.SemanticName;
            missing.SemanticIndex = input.SemanticIndex;
            missing.AlignedByteOffset = 0;
            missing.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            switch (input.ComponentType)
            {
            case D3D_REGISTER_COMPONENT_FLOAT32:
                missing.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                missing.InputSlot = 31;
                break;
            case D3D_REGISTER_COMPONENT_UINT32:
                missing.Format = DXGI_FORMAT_R32G32B32A32_UINT;
                missing.InputSlot = 30;
                break;
            case D3D_REGISTER_COMPONENT_SINT32:
                missing.Format = DXGI_FORMAT_R32G32B32A32_SINT;
                missing.InputSlot = 29;
                break;
            default:
                return E_NOTIMPL;
            }
            native.push_back(missing);
            continue;
        }
        const D3DVERTEXELEMENT9& source = *match;
        const DXGI_FORMAT format = Format(source.Type);
        const char* semantic = Semantic(source.Usage);
        if (format == DXGI_FORMAT_UNKNOWN || !semantic || source.Stream >=
            D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT)
            return E_NOTIMPL;
        D3D11_INPUT_ELEMENT_DESC desc = {};
        desc.SemanticName = input.SemanticName;
        desc.SemanticIndex = input.SemanticIndex;
        desc.Format = format;
        desc.InputSlot = source.Stream;
        desc.AlignedByteOffset = source.Offset;
        desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        native.push_back(desc);
    }
    if (native.empty()) return E_INVALIDARG;
    result = device->CreateInputLayout(native.data(), static_cast<UINT>(native.size()),
                                       signature, signatureBytes, &layout);
    if (FAILED(result))
        std::snprintf(g_inputLayoutDiagnostic, sizeof(g_inputLayoutDiagnostic),
                      "CreateInputLayout failed with %u reflected inputs", (unsigned)native.size());
    return result;
}

} }
