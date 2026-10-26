#include "StdAfx.h"
#include <system/debug/Blur2PCAutomation.h>
#include "Display/Render/Render.h"
#include "Display/Utils/Screenshot.h"
#include <wrl/client.h>
#include <d3dcompiler.h>
#include "Render/PostProcess/Interface.h"

#include "Render/PostProcess/Manager.h"

using namespace Display;
using Microsoft::WRL::ComPtr;

namespace PostProcess
{

I_PostProcess::I_PostProcess(C_Manager& manager) : m_Manager(manager), m_pShader(NULL), m_TextureFilter(Display::E_TextureFilter_Linear), m_eResolve(E_Resolve_True), m_eType(E_Type_Intermediate), m_eChainable(E_Chainable_True), m_bDepthEnable(false)
{
	m_Manager.AddPostProcess(this);
} 

I_PostProcess::~I_PostProcess()
{
	m_Manager.RemovePostProcess(this);
}

void I_PostProcess::DisplayFinal(const Display::C_PixelShader* pPixelShader, Display::E_TextureFilter textureFilter) const
{
	// if this is the final post process we tell the manager, this will set up our final viewports correctly
	if(m_eType == E_Type_Final)
	{
		m_Manager.RestoreColourSurface();
		m_Manager.RestoreViewport();
		m_Manager.SetFinalQuadSettings();
		DisplayIntermediate(pPixelShader, textureFilter);
		{
			static bool probed = false;
			char probe[2] = {};
			if (!probed && Blur2PCAutomation::Now() > 25.0 &&
				GetEnvironmentVariableA("BLUR_DX11_TEST_QUAD", probe, sizeof(probe)) && probe[0] == '1')
			{
				probed = true;
				Blur2PCAutomation::LogF("DX11 final-display probe quad t=%.1f", Blur2PCAutomation::Now());
				{
					ID3D11DeviceContext* shotContext = nullptr;
					Display::Device()->GetD3DDevice()->GetImmediateContext(&shotContext);
					ID3D11RenderTargetView* shotTarget = nullptr;
					if (shotContext) shotContext->OMGetRenderTargets(1, &shotTarget, nullptr);
					ComPtr<ID3D11Resource> shotResource;
					if (shotTarget) { shotTarget->GetResource(&shotResource); shotTarget->Release(); }
					ComPtr<ID3D11Texture2D> shotTexture;
					if (shotResource) shotResource.As(&shotTexture);
					if (shotResource)
						Display::Device()->ReportTextureStats("final", shotResource.Get(), 0);
					if (ID3D11Resource* sampled = Display::Device()->GetBoundTexture(0))
						Display::Device()->ReportTextureStats("final-source", sampled, 0);
					else
						Blur2PCAutomation::LogF("DX11 final-source none bound");
					Display::Device()->ReportPixelConstants(0, 16);
					if (const Display::C_PixelShader* boundShader = Display::Device()->GetPixelShader())
					{
						if (!boundShader->m_Bytecode.empty())
						{
							ComPtr<ID3DBlob> listing;
							if (SUCCEEDED(D3DDisassemble(boundShader->m_Bytecode.data(), boundShader->m_Bytecode.size(),
								0, nullptr, &listing)))
							{
								FILE* file = nullptr;
								CreateDirectoryA("dx11_trace", nullptr); // [Blur2PC] opt-in dump beside the game, not a developer path
								if (fopen_s(&file, "dx11_trace\\final.ps.txt", "wb") == 0)
								{
									fwrite(listing->GetBufferPointer(), 1, listing->GetBufferSize(), file);
									fclose(file);
								}
								Blur2PCAutomation::LogF("DX11 final-display shader dumped bytes=%u", (unsigned)boundShader->m_Bytecode.size());
							}
						}
					}
					{
						char slots[256] = {};
						size_t used = 0;
						for (U32 slot = 0; slot < 11; ++slot)
						{
							ID3D11Resource* bound = Display::Device()->GetBoundTexture(slot);
							used += sprintf_s(slots + used, sizeof(slots) - used, "%u=%s ", slot, bound ? "yes" : "no");
						}
						Blur2PCAutomation::LogF("DX11 final-display samplers %s", slots);
					}
					if (shotContext) shotContext->Release();
				}
				{
					// [Blur1PCDX11] Raw draw with locally built shaders and buffer, to tell a broken
					// pipeline state from a broken engine draw path.
					struct S_Vertex { float x, y, z; float r, g, b, a; };
					static ComPtr<ID3D11VertexShader> rawVS;
					static ComPtr<ID3D11PixelShader> rawPS;
					static ComPtr<ID3D11InputLayout> rawLayout;
					static ComPtr<ID3D11Buffer> rawBuffer;
					ID3D11Device* rawDevice = Display::Device()->GetD3DDevice();
					ID3D11DeviceContext* rawContext = nullptr;
					rawDevice->GetImmediateContext(&rawContext);
					if (!rawVS)
					{
						const char* vsSource = "struct I { float3 p : POSITION; float4 c : COLOR0; };"
							"struct O { float4 c : COLOR0; float4 p : SV_Position; };"
							"O main(I i) { O o; o.c = i.c; o.p = float4(i.p, 1); return o; }";
						const char* psSource = "float4 main(float4 c : COLOR0) : SV_Target { return c; }";
						ComPtr<ID3DBlob> vsBlob, psBlob, error;
						if (SUCCEEDED(D3DCompile(vsSource, strlen(vsSource), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, &error)) &&
							SUCCEEDED(D3DCompile(psSource, strlen(psSource), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &error)))
						{
							rawDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &rawVS);
							rawDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &rawPS);
							const D3D11_INPUT_ELEMENT_DESC elements[] = {
								{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
								{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}};
							rawDevice->CreateInputLayout(elements, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &rawLayout);
							const S_Vertex vertices[3] = {
								{-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
								{-1.0f,  3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
								{ 3.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f}};
							D3D11_BUFFER_DESC bufferDesc = {};
							bufferDesc.ByteWidth = sizeof(vertices);
							bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
							bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
							D3D11_SUBRESOURCE_DATA initial = {};
							initial.pSysMem = vertices;
							rawDevice->CreateBuffer(&bufferDesc, &initial, &rawBuffer);
						}
						Blur2PCAutomation::LogF("DX11 raw draw objects VS=%p PS=%p IA=%p VB=%p",
							rawVS.Get(), rawPS.Get(), rawLayout.Get(), rawBuffer.Get());
					}
					if (rawVS && rawPS && rawLayout && rawBuffer && rawContext)
					{
						ID3D11Buffer* buffer = rawBuffer.Get();
						UINT stride = sizeof(S_Vertex), offset = 0;
						rawContext->IASetInputLayout(rawLayout.Get());
						rawContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
						rawContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
						rawContext->VSSetShader(rawVS.Get(), nullptr, 0);
						rawContext->PSSetShader(rawPS.Get(), nullptr, 0);
						D3D11_BLEND_DESC rawBlend = {};
						rawBlend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
						rawBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
						rawBlend.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
						rawBlend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
						rawBlend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
						rawBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
						rawBlend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
						ComPtr<ID3D11BlendState> rawBlendState;
						rawDevice->CreateBlendState(&rawBlend, &rawBlendState);
						const float factor[4] = {1, 1, 1, 1};
						rawContext->OMSetBlendState(rawBlendState.Get(), factor, 0xffffffff);
						D3D11_DEPTH_STENCIL_DESC rawDepth = {};
						rawDepth.DepthEnable = FALSE;
						rawDepth.StencilEnable = FALSE;
						ComPtr<ID3D11DepthStencilState> rawDepthState;
						rawDevice->CreateDepthStencilState(&rawDepth, &rawDepthState);
						rawContext->OMSetDepthStencilState(rawDepthState.Get(), 0);
						D3D11_RASTERIZER_DESC rawRaster = {};
						rawRaster.FillMode = D3D11_FILL_SOLID;
						rawRaster.CullMode = D3D11_CULL_NONE;
						rawRaster.DepthClipEnable = TRUE;
						ComPtr<ID3D11RasterizerState> rawRasterState;
						rawDevice->CreateRasterizerState(&rawRaster, &rawRasterState);
						rawContext->RSSetState(rawRasterState.Get());
						Blur2PCAutomation::LogF("DX11 raw draw issued");
						rawContext->Draw(3, 0);
						// [Blur1PCDX11] The diagnostic bypassed the device's geometry setters.
						Display::Device()->InvalidateNativeInputBindings();
					}
					if (rawContext) rawContext->Release();
				}
				{
					ID3D11DeviceContext* clearContext = nullptr;
					Display::Device()->GetD3DDevice()->GetImmediateContext(&clearContext);
					ID3D11RenderTargetView* clearTarget = nullptr;
					if (clearContext) clearContext->OMGetRenderTargets(1, &clearTarget, nullptr);
					if (clearTarget)
					{
						const float probeColour[4] = {1.0f, 0.0f, 1.0f, 1.0f};
						clearContext->ClearRenderTargetView(clearTarget, probeColour);
						Blur2PCAutomation::LogF("DX11 final-display cleared the bound target");
						clearTarget->Release();
					}
					if (clearContext) clearContext->Release();
				}
				{
					ID3D11DeviceContext* probeContext = nullptr;
					Display::Device()->GetD3DDevice()->GetImmediateContext(&probeContext);
					if (probeContext)
					{
						D3D11_VIEWPORT probeViewport = {}; UINT viewportCount = 1;
						probeContext->RSGetViewports(&viewportCount, &probeViewport);
						D3D11_RECT probeScissor = {};
						probeContext->RSGetScissorRects(&viewportCount, &probeScissor);
						ComPtr<ID3D11RasterizerState> probeRaster;
						probeContext->RSGetState(&probeRaster);
						D3D11_RASTERIZER_DESC probeRasterDesc = {};
						if (probeRaster) probeRaster->GetDesc(&probeRasterDesc);
						ID3D11RenderTargetView* probeTarget = nullptr;
						probeContext->OMGetRenderTargets(1, &probeTarget, nullptr);
						ID3D11Resource* probeResource = nullptr;
						if (probeTarget) probeTarget->GetResource(&probeResource);
						const Display::C_Surface* probeBack = Display::Device()->GetBackBuffer().GetColourSurface();
						ComPtr<ID3D11BlendState> probeBlend;
						float probeBlendFactor[4] = {}; UINT probeSampleMask = 0;
						probeContext->OMGetBlendState(&probeBlend, probeBlendFactor, &probeSampleMask);
						D3D11_BLEND_DESC probeBlendDesc = {};
						if (probeBlend) probeBlend->GetDesc(&probeBlendDesc);
						ID3D11DepthStencilView* probeDepth = nullptr;
						probeContext->OMGetRenderTargets(0, nullptr, &probeDepth);
						{
							D3D11_TEXTURE2D_DESC colourDesc = {};
							ComPtr<ID3D11Resource> probeColour; if (probeTarget) probeTarget->GetResource(&probeColour);
							ComPtr<ID3D11Texture2D> probeColourTexture; if (probeColour) probeColour.As(&probeColourTexture);
							if (probeColourTexture) probeColourTexture->GetDesc(&colourDesc);
							D3D11_TEXTURE2D_DESC depthDesc = {};
							ComPtr<ID3D11Resource> probeDepthResource;
							if (probeDepth) probeDepth->GetResource(&probeDepthResource);
							ComPtr<ID3D11Texture2D> probeDepthTexture; if (probeDepthResource) probeDepthResource.As(&probeDepthTexture);
							if (probeDepthTexture) probeDepthTexture->GetDesc(&depthDesc);
							Blur2PCAutomation::LogF("DX11 final-display targets colour=%ux%u fmt=%u depth=%ux%u fmt=%u dsv=%p",
								colourDesc.Width, colourDesc.Height, (unsigned)colourDesc.Format,
								depthDesc.Width, depthDesc.Height, (unsigned)depthDesc.Format, probeDepth);
						}
						if (probeDepth) probeDepth->Release();
						ComPtr<ID3D11RasterizerState> probeRasterState;
						probeContext->RSGetState(&probeRasterState);
						D3D11_RASTERIZER_DESC probeRasterStateDesc = {};
						if (probeRasterState) probeRasterState->GetDesc(&probeRasterStateDesc);
						Blur2PCAutomation::LogF("DX11 final-display raster fill=%u cull=%u ccw=%u depthClip=%u scissor=%u",
							probeRasterStateDesc.FillMode, probeRasterStateDesc.CullMode,
							probeRasterStateDesc.FrontCounterClockwise ? 1u : 0u,
							probeRasterStateDesc.DepthClipEnable ? 1u : 0u,
							probeRasterStateDesc.ScissorEnable ? 1u : 0u);
						Blur2PCAutomation::LogF("DX11 final-display blend enable=%u src=%u dst=%u op=%u mask=%u factor=%.2f,%.2f,%.2f,%.2f",
							probeBlendDesc.RenderTarget[0].BlendEnable, probeBlendDesc.RenderTarget[0].SrcBlend,
							probeBlendDesc.RenderTarget[0].DestBlend, probeBlendDesc.RenderTarget[0].BlendOp,
							probeBlendDesc.RenderTarget[0].RenderTargetWriteMask,
							probeBlendFactor[0], probeBlendFactor[1], probeBlendFactor[2], probeBlendFactor[3]);
						Blur2PCAutomation::LogF("DX11 final-display state vp=%.0f,%.0f %.0fx%.0f scissor=%u rtvResource=%p backBufferTexture=%p engineSurface=%p windowSurface=%p",
							probeViewport.TopLeftX, probeViewport.TopLeftY, probeViewport.Width, probeViewport.Height,
							probeRasterDesc.ScissorEnable ? 1u : 0u, probeResource,
							probeBack ? probeBack->m_pd3dSurface : nullptr,
							Display::Device()->GetRenderTarget().GetColourSurface(),
							Display::Device()->GetWindowCount() > 0 ? &Display::Device()->GetWindow(0)->m_ColourRenderTexture.GetSurface() : nullptr);
						if (probeResource) probeResource->Release();
						if (probeTarget) probeTarget->Release();
						probeContext->Release();
					}
				}
				Display::Quad::DrawFullScreenColour(Maths::C_RGBA_F32(1.0f, 0.0f, 1.0f, 1.0f));
				if (ID3D11Resource* afterQuad = Display::Device()->GetBackBuffer().GetColourSurface()->m_pd3dSurface)
					Display::Device()->ReportTextureStats("after-magenta", afterQuad, 0);
			}
		}
		m_Manager.SetActive(false);
	}
	else
		DisplayIntermediate(pPixelShader, textureFilter);
}

void I_PostProcess::DisplayIntermediate(const Display::C_PixelShader* pPixelShader, Display::E_TextureFilter textureFilter) const
{
	
	Display::Device()->SetTextureAddressUVW(0, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_False);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);

	if(textureFilter != Display::E_TextureFilter_Unknown)
		Display::Device()->SetTextureFilter(0, textureFilter);
	else
		Display::Device()->SetTextureFilter(0, m_TextureFilter);
	
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetVertexShader(&VertexShader_XY::m_VertexShader);
	if(pPixelShader)
		Display::Device()->SetPixelShader(pPixelShader);
	else
		Display::Device()->SetPixelShader(m_pShader);
	
	if(m_bDepthEnable)
		Display::Quad::DrawFullScreenDepth();
	else
		Display::Quad::DrawFullScreen();

}

// resolves the current rendertarget into m_pRenderTexture and restores the viewport for this view if not chainable
void I_PostProcess::SetViewport() const
{
	// resolve the current render target if necessary
	if(m_eResolve == E_Resolve_True)
		Resolve();

	// if this post process doesn't chain from the previous restore the full viewport for the current view
	if(m_eChainable == E_Chainable_False)
	{
		m_Manager.SetViewport();
	}

	// store the current render target as the chain render target
	m_Manager.SetChainTexture();
}

void I_PostProcess::Resolve() const
{
	m_Manager.ResolveRenderTexture();
}

}; // namespace PostProcess
