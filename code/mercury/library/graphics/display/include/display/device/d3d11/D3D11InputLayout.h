#ifndef DISPLAY_DEVICE_D3D11_INPUT_LAYOUT_H
#define DISPLAY_DEVICE_D3D11_INPUT_LAYOUT_H

// [Blur1PCDX11] Build a native input layout from the game's vertex metadata.
#include <d3d11.h>
#include <d3d9types.h>
#include <wrl/client.h>

namespace Display { namespace D3D11 {

HRESULT CreateGameInputLayout(ID3D11Device* device,
                              const D3DVERTEXELEMENT9* elements, UINT count,
                              const void* signature, SIZE_T signatureBytes,
                              Microsoft::WRL::ComPtr<ID3D11InputLayout>& layout);
const char* LastGameInputLayoutDiagnostic();

} }

#endif
