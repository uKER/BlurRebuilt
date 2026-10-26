#ifndef DISPLAY_DEVICE_D3D11_CONSTANTS_H
#define DISPLAY_DEVICE_D3D11_CONSTANTS_H

#include <d3d11.h>
#include <wrl/client.h>
#include <map>
#include <string>
#include <vector>

namespace Display { namespace D3D11 {

// [Blur1PCDX11] Bind the engine's named shader parameters to SM5 constant
// buffers. D3D9 c-register offsets are not preserved by the SM5 compiler.
class C_ShaderConstants
{
public:
    HRESULT Initialise(ID3D11Device* device, const void* shader, size_t shaderBytes);
    bool FindVariable(const char* name, UINT* bufferSlot, UINT* offset, UINT* bytes) const;
    HRESULT SetVariable(const char* name, const void* data, UINT bytes);
    HRESULT UploadAndBindVS(ID3D11DeviceContext* context);
    HRESULT UploadAndBindPS(ID3D11DeviceContext* context);

private:
    struct C_Binding
    {
        UINT m_BufferIndex;
        UINT m_Offset;
        UINT m_Bytes;
    };
    struct C_Buffer
    {
        UINT m_Slot;
        bool m_Dirty;
        std::vector<unsigned char> m_Data;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_GPU;
    };
    HRESULT UploadAndBind(ID3D11DeviceContext* context, bool vertex);

    std::vector<C_Buffer> m_Buffers;
    std::map<std::string, C_Binding> m_Bindings;
};

} }

#endif
