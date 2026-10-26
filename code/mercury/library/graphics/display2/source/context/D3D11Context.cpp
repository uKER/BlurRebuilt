// [Blur1PCDX11] Native Display2 resource objects and context state.
#include "StdAfx.h"
#include "Display2/Context/D3D11Context.h"
#include "Display2/Device/Device.h"
#include <d3dcompiler.h>
#include <d3d11sdklayers.h>
#include <map>
#include <mutex>
#include <vector>
#include <system/debug/Blur2PCAutomation.h>

namespace
{
void DumpNativeInfoQueue(ID3D11Device* device, const char* where)
{
	if (!device) return;
	ID3D11InfoQueue* queue = nullptr;
	if (FAILED(device->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&queue))) || !queue)
		return;
	const UINT64 count = queue->GetNumStoredMessages();
	for (UINT64 index = 0; index < count; ++index)
	{
		SIZE_T size = 0;
		queue->GetMessage(index, nullptr, &size);
		std::vector<U8> bytes(size);
		D3D11_MESSAGE* message = reinterpret_cast<D3D11_MESSAGE*>(bytes.data());
		if (SUCCEEDED(queue->GetMessage(index, message, &size)))
			Blur2PCAutomation::LogF("DX11 debug %s sev=%u: %s", where,
				static_cast<unsigned>(message->Severity), message->pDescription);
	}
	queue->Release();
}

// [Blur1PCDX11] D3D11 default resources cannot be mapped. Keep a staging copy
// for the duration of each legacy resource lock and copy writes back on unlock.
struct S_StagingLock
{
	ID3D11Resource* staging;
	ID3D11DeviceContext* context;
	UINT subresource;
	bool write;
	D3DFORMAT pcFormat;
	UINT width, height, depth;
	D3D11_MAPPED_SUBRESOURCE nativeMap;
	std::vector<U8> pcBytes;
	std::vector<U8>* shadow = nullptr; // dynamic buffer contents, see LockResource
	unsigned nested = 1;                // buffer locks held (D3D9 allows nested buffer locks)
	D3D11_MAPPED_SUBRESOURCE callerMap = {};
	UINT blockRows = 0; // packed PC DXT rows, excluding native staging padding
};
using S_StagingKey = std::pair<const ID3D11Resource*, UINT>;
std::map<S_StagingKey, S_StagingLock> g_stagingLocks;
std::mutex g_stagingMutex;

void TraceLockFailure(const char* reason, ID3D11Resource* resource, UINT subresource, HRESULT result)
{
	static unsigned count = 0;
	if (count++ >= 16) return;
	void* frames[8] = {};
	const USHORT captured = CaptureStackBackTrace(1,8,frames,nullptr);
	Blur2PCAutomation::LogF("DX11 resource lock failed %s resource=%p sub=%u hr=%08x",
		reason,resource,subresource,static_cast<unsigned>(result));
	for (USHORT i=0; i<captured; ++i) Blur2PCAutomation::LogF("DX11 lock caller[%u]=%p",i,frames[i]);
}

// [Blur1PCDX11] The D3D11 debug layer raises 0x87D on the corruption it detects;
// catch it so the message reaches the log instead of the crash handler.
HRESULT TryMapStaging(ID3D11DeviceContext* context, ID3D11Resource* staging,
                      D3D11_MAPPED_SUBRESOURCE& mapped,
                      D3D11_MAP mode = D3D11_MAP_READ_WRITE)
{
	__try
	{
		return context->Map(staging, 0, mode, 0, &mapped);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
}

// [Blur1PCDX11] PC D3D9 creates every resource without E_Flags_Video in the
// MANAGED or SYSTEMMEM pool (D3D9Context.inl D3DPool), where a lock touches
// only the runtime's system-memory copy and never waits for the GPU. Keep that
// copy as a persistent staging mirror per locked subresource of any resource
// the GPU cannot render into. The first lock reads it back; later locks map it
// directly, and unlock writes it back as before. GPU copies into such a
// resource refresh the mirror right after the write (D3D11ResourceWritten), as
// D3D9's GetRenderTargetData fills a system-memory surface.
const GUID kStagingMirrorGuid =
	{ 0x6c0f6a2e, 0x5d43, 0x4c1b, { 0x9a, 0x31, 0x2b, 0x8e, 0x74, 0x10, 0x5f, 0xd2 } };

class C_StagingMirrors final : public IUnknown
{
public:
	std::map<UINT, ID3D11Resource*> m_Staging; // owned references, by subresource
	// A DYNAMIC buffer cannot be read back. D3D9 keeps its contents across a lock
	// without DISCARD/NOOVERWRITE; this is that contents for such locks.
	std::vector<U8> m_Shadow;
	bool m_ShadowStale = false;

	~C_StagingMirrors() { for (auto& entry : m_Staging) entry.second->Release(); }
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override
	{
		if (!object) return E_POINTER;
		*object = nullptr;
		if (iid != __uuidof(IUnknown)) return E_NOINTERFACE;
		AddRef();
		*object = static_cast<IUnknown*>(this);
		return S_OK;
	}
	ULONG STDMETHODCALLTYPE AddRef() override { return static_cast<ULONG>(InterlockedIncrement(&m_References)); }
	ULONG STDMETHODCALLTYPE Release() override
	{
		const LONG references = InterlockedDecrement(&m_References);
		if (!references) delete this;
		return static_cast<ULONG>(references);
	}

private:
	LONG m_References = 1;
};

// The resource owns its mirrors through private data, so they are released
// with it; the returned pointer is valid while the resource is.
C_StagingMirrors* StagingMirrors(ID3D11Resource* resource, bool create)
{
	IUnknown* found = nullptr;
	UINT size = sizeof(found);
	if (SUCCEEDED(resource->GetPrivateData(kStagingMirrorGuid, &size, &found)) && found)
	{
		found->Release();
		return static_cast<C_StagingMirrors*>(found);
	}
	if (!create) return nullptr;
	C_StagingMirrors* mirrors = new C_StagingMirrors;
	const HRESULT hr = resource->SetPrivateDataInterface(kStagingMirrorGuid, mirrors);
	mirrors->Release();
	return SUCCEEDED(hr) ? mirrors : nullptr;
}

bool CanMirror(ID3D11Resource* resource, D3D11_RESOURCE_DIMENSION dimension)
{
	const UINT gpuWrites = D3D11_BIND_RENDER_TARGET | D3D11_BIND_DEPTH_STENCIL |
		D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_STREAM_OUTPUT;
	if (dimension == D3D11_RESOURCE_DIMENSION_BUFFER)
	{
		D3D11_BUFFER_DESC desc;
		static_cast<ID3D11Buffer*>(resource)->GetDesc(&desc);
		return desc.Usage == D3D11_USAGE_DEFAULT && !(desc.BindFlags & gpuWrites);
	}
	if (dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
	{
		D3D11_TEXTURE2D_DESC desc;
		static_cast<ID3D11Texture2D*>(resource)->GetDesc(&desc);
		return desc.Usage == D3D11_USAGE_DEFAULT && !(desc.BindFlags & gpuWrites) && desc.SampleDesc.Count == 1;
	}
	if (dimension == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
	{
		D3D11_TEXTURE3D_DESC desc;
		static_cast<ID3D11Texture3D*>(resource)->GetDesc(&desc);
		return desc.Usage == D3D11_USAGE_DEFAULT && !(desc.BindFlags & gpuWrites);
	}
	return false;
}

bool LockResource(ID3D11Resource* resource, UINT subresource, bool write,
                 D3D11_MAPPED_SUBRESOURCE& mapped, U32 flags = 0)
{
	if (!resource) { TraceLockFailure("null resource",resource,subresource,E_POINTER); return false; }
	std::lock_guard<std::mutex> lock(g_stagingMutex);
	const S_StagingKey key(resource, subresource);
	const auto locked = g_stagingLocks.find(key);
	if (locked != g_stagingLocks.end())
	{
		// [Blur1PCDX11] D3D9 vertex and index buffers accept nested locks and return the
		// same memory; each Unlock releases one. Textures keep the single-lock rule.
		D3D11_RESOURCE_DIMENSION lockedDimension;
		resource->GetType(&lockedDimension);
		if (lockedDimension == D3D11_RESOURCE_DIMENSION_BUFFER)
		{
			++locked->second.nested;
			// A directly mapped dynamic buffer (staging == resource) is never copied back.
			if (write && (locked->second.shadow || locked->second.staging != resource))
				locked->second.write = true;
			mapped = locked->second.callerMap;
			return true;
		}
		TraceLockFailure("already locked",resource,subresource,E_FAIL);
		return false;
	}
	ID3D11Device* device = nullptr;
	resource->GetDevice(&device);
	if (!device) return false;
	ID3D11DeviceContext* context = nullptr;
	device->GetImmediateContext(&context);
	D3D11_RESOURCE_DIMENSION dimension;
	resource->GetType(&dimension);
	if (dimension == D3D11_RESOURCE_DIMENSION_BUFFER)
	{
		D3D11_BUFFER_DESC desc;
		static_cast<ID3D11Buffer*>(resource)->GetDesc(&desc);
		if (desc.Usage == D3D11_USAGE_DYNAMIC)
		{
			const bool append = (flags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)) != 0;
			C_StagingMirrors* mirrors = StagingMirrors(resource, !append);
			if (mirrors && (flags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)))
				mirrors->m_ShadowStale = !mirrors->m_Shadow.empty(); // written below without the copy
			else if (mirrors)
			{
				// [Blur1PCDX11] D3D9 returns a dynamic buffer's current contents for a lock
				// without DISCARD/NOOVERWRITE (DynamicEffect::C_DynamicVertexBuffer locks
				// that way each frame). Keep them on the CPU and write the whole buffer
				// with WRITE_DISCARD on unlock, so the GPU copy is renamed and never waited for.
				if (mirrors->m_ShadowStale)
				{
					static unsigned stale = 0;
					if (stale++ < 4) Blur2PCAutomation::LogF("DX11 dynamic buffer %p mixes DISCARD and plain locks", resource);
					mirrors->m_ShadowStale = false;
				}
				if (mirrors->m_Shadow.size() != desc.ByteWidth) mirrors->m_Shadow.assign(desc.ByteWidth, 0);
				mapped.pData = mirrors->m_Shadow.data();
				mapped.RowPitch = mapped.DepthPitch = desc.ByteWidth;
				S_StagingLock& held = g_stagingLocks[key];
				held = {};
				resource->AddRef();
				held.staging = resource; held.context = context; held.subresource = subresource;
				held.write = write; held.pcFormat = D3DFMT_UNKNOWN; held.shadow = &mirrors->m_Shadow;
				held.callerMap = mapped;
				context->Release();
				device->Release();
				return true;
			}
		}
	}
	ID3D11Resource* staging = nullptr;
	HRESULT hr = E_FAIL;
	bool directWrite = false;
	D3D11_MAP mode = D3D11_MAP_READ_WRITE;
	UINT width = 0, height = 1, depth = 1;
	const bool mirrored = CanMirror(resource, dimension);
	bool reused = false;
	if (mirrored)
	{
		if (C_StagingMirrors* mirrors = StagingMirrors(resource, false))
		{
			const auto found = mirrors->m_Staging.find(subresource);
			if (found != mirrors->m_Staging.end())
			{
				staging = found->second;
				staging->AddRef();
				reused = true;
				hr = S_OK;
			}
		}
	}
	if (dimension == D3D11_RESOURCE_DIMENSION_BUFFER)
	{
		D3D11_BUFFER_DESC desc;
		static_cast<ID3D11Buffer*>(resource)->GetDesc(&desc);
		// [Blur1PCDX11] Retail DynamicVertexBuffer discards at offset zero and
		// appends with NOOVERWRITE. Honor those promises without a GPU readback.
		if (write && desc.Usage == D3D11_USAGE_DYNAMIC &&
			(flags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)))
		{
			directWrite = true;
			mode = flags & D3DLOCK_DISCARD ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE;
			staging = resource;
			staging->AddRef();
			hr = S_OK;
		}
		else if (!reused)
		{
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			ID3D11Buffer* buffer = nullptr;
			hr = device->CreateBuffer(&desc, nullptr, &buffer);
			staging = buffer;
		}
	}
	else if (dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
	{
		D3D11_TEXTURE2D_DESC desc;
		static_cast<ID3D11Texture2D*>(resource)->GetDesc(&desc);
		const UINT mip = subresource % desc.MipLevels;
		desc.Width = std::max(1U, desc.Width >> mip);
		desc.Height = std::max(1U, desc.Height >> mip);
		width = desc.Width; height = desc.Height;
		const Display2::E_TextureFormat pcFormat = Display2::D3D11PCFormat(desc.Format);
		if (pcFormat == Display2::E_TextureFormat_DXT1 || pcFormat == Display2::E_TextureFormat_DXT3 ||
			pcFormat == Display2::E_TextureFormat_DXT5)
		{
			// [Blur1PCDX11] A standalone staging mip needs whole BC blocks.
			desc.Width = (desc.Width + 3) & ~3U;
			desc.Height = (desc.Height + 3) & ~3U;
		}
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		if (desc.SampleDesc.Count > 1)
		{
			// Multisampled surfaces are read through a single-sample resolve.
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
		}
		if (!reused)
		{
			ID3D11Texture2D* texture = nullptr;
			hr = device->CreateTexture2D(&desc, nullptr, &texture);
			staging = texture;
		}
	}
	else if (dimension == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
	{
		D3D11_TEXTURE3D_DESC desc;
		static_cast<ID3D11Texture3D*>(resource)->GetDesc(&desc);
		desc.Width = std::max(1U, desc.Width >> subresource);
		desc.Height = std::max(1U, desc.Height >> subresource);
		desc.Depth = std::max(1U, desc.Depth >> subresource);
		width = desc.Width; height = desc.Height; depth = desc.Depth;
		desc.MipLevels = 1;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		if (!reused)
		{
			ID3D11Texture3D* texture = nullptr;
			hr = device->CreateTexture3D(&desc, nullptr, &texture);
			staging = texture;
		}
	}
	if (SUCCEEDED(hr) && staging)
	{
		if (reused || directWrite)
			; // the mirror already holds the resource's contents
		else if (dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
		{
			D3D11_TEXTURE2D_DESC desc;
			static_cast<ID3D11Texture2D*>(resource)->GetDesc(&desc);
			if (desc.SampleDesc.Count > 1)
				context->ResolveSubresource(staging, 0, resource, subresource, desc.Format);
			else
				context->CopySubresourceRegion(staging, 0, 0, 0, 0, resource, subresource, nullptr);
		}
		else if (dimension == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
			context->CopySubresourceRegion(staging, 0, 0, 0, 0, resource, subresource, nullptr);
		else
			context->CopyResource(staging, resource);
		hr = TryMapStaging(context, staging, mapped, mode);
		if (SUCCEEDED(hr))
		{
			if (mirrored && !reused)
			{
				if (C_StagingMirrors* mirrors = StagingMirrors(resource, true))
				{
					staging->AddRef();
					mirrors->m_Staging[subresource] = staging;
				}
			}
			S_StagingLock& held = g_stagingLocks[key];
			held = {};
			held.staging = staging; held.context = context; held.subresource = subresource;
			// [Blur1PCDX11] A directly mapped buffer is already the GPU resource;
			// unlock unmaps it and must not copy it onto itself.
			held.write = write && !directWrite;
			held.pcFormat = Display2::D3D11TextureFormats::GetPCFormat(resource);
			held.width = width; held.height = height; held.depth = depth; held.nativeMap = mapped;
			DXGI_FORMAT storage;
			unsigned pcBytes, nativeBytes;
			if (Display2::D3D11TextureFormats::LuminanceLayout(held.pcFormat,&storage,&pcBytes,&nativeBytes))
			{
				const UINT row = width*pcBytes, slice = row*height;
				held.pcBytes.resize(static_cast<size_t>(slice)*depth);
				Display2::D3D11TextureFormats::ConvertLuminance(held.pcFormat,width,height,depth,
					mapped.pData,mapped.RowPitch,mapped.DepthPitch,held.pcBytes.data(),row,slice,false);
				mapped.pData = held.pcBytes.data(); mapped.RowPitch = row; mapped.DepthPitch = slice;
			}
			else if (held.pcFormat == D3DFMT_DXT1 || held.pcFormat == D3DFMT_DXT3 ||
				held.pcFormat == D3DFMT_DXT5)
			{
				// [Blur1PCDX11] The PC DDS loader writes the packed mip in one
				// memcpy. Native staging rows can be padded (the 16x16 DXT5 UI
				// wipe is 64 bytes/row on D3D9, but 128 on the DX11 driver).
				// Expose PC block rows and translate at both lock boundaries.
				const UINT row = ((width + 3) / 4) * (held.pcFormat == D3DFMT_DXT1 ? 8 : 16);
				held.blockRows = (height + 3) / 4;
				const UINT slice = row * held.blockRows;
				held.pcBytes.resize(static_cast<size_t>(slice) * depth);
				for (UINT z = 0; z < depth; ++z)
					for (UINT y = 0; y < held.blockRows; ++y)
						memcpy(held.pcBytes.data() + z * slice + y * row,
							static_cast<const U8*>(mapped.pData) + z * mapped.DepthPitch + y * mapped.RowPitch, row);
				mapped.pData = held.pcBytes.data(); mapped.RowPitch = row; mapped.DepthPitch = slice;
			}
			held.callerMap = mapped;
		}
		else
		{
			TraceLockFailure("staging map",resource,subresource,hr);
			static unsigned traced = 0;
			if (traced++ < 3)
			{
				Blur2PCAutomation::LogF("DX11 staging map failed hr=%08x resource=%p sub=%u write=%u",
					static_cast<unsigned>(hr), resource, subresource, write ? 1u : 0u);
				DumpNativeInfoQueue(device, "staging map");
			}
			staging->Release();
		}
	}
	else TraceLockFailure("staging allocation",resource,subresource,hr);
	context->Release();
	device->Release();
	return SUCCEEDED(hr);
}

void UnlockStagingResource(ID3D11Resource* resource, UINT subresource)
{
	std::lock_guard<std::mutex> guard(g_stagingMutex);
	auto it = g_stagingLocks.find(S_StagingKey(resource, subresource));
	if (it == g_stagingLocks.end()) return;
	if (--it->second.nested > 0) return;
	const S_StagingLock& lock = it->second;
	if (lock.shadow)
	{
		if (lock.write)
		{
			D3D11_MAPPED_SUBRESOURCE target = {};
			if (SUCCEEDED(TryMapStaging(lock.context, resource, target, D3D11_MAP_WRITE_DISCARD)))
			{
				memcpy(target.pData, lock.shadow->data(), lock.shadow->size());
				lock.context->Unmap(resource, 0);
			}
		}
		lock.staging->Release();
		g_stagingLocks.erase(it);
		return;
	}
	if (lock.write && !lock.pcBytes.empty())
	{
		if (lock.blockRows)
		{
			for (UINT z = 0; z < lock.depth; ++z)
				for (UINT y = 0; y < lock.blockRows; ++y)
					memcpy(static_cast<U8*>(lock.nativeMap.pData) + z * lock.nativeMap.DepthPitch + y * lock.nativeMap.RowPitch,
						lock.pcBytes.data() + z * lock.callerMap.DepthPitch + y * lock.callerMap.RowPitch, lock.callerMap.RowPitch);
		}
		else
		{
		const UINT row = static_cast<UINT>(lock.pcBytes.size())/lock.height/lock.depth;
		Display2::D3D11TextureFormats::ConvertLuminance(lock.pcFormat,lock.width,lock.height,lock.depth,
			lock.pcBytes.data(),row,row*lock.height,lock.nativeMap.pData,
			lock.nativeMap.RowPitch,lock.nativeMap.DepthPitch,true);
		}
	}
	lock.context->Unmap(lock.staging, 0);
	if (lock.write)
	{
		D3D11_RESOURCE_DIMENSION dimension;
		resource->GetType(&dimension);
		if (dimension == D3D11_RESOURCE_DIMENSION_BUFFER)
			lock.context->CopyResource(resource, lock.staging);
		else
			lock.context->CopySubresourceRegion(resource, lock.subresource, 0, 0, 0,
			                                   lock.staging, 0, nullptr);
	}
	lock.staging->Release();
	g_stagingLocks.erase(it);
}
}

namespace Display2
{
void D3D11ResourceWritten(ID3D11Resource* resource, ID3D11DeviceContext* context)
{
	if (!resource || !context) return;
	C_StagingMirrors* mirrors = StagingMirrors(resource, false);
	if (!mirrors) return;
	std::lock_guard<std::mutex> lock(g_stagingMutex);
	D3D11_RESOURCE_DIMENSION dimension;
	resource->GetType(&dimension);
	for (auto& entry : mirrors->m_Staging)
	{
		// A mirror mapped by an open lock is refreshed by that lock's unlock.
		if (g_stagingLocks.count(S_StagingKey(resource, entry.first))) continue;
		if (dimension == D3D11_RESOURCE_DIMENSION_BUFFER) context->CopyResource(entry.second, resource);
		else context->CopySubresourceRegion(entry.second, 0, 0, 0, 0, resource, entry.first, nullptr);
	}
}

E_SurfaceFormat GetSurfaceFormatFromTexture(const E_TextureFormat format)
{
	return (E_SurfaceFormat)format;
}

U32 GetSurfacePitchFromWidth(U32 width, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample)
{
	U32 pitch = 0;

	switch(format)
	{
	case E_SurfaceFormat_L8:
	case E_SurfaceFormat_A8:
		pitch = width * 1;
		break;
	case E_SurfaceFormat_D16:
		pitch = width * 2;
		break;
	case E_SurfaceFormat_A8R8G8B8:
	case E_SurfaceFormat_D24S8:
	case E_SurfaceFormat_R32F:
		pitch = width * 4;
		break;
	case E_SurfaceFormat_A16B16G16R16F:
		pitch = width * 8;
		break;
	case E_SurfaceFormat_A32B32G32R32F:
		pitch = width * 16;
		break;
	default:
		assert(0 && "Unsupported surface format");
		break;
	}

	switch(multisample)
	{
	case E_Multisample_None:
		break;
	case E_Multisample_2:
	case E_Multisample_4:
		pitch = pitch * 2;
		break;
	default:
		assert(0 && "Unsupported surface multisample");
		break;
	}

	return pitch;
}

U32 GetSurfaceSizeFromHeightAndPitch(U32 height, const U32 pitch, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample)
{
	switch(multisample)
	{
	case E_Multisample_None:
	case E_Multisample_2:
		break;
	case E_Multisample_4:
		height = height * 2;
		break;
	default:
		assert(0 && "Unsupported surface multisample");
		break;
	}

	U32 size = pitch * height;

	return size;
}

U32 GetSurfaceSize(const U32 width, const U32 height, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample)
{

	const U32 pitch = GetSurfacePitchFromWidth(width, flags, format, multisample);
	return GetSurfaceSizeFromHeightAndPitch(height, pitch, flags, format, multisample);
}

void GetSurfaceSizeAndPitch(const U32 width, const U32 height, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample, U32& size, U32& pitch)
{
	pitch = GetSurfacePitchFromWidth(width, flags, format, multisample);
	size = GetSurfaceSize(width, height, flags, format, multisample);
}

U32 GetTexturePitchFromWidth(U32 width, const U32 flags, const E_TextureFormat format)
{
	U32 pitch = 0;

	switch(format)
	{
	case E_TextureFormat_DXT1:
		pitch = ((width + 3) / 4) * 8;
		break;
	case E_TextureFormat_DXT3:
	case E_TextureFormat_DXT5:
		pitch = ((width + 3) / 4) * 16;	
		break;
	case E_TextureFormat_L8:
	case E_TextureFormat_A8:
		pitch = width * 1;
		break;
	case E_TextureFormat_D16:
		pitch = width * 2;
		break;
	case E_TextureFormat_A8R8G8B8:
	case E_TextureFormat_X8R8G8B8:
	case E_TextureFormat_D24S8:
	case E_TextureFormat_R32F:
		pitch = width * 4;
		break;
	case E_TextureFormat_A16B16G16R16F:
		return width * 8;
	case E_TextureFormat_A32B32G32R32F:
		return width * 16;
	default:
		assert(0 && "Unsupported texture format");
		break;
	}

	return pitch;
}

U32 GetTextureSizeFromHeightAndPitch(U32 height, const U32 pitch, const U32 flags, const E_TextureFormat format)
{
	switch(format)
	{
	case E_TextureFormat_DXT1:
	case E_TextureFormat_DXT3:
	case E_TextureFormat_DXT5:
		height = ((height + 3) / 4);
		break;
	default:
		break;
	}

	U32 size = pitch * height;

	return size;
}


U32 GetTexturePitch(U32 width, const U32 flags, const E_TextureFormat format)
{
	return GetTexturePitchFromWidth(width, flags, format);
}

U32 GetTextureRowPitch(U32 width, const U32 flags, const E_TextureFormat format)
{
	return GetTexturePitchFromWidth(width, flags, format);
}

U32 GetTextureSlicePitch(U32 width, U32 height, U32 levels, const U32 flags, const E_TextureFormat format)
{
	U32 pitch = GetTexturePitchFromWidth(width, flags, format);
	U32 size = GetTextureSizeFromHeightAndPitch(height, pitch, flags, format);
	return size * levels;
}

U32 GetTexture2DSize(const U32 width, const U32 height, const U32 flags, const E_TextureFormat format)
{

	const U32 pitch = GetTexturePitchFromWidth(width, flags, format);
	return GetTextureSizeFromHeightAndPitch(height, pitch, flags, format);
}

U32 GetTexture2DSize(U32 width, U32 height, const U32 levels, const U32 flags, const E_TextureFormat format)
{

	U32 size = 0;
	for(U32 level=0; level<levels; level++)
	{
		size += GetTexture2DSize(width, height, flags, format);
		if(width > 1)
			width >>= 1;
		if(height > 1)
			height >>= 1;
	}
	return size;
}

void GetTexture2DSizeAndPitch(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch)
{
	pitch = GetTexturePitchFromWidth(width, flags, format);
	size = GetTexture2DSize(width, height, levels, flags, format);
}

U32 GetTexture3DSize(const U32 width, const U32 height, const U32 depth, const U32 flags, const E_TextureFormat format)
{

	return GetTexture2DSize(width, height, flags, format) * depth;
}

U32 GetTexture3DSize(U32 width, U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format)
{

	U32 size = 0;
	for(U32 level = 0; level < levels; level++)
	{
		size += GetTexture3DSize(width, height, depth, flags, format);
		if(width > 1)
			width >>= 1;
		if(height > 1)
			height >>= 1;
	}
	return size;
}

void GetTexture3DSizeAndPitch(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch)
{
	pitch = GetTexturePitchFromWidth(width, flags, format);
	size = GetTexture3DSize(width, height, depth, levels, flags, format);
}

U32 GetTextureCubeSize(const U32 width, const U32 flags, const E_TextureFormat format)
{

	U32 size = 0;
	for(U32 i=0; i<6; i++)
	{
		size += GetTexture2DSize(width, width, flags, format);
	}
	return size;
}

U32 GetTextureCubeSize(U32 width, const U32 levels, const U32 flags, const E_TextureFormat format)
{

	U32 size = 0;
	for(U32 i=0; i<6; i++)
	{
		size += GetTexture2DSize(width, width, levels, flags, format);
	}
	return size;
}

void GetTextureCubeSizeAndPitch(const U32 width, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch)
{
	pitch = GetTexturePitchFromWidth(width, flags, format);
	size = GetTextureCubeSize(width, levels, flags, format);
}

U32 GetTexture2DLevelOffset(const U32 level, U32 width, U32 height, const U32 levels, const U32 flags, const E_TextureFormat format)
{

	U32 offset = 0;

	for(U32 i=0; i<level; i++)
	{
		if(i >= levels)
			return offset;

		const U32 size = GetTexture2DSize(width, height, flags, format);

		offset += size;

		if(width > 1)
			width /= 2;
		if(height > 1)
			height /= 2;
	}

	return offset;
}

U32 GetTexture3DLevelOffset(const U32 level, U32 width, U32 height, U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format)
{

	U32 offset = 0;

	for(U32 i=0; i<level; i++)
	{
		if(i >= levels)
			return offset;

		const U32 size = GetTexture3DSize(width, height, depth, flags, format);

		offset += size;

		if(width > 1)
			width /= 2;
		if(height > 1)
			height /= 2;
		if(depth > 1)
			depth /= 2;
	}

	return offset;
}

U32 GetTextureCubeLevelOffset(const E_CubeMapFace face, const U32 level, U32 width, const U32 levels, const U32 flags, const E_TextureFormat format)
{

	U32 offset = 0;

	for(S32 f=0; f<face; f++)
	{
		const U32 size = GetTexture2DSize(width, width, levels, flags, format);

		offset += size;
	}

	for(U32 l=0; l<level; l++)
	{
		if(l >= levels)
			return offset;

		const U32 size = GetTextureCubeSize(width, flags, format);

		offset += size;

		if(width > 1)
			width /= 2;
	}

	return offset;
}


bool DebugCheckGPUTexture(const GPUTexture* pgpuTexture)
{
	if(!pgpuTexture)
		return true;

	
	return true;
}

bool DebugCheckTexture(const C_Texture* pTexture)
{
	if(!pTexture)
		return true;
	if(!pTexture->IsCreated())
		return true;
	return DebugCheckGPUTexture((GPUTexture*)pTexture->GetGPUTexture());
}

const Util::C_String GetD3DFormatAsString(const D3DFORMAT format)
{
	switch(format)
	{
	case D3DFMT_A8R8G8B8:
		return "A8R8G8B8";
	case D3DFMT_X8R8G8B8:
		return "X8R8G8B8";
	case D3DFMT_D24S8:
		return "D24S8";
	case D3DFMT_D24FS8:
		return "D24FS8";
	default:
		return "UNKNOWN";
	}
}

const Util::C_String GetD3DMultisampleTypeAsString(const D3DMULTISAMPLE_TYPE multisampleType)
{
	switch(multisampleType)
	{
	case D3DMULTISAMPLE_NONE:
		return "NONE";
	#ifdef PLATFORM_PC
    case D3DMULTISAMPLE_NONMASKABLE:
		return "NONMASKABLE";
	#endif
    case D3DMULTISAMPLE_2_SAMPLES:
		return "2_SAMPLES";
    #ifdef PLATFORM_PC
    case D3DMULTISAMPLE_3_SAMPLES:
		return "3_SAMPLES";
	#endif
    case D3DMULTISAMPLE_4_SAMPLES:
		return "4_SAMPLES";
    #ifdef PLATFORM_PC
	case D3DMULTISAMPLE_5_SAMPLES:
		return "5_SAMPLES";
	case D3DMULTISAMPLE_6_SAMPLES:
		return "6_SAMPLES";
    case D3DMULTISAMPLE_7_SAMPLES:
		return "7_SAMPLES";
    case D3DMULTISAMPLE_8_SAMPLES:
		return "8_SAMPLES";
    case D3DMULTISAMPLE_9_SAMPLES:
		return "9_SAMPLES";
    case D3DMULTISAMPLE_10_SAMPLES:
		return "10_SAMPLES";
    case D3DMULTISAMPLE_11_SAMPLES:
		return "11_SAMPLES";
    case D3DMULTISAMPLE_12_SAMPLES:
		return "12_SAMPLES";
    case D3DMULTISAMPLE_13_SAMPLES:
		return "13_SAMPLES";
    case D3DMULTISAMPLE_14_SAMPLES:
		return "14_SAMPLES";
    case D3DMULTISAMPLE_15_SAMPLES:
		return "15_SAMPLES";
    case D3DMULTISAMPLE_16_SAMPLES:
		return "16_SAMPLES";
	#endif
    default:
		return "UNKNOWN";
	}
}

const Util::C_String GetD3DSwapEffectAsString(const D3DSWAPEFFECT swapEffect)
{
	switch(swapEffect)
	{
	case D3DSWAPEFFECT_DISCARD:
		return "DISCARD";
	case D3DSWAPEFFECT_FLIP:
		return "FLIP";
	case D3DSWAPEFFECT_COPY:
		return "COPY";
	default:
		return "UNKNOWN";
	}
}

const Util::C_String GetD3DPresentFlagsAsString(const DWORD presentFlags)
{
	Util::C_String temp = "";

	#ifdef PLATFORM_PC
	if(presentFlags & D3DPRESENTFLAG_DEVICECLIP)
		temp += "DEVICECLIP/";
	#endif
	if(presentFlags & D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL)
		temp += "DISCARD_DEPTHSTENCIL/";
	#ifdef PLATFORM_PC
	if(presentFlags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)
		temp += "LOCKABLE_BACKBUFFER/";
	if(presentFlags & D3DPRESENTFLAG_NOAUTOROTATE)
		temp += "NOAUTOROTATE/";
	if(presentFlags & D3DPRESENTFLAG_UNPRUNEDMODE)
		temp += "UNPRUNEDMODE/";
	if(presentFlags & D3DPRESENTFLAG_VIDEO)
		temp += "VIDEO/";
	#endif

	return temp;
}

const Util::C_String GetD3DPresentIntervalAsString(const UINT presentInterval)
{
	switch(presentInterval)
	{
	//case D3DPRESENT_DONOTFLIP:
		//return "DONOTFLIP";
	//case D3DPRESENT_DONOTWAIT:
		//return "DONOTWAIT";
	//case D3DPRESENT_FLIPRESTART:
		//return "FLIPRESTART";
	case D3DPRESENT_INTERVAL_DEFAULT:
		return "INTERVAL_DEFAULT";
	case D3DPRESENT_INTERVAL_ONE:
		return "INTERVAL_ONE";
	case D3DPRESENT_INTERVAL_TWO:
		return "INTERVAL_TWO";
	case D3DPRESENT_INTERVAL_THREE:
		return "INTERVAL_THREE";
	case D3DPRESENT_INTERVAL_FOUR:
		return "INTERVAL_FOUR";
	case D3DPRESENT_INTERVAL_IMMEDIATE:
		return "INTERVAL_IMMEDIATE";
	//case D3DPRESENT_LINEAR_CONTENT:
		//return "LINEAR_CONTENT";
	#ifdef PLATFORM_PC
	case D3DPRESENT_VIDEO_RESTRICT_TO_MONITOR:
		return "VIDEO_RESTRICT_TO_MONITOR";
	#endif
	default:
		return "UNKNOWN";
	}
}

const Util::C_String GetFlagsAsString(const U32 flags)
{
	Util::C_String str = "";
	
	#define ADD_FLAG_TO_STR(_str, _flags, _flag, _name) if(_flags & _flag) { if(_str != "") _str += "|"; _str += _name; }
	
	ADD_FLAG_TO_STR(str, flags, E_Flags_Video,			"Video");
	ADD_FLAG_TO_STR(str, flags, E_Flags_System,			"System");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Indirect,		"Indirect");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Dynamic,		"Dynamic");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Colour,			"Colour");
	ADD_FLAG_TO_STR(str, flags, E_Flags_DepthStencil,	"DepthStencil");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Raw,			"Raw");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Tiled,			"Tiled");
	ADD_FLAG_TO_STR(str, flags, E_Flags_Linear,			"Linear");
	ADD_FLAG_TO_STR(str, flags, E_Flags_SRGB,			"SRGB");
	ADD_FLAG_TO_STR(str, flags, E_Flags_DoNotTrack,		"DoNotTrack");
	ADD_FLAG_TO_STR(str, flags, E_Flags_DoNotReload,	"DoNotReload");

	return str;
}

const Util::C_String GetIndexFormatAsString(const E_IndexFormat indexFormat)
{
	switch(indexFormat)
	{
	case E_IndexFormat_16:						return "16";	
	case E_IndexFormat_32:						return "32";		
	default:									return "Unknown";
	}
}

const Util::C_String GetMultisampleAsString(const E_Multisample multisample)
{
	switch(multisample)
	{
	case E_Multisample_None:					return "1xAA";	
	case E_Multisample_2:						return "2xAA";		
	case E_Multisample_4:						return "4xAA";		
	default:									return "Unknown";
	}
}

const Util::C_String GetSurfaceFormatAsString(const E_SurfaceFormat format)
{
	switch(format)
	{
	// 8bpp:
	case E_SurfaceFormat_A8: 					return "A8";
	case E_SurfaceFormat_L8: 					return "L8";

	// 16bpp:
	case E_SurfaceFormat_R5G6B5: 				return "R5G6B5";
	case E_SurfaceFormat_L6V5U5: 				return "L6V5U5";
	case E_SurfaceFormat_X1R5G5B5: 				return "X1R5G5B5";
	case E_SurfaceFormat_A1R5G5B5: 				return "A1R5G5B5";
	case E_SurfaceFormat_A4R4G4B4: 				return "A4R4G4B4";
	case E_SurfaceFormat_X4R4G4B4: 				return "X4R4G4B4";
	case E_SurfaceFormat_A8L8: 					return "A8L8";
	case E_SurfaceFormat_V8U8: 					return "V8U8";
	case E_SurfaceFormat_D16: 					return "D16";
	case E_SurfaceFormat_L16: 					return "L16";
	case E_SurfaceFormat_R16F: 					return "R16F";
	case E_SurfaceFormat_UYVY: 					return "UYVY";
	case E_SurfaceFormat_G8R8_G8B8: 			return "G8R8_G8B8";
	case E_SurfaceFormat_R8G8_B8G8: 			return "R8G8_B8G8";
	case E_SurfaceFormat_YUY2:					return "YUY2";

	// 32bpp:
	case E_SurfaceFormat_A8R8G8B8: 				return "A8R8G8B8";
	case E_SurfaceFormat_X8R8G8B8: 				return "X8R8G8B8";
	case E_SurfaceFormat_A8B8G8R8: 				return "A8B8G8R8";
	case E_SurfaceFormat_X8B8G8R8: 				return "X8B8G8R8";
	case E_SurfaceFormat_X8L8V8U8: 				return "X8L8V8U8";
	case E_SurfaceFormat_Q8W8V8U8: 				return "Q8W8V8U8";
	case E_SurfaceFormat_A2R10G10B10: 			return "A2R10G10B10";
	case E_SurfaceFormat_A2B10G10R10: 			return "A2B10G10R10";
	case E_SurfaceFormat_A2W10V10U10: 			return "A2W10V10U10";
	case E_SurfaceFormat_G16R16: 				return "G16R16";
	case E_SurfaceFormat_V16U16: 				return "V16U16";
	case E_SurfaceFormat_G16R16F:				return "G16R16F";
	case E_SurfaceFormat_R32F:					return "R32F";
	case E_SurfaceFormat_D24S8:					return "D24S8";
	case E_SurfaceFormat_D24X8:					return "D24X8";
	case E_SurfaceFormat_D24FS8:				return "D24FS8";
	case E_SurfaceFormat_D32:					return "D32";

	// 64bpp:
	case E_SurfaceFormat_A16B16G16R16: 			return "A16B16G16R16";
	case E_SurfaceFormat_Q16W16V16U16: 			return "Q16W16V16U16";
	case E_SurfaceFormat_A16B16G16R16F:			return "A16B16G16R16F";
	case E_SurfaceFormat_G32R32F:				return "G32R32F";

	// 128bpp:
	case E_SurfaceFormat_A32B32G32R32F:			return "A32B32G32R32F";
	
	// EDRAM only:

	// Front buffer formats have to be little endian:

	default:									return "Unknown";
	}
}

const Util::C_String GetTextureFormatAsString(const E_TextureFormat format)
{
	switch(format)
	{
	// DXT:
	case E_TextureFormat_DXT1: 					return "DXT1";
	case E_TextureFormat_DXT3: 					return "DXT3";
	case E_TextureFormat_DXT5: 					return "DXT5";

	// 8bpp:
	case E_TextureFormat_A8: 					return "A8";
	case E_TextureFormat_L8: 					return "L8";

	// 16bpp:
	case E_TextureFormat_R5G6B5: 				return "R5G6B5";
	case E_TextureFormat_L6V5U5: 				return "L6V5U5";
	case E_TextureFormat_X1R5G5B5: 				return "X1R5G5B5";
	case E_TextureFormat_A1R5G5B5: 				return "A1R5G5B5";
	case E_TextureFormat_A4R4G4B4: 				return "A4R4G4B4";
	case E_TextureFormat_X4R4G4B4: 				return "X4R4G4B4";
	case E_TextureFormat_A8L8: 					return "A8L8";
	case E_TextureFormat_V8U8: 					return "V8U8";
	case E_TextureFormat_D16: 					return "D16";
	case E_TextureFormat_L16: 					return "L16";
	case E_TextureFormat_R16F: 					return "R16F";
	case E_TextureFormat_UYVY: 					return "UYVY";
	case E_TextureFormat_G8R8_G8B8: 			return "G8R8_G8B8";
	case E_TextureFormat_R8G8_B8G8: 			return "R8G8_B8G8";
	case E_TextureFormat_YUY2:					return "YUY2";

	// 32bpp:
	case E_TextureFormat_A8R8G8B8: 				return "A8R8G8B8";
	case E_TextureFormat_X8R8G8B8: 				return "X8R8G8B8";
	case E_TextureFormat_A8B8G8R8: 				return "A8B8G8R8";
	case E_TextureFormat_X8B8G8R8: 				return "X8B8G8R8";
	case E_TextureFormat_X8L8V8U8: 				return "X8L8V8U8";
	case E_TextureFormat_Q8W8V8U8: 				return "Q8W8V8U8";
	case E_TextureFormat_A2R10G10B10: 			return "A2R10G10B10";
	case E_TextureFormat_A2B10G10R10: 			return "A2B10G10R10";
	case E_TextureFormat_A2W10V10U10: 			return "A2W10V10U10";
	case E_TextureFormat_G16R16: 				return "G16R16";
	case E_TextureFormat_V16U16: 				return "V16U16";
	case E_TextureFormat_G16R16F:				return "G16R16F";
	case E_TextureFormat_R32F:					return "R32F";
	case E_TextureFormat_D24S8:					return "D24S8";
	case E_TextureFormat_D24X8:					return "D24X8";
	case E_TextureFormat_D24FS8:				return "D24FS8";
	case E_TextureFormat_D32:					return "D32";

	// 64bpp:
	case E_TextureFormat_A16B16G16R16: 			return "A16B16G16R16";
	case E_TextureFormat_Q16W16V16U16: 			return "Q16W16V16U16";
	case E_TextureFormat_A16B16G16R16F:			return "A16B16G16R16F";
	case E_TextureFormat_G32R32F:				return "G32R32F";

	// 128bpp:
	case E_TextureFormat_A32B32G32R32F:			return "A32B32G32R32F";
	
	// EDRAM only:

	// Front buffer formats have to be little endian:

	default:									return "Unknown";
	}
}

const Util::C_String GetTextureTypeAsString(const E_TextureType type)
{
	switch(type)
	{
	case E_TextureType_2D:						return "2D";
	case E_TextureType_3D:						return "3D";	
	case E_TextureType_Cube:					return "Cube";
	default:									return "Unknown";
	}
}

C_Viewport::C_Viewport()
{
	TopLeftX		= 0;
	TopLeftY		= 0;
	Width	= 1;
	Height	= 1;
	MinDepth	= 0.0f;
	MaxDepth	= 1.0f;
}

C_Viewport::C_Viewport(const U32 x, const U32 y, const U32 width, const U32 height)
{
	TopLeftX		= x;
	TopLeftY		= y;
	Width	= width;
	Height	= height;
	MinDepth	= 0.0f;
	MaxDepth	= 1.0f;
}

C_Viewport::C_Viewport(const U32 x, const U32 y, const U32 width, const U32 height, const F32 minZ, const F32 maxZ)
{
	TopLeftX		= x;
	TopLeftY		= y;
	Width	= width;
	Height	= height;
	MinDepth	= minZ;
	MaxDepth	= maxZ;
}

// C_VertexElement

C_VertexElement::C_VertexElement()
{
	Stream		= 0xFF;
	Offset		= 0;
	Type		= D3DDECLTYPE_UNUSED;
	Method		= 0;
	Usage		= 0;
	UsageIndex	= 0;
}

C_VertexElement::C_VertexElement(const U16 stream, const U32 offset, const U32 type, const U8 usage, const U8 usageIndex)
{
	Stream		= stream;
	Offset		= offset;
	Type		= type;
	Method		= 0;
	Usage		= usage;
	UsageIndex	= usageIndex;
}

void C_VertexElement::SetStream(const U16 stream)
{
	Stream = stream;
}

void C_VertexElement::SetOffset(const U32 offset)
{
	Offset = offset;
}

void C_VertexElement::SetType(const U32 type)
{
	Type = type;
}

void C_VertexElement::SetUsage(const U8 usage)
{
	Usage = usage;
}

void C_VertexElement::SetUsageIndex(const U8 usageIndex)
{
	UsageIndex = usageIndex;
}

// C_VertexDeclaration

C_VertexDeclaration::C_VertexDeclaration() : m_pd3dVertexDeclaration(nullptr), m_ElementCount(0) {}
C_VertexDeclaration::~C_VertexDeclaration() {}
S32 C_VertexDeclaration::GetElementCount() const { return static_cast<S32>(m_ElementCount); }
bool C_VertexDeclaration::GetElements(U32 count, C_VertexElement* elements) const
{
    if (!elements || count != m_ElementCount) return false;
    for (U32 i = 0; i < count; ++i) elements[i] = m_Elements[i];
    return true;
}
U32 C_VertexDeclaration::GetStride(U32 stream) const
{
    U32 stride = 0;
    for (U32 i = 0; i < m_ElementCount; ++i)
        if (m_Elements[i].GetStream() == stream)
            stride = std::max(stride, m_Elements[i].GetOffset() + m_Elements[i].GetSize());
    return stride;
}

// C_VertexBuffer

C_VertexBuffer::C_VertexBuffer() : m_Flags(0)
{
	m_pd3dVertexBuffer = NULL;
}

C_VertexBuffer::~C_VertexBuffer()
{
}

void C_VertexBuffer::SetSize(const U32 size)
{
}

void C_VertexBuffer::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

void C_VertexBuffer::SetAddress(void* pAddress)
{
}

void* C_VertexBuffer::Lock(const U32 offset, const U32 size, const U32 flags)
{
	BIZ_ASSERT(IsCreated());
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	const bool ok = LockResource(m_pd3dVertexBuffer, 0, true, mapped, flags);
	BIZ_ASSERT_MSG(ok, "Failed to lock the d3d11 vertex buffer");
	return ok ? static_cast<U8*>(mapped.pData) + offset : nullptr;
}

void* C_VertexBuffer::Lock()
{
	return Lock(0, 0, 0);
}

void C_VertexBuffer::Unlock()
{
	BIZ_ASSERT(IsCreated());
	UnlockStagingResource(m_pd3dVertexBuffer, 0);
}

// C_IndexBuffer

C_IndexBuffer::C_IndexBuffer() : m_Flags(0)
{
	m_pd3dIndexBuffer = NULL;
}

C_IndexBuffer::~C_IndexBuffer()
{
}

void C_IndexBuffer::SetSize(const U32 size)
{
}

void C_IndexBuffer::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

void C_IndexBuffer::SetAddress(void* pAddress)
{
}

void* C_IndexBuffer::Lock(const U32 offset, const U32 size, const U32 flags)
{
	BIZ_ASSERT(IsCreated());
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	const bool ok = LockResource(m_pd3dIndexBuffer, 0, true, mapped, flags);
	BIZ_ASSERT_MSG(ok, "Failed to lock the d3d11 index buffer");
	return ok ? static_cast<U8*>(mapped.pData) + offset : nullptr;
}

void* C_IndexBuffer::Lock()
{
	return Lock(0, 0, 0);
}

void C_IndexBuffer::Unlock()
{
	BIZ_ASSERT(IsCreated());
	UnlockStagingResource(m_pd3dIndexBuffer, 0);
}

// C_ShaderSemantic

C_ShaderSemantic::C_ShaderSemantic() :
	m_Usage(UCHAR_MAX),
	m_UsageIndex(UCHAR_MAX)
{
}

// C_ShaderParam

C_ShaderParam::C_ShaderParam() :
	m_RegisterIndex(UINT_MAX),
	m_RegisterCount(0)
{
}

C_ShaderParam::C_ShaderParam(const U32 registerIndex, const U32 registerCount) :
	m_RegisterIndex(registerIndex),
	m_RegisterCount(registerCount)
{
}

void C_ShaderParam::SetupFromFunc(const char* name, const U8* func, U32 size)
{
    if (!name || !func || !size) return;
    ID3D11ShaderReflection* reflection = nullptr;
    if (FAILED(D3DReflect(func, size, IID_ID3D11ShaderReflection, (void**)&reflection))) return;
    D3D11_SHADER_DESC shader = {};
    if (SUCCEEDED(reflection->GetDesc(&shader)))
        for (UINT bufferIndex = 0; bufferIndex < shader.ConstantBuffers; ++bufferIndex)
        {
            ID3D11ShaderReflectionConstantBuffer* buffer = reflection->GetConstantBufferByIndex(bufferIndex);
            D3D11_SHADER_BUFFER_DESC bufferDesc = {};
            if (FAILED(buffer->GetDesc(&bufferDesc))) continue;
            for (UINT variableIndex = 0; variableIndex < bufferDesc.Variables; ++variableIndex)
            {
                ID3D11ShaderReflectionVariable* variable = buffer->GetVariableByIndex(variableIndex);
                D3D11_SHADER_VARIABLE_DESC desc = {};
                if (SUCCEEDED(variable->GetDesc(&desc)) && strcmp(desc.Name, name) == 0)
                {
                    m_RegisterIndex = desc.StartOffset / 16;
                    m_RegisterCount = (desc.Size + 15) / 16;
                    reflection->Release();
                    return;
                }
            }
        }
    reflection->Release();
}

// C_VertexShader

C_VertexShader::C_VertexShader() : m_pd3dVertexShader(NULL), m_Flags(0)
{
}

C_VertexShader::~C_VertexShader()
{
}

void C_VertexShader::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

const C_ShaderParam C_VertexShader::GetParamFromName(const char* name) const
{
    C_ShaderParam param;
    if (!m_Bytecode.empty()) param.SetupFromFunc(name, m_Bytecode.data(), m_Bytecode.size());
    return param;
}

const C_ShaderParam C_VertexShader::GetParameter(const S32 index, const char* name) const
{
	return GetParamFromName(name);
}

bool C_VertexShader::GetSemantics(Btl::Vector<C_ShaderSemantic>& semantics)
{
    if (m_Bytecode.empty()) return false;
    ID3D11ShaderReflection* reflection = nullptr;
    if (FAILED(D3DReflect(m_Bytecode.data(), m_Bytecode.size(),
                          IID_ID3D11ShaderReflection, (void**)&reflection))) return false;
    D3D11_SHADER_DESC shader = {};
    const bool valid = SUCCEEDED(reflection->GetDesc(&shader));
    if (valid)
        for (UINT i = 0; i < shader.InputParameters; ++i)
        {
            D3D11_SIGNATURE_PARAMETER_DESC desc = {};
            if (SUCCEEDED(reflection->GetInputParameterDesc(i, &desc)))
            {
                C_ShaderSemantic semantic;
                semantic.m_Usage = static_cast<U8>(desc.SystemValueType == D3D_NAME_POSITION ?
                    D3DDECLUSAGE_POSITION : D3DDECLUSAGE_TEXCOORD);
                semantic.m_UsageIndex = static_cast<U8>(desc.SemanticIndex);
                semantics.PushBack(semantic);
            }
        }
    reflection->Release();
    return valid;
}

// C_PixelShader

C_PixelShader::C_PixelShader() : m_pd3dPixelShader(NULL), m_Flags(0)
{
}

C_PixelShader::~C_PixelShader()
{
}

void C_PixelShader::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

void C_PixelShader::SetAddress(U8* pAddress)
{
}

const C_ShaderParam C_PixelShader::GetParamFromName(const char* name) const
{
    C_ShaderParam param;
    if (!m_Bytecode.empty()) param.SetupFromFunc(name, m_Bytecode.data(), m_Bytecode.size());
    return param;
}

const C_ShaderParam C_PixelShader::GetParameter(const S32 index, const char* name) const
{
	return GetParamFromName(name);
}

const C_ShaderParam C_PixelShader::GetParameterArray(const S32 index, const char* name, const U32 count) const
{
	return GetParamFromName(name);
}

// C_LockedRect

C_LockedRect::C_LockedRect()
{
	pBits = NULL;
	pitch = 0;
}

// C_ConstLockedRect

C_ConstLockedRect::C_ConstLockedRect()
{
	pBits = NULL;
	pitch = 0;
}

// C_LockedBox

C_LockedBox::C_LockedBox()
{
	pBits = NULL;
	rowPitch = 0;
	slicePitch = 0;
}

// C_ConstLockedBox

C_ConstLockedBox::C_ConstLockedBox()
{
	pBits = NULL;
	rowPitch = 0;
	slicePitch = 0;
}

// C_SurfaceParams

C_SurfaceParams::C_SurfaceParams()
{
}

C_SurfaceParams::~C_SurfaceParams()
{
}

void C_SurfaceParams::OverSurface(const C_Surface* pSurface)
{
}

void C_SurfaceParams::AfterSurface(const C_Surface* pSurface)
{
}

void C_SurfaceParams::BeforeSurface(const C_Surface* pSurface)
{
}

// C_Surface

C_Surface::C_Surface() : m_Flags(0), m_Subresource(0)
{
	m_pd3dSurface = NULL;
}

C_Surface::~C_Surface()
{
}

void C_Surface::SetSize(const U32 size)
{
}

void C_Surface::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

void C_Surface::SetAddress(U8* pAddress)
{
}

C_LockedRect C_Surface::LockRect(const U32 flags)
{
    BIZ_ASSERT(IsCreated());
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    const bool ok = LockResource(m_pd3dSurface, m_Subresource, true, mapped);
    BIZ_ASSERT_MSG(ok, "Failed to lock the d3d11 surface");
    C_LockedRect rect = {};
    if (ok) { rect.pBits = static_cast<U8*>(mapped.pData); rect.pitch = mapped.RowPitch; }
    return rect;
}

void C_Surface::UnlockRect()
{
    BIZ_ASSERT(IsCreated());
    UnlockStagingResource(m_pd3dSurface, m_Subresource);
}

// C_Texture

C_Texture::C_Texture() : m_Size(0), m_Flags(0)
{
	m_pd3dTexture = NULL;
}

C_Texture::~C_Texture()
{
}

void C_Texture::SetSize(const U32 size)
{
	m_Size = size;
}

void C_Texture::SetFlags(const U32 flags)
{
	m_Flags = flags;
}

void C_Texture::SetAddress(U8* pAddress)
{
}

void C_Texture::SetSRGB(const bool enable)
{
	if(enable)
	{
		m_Flags |= E_Flags_SRGB;

	}
	else
	{
		m_Flags &= ~E_Flags_SRGB;

	}
}

C_LockedRect C_Texture::LockRect2D(const U32 level, const U32 flags)
{
    BIZ_ASSERT(IsCreated() && GetType() == E_TextureType_2D);
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    const bool ok = LockResource(GetGPUTexture2D(), level, true, mapped);
    BIZ_ASSERT_MSG(ok, "Failed to lock d3d11 texture 2d");
    C_LockedRect rect = {};
    if (ok) { rect.pBits = static_cast<U8*>(mapped.pData); rect.pitch = mapped.RowPitch; }
    return rect;
}

C_ConstLockedRect C_Texture::LockRect2D(const U32 level, const U32 flags) const
{
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    const bool ok = LockResource(const_cast<GPUTexture2D*>(GetGPUTexture2D()), level, false, mapped);
    BIZ_ASSERT_MSG(ok, "Failed to lock d3d11 texture 2d");
    C_ConstLockedRect rect = {};
    if (ok) { rect.pBits = static_cast<const U8*>(mapped.pData); rect.pitch = mapped.RowPitch; }
    return rect;
}

void C_Texture::UnlockRect2D(const U32 level) const
{
    UnlockStagingResource(const_cast<GPUTexture2D*>(GetGPUTexture2D()), level);
}

C_LockedBox C_Texture::LockBox3D(const U32 level, const U32 flags)
{
    BIZ_ASSERT(IsCreated() && GetType() == E_TextureType_3D);
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    const bool ok = LockResource(GetGPUTexture3D(), level, true, mapped);
    BIZ_ASSERT_MSG(ok, "Failed to lock d3d11 texture 3d");
    C_LockedBox box = {};
    if (ok) { box.pBits = static_cast<U8*>(mapped.pData); box.rowPitch = mapped.RowPitch; box.slicePitch = mapped.DepthPitch; }
    return box;
}

C_ConstLockedBox C_Texture::LockBox3D(const U32 level, const U32 flags) const
{
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    const bool ok = LockResource(const_cast<GPUTexture3D*>(GetGPUTexture3D()), level, false, mapped);
    BIZ_ASSERT_MSG(ok, "Failed to lock d3d11 texture 3d");
    C_ConstLockedBox box = {};
    if (ok) { box.pBits = static_cast<const U8*>(mapped.pData); box.rowPitch = mapped.RowPitch; box.slicePitch = mapped.DepthPitch; }
    return box;
}

void C_Texture::UnlockBox3D(const U32 level) const
{
    UnlockStagingResource(const_cast<GPUTexture3D*>(GetGPUTexture3D()), level);
}

C_LockedRect C_Texture::LockRectCube(const U32 face, const U32 level, const U32 flags)
{
    BIZ_ASSERT(IsCreated() && GetType() == E_TextureType_Cube);
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    const bool ok = LockResource(GetGPUTextureCube(), face * GetLevels() + level, true, mapped);
    BIZ_ASSERT_MSG(ok, "Failed to lock d3d11 cube texture");
    C_LockedRect rect = {};
    if (ok) { rect.pBits = static_cast<U8*>(mapped.pData); rect.pitch = mapped.RowPitch; }
    return rect;
}

C_ConstLockedRect C_Texture::LockRectCube(const U32 face, const U32 level, const U32 flags) const
{
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    const bool ok = LockResource(const_cast<GPUTextureCube*>(GetGPUTextureCube()), face * GetLevels() + level, false, mapped);
    BIZ_ASSERT_MSG(ok, "Failed to lock d3d11 cube texture");
    C_ConstLockedRect rect = {};
    if (ok) { rect.pBits = static_cast<const U8*>(mapped.pData); rect.pitch = mapped.RowPitch; }
    return rect;
}

void C_Texture::UnlockRectCube(const U32 face, const U32 level) const
{
    UnlockStagingResource(const_cast<GPUTextureCube*>(GetGPUTextureCube()), face * GetLevels() + level);
}

GPUTexture* C_Texture::Get()
{
	return m_pd3dTexture;

}
const Util::C_String C_Texture::GetFileName() const
{
	return "";
}

C_LockedRect C_Texture::Lock(const U32 level)
{
	BIZ_ASSERT(GetType() == E_ResourceType_Texture2D);

	return LockRect2D(level);
}

C_ConstLockedRect C_Texture::Lock(const U32 level) const
{
	BIZ_ASSERT(GetType() == E_ResourceType_Texture2D);

	return LockRect2D(level);
}

void C_Texture::Unlock(const U32 level) const
{
	BIZ_ASSERT(GetType() == E_ResourceType_Texture2D);
	
	UnlockRect2D(level);
}

// C_RenderTexture2D

C_RenderTexture2D::C_RenderTexture2D() : m_Flags(0)
{
#if DEVICE_D3D11_PC // [Blur1PCDX11] The native layout carries these slots too.
	m_TextureIndex = 0;
	m_SurfaceIndex = 0;
	m_Multisample = D3DMULTISAMPLE_NONE;	// [Blur2PC]
#endif
}

C_RenderTexture2D::~C_RenderTexture2D()
{
}

// C_RenderTextureCube

C_RenderTextureCube::C_RenderTextureCube() : m_Flags(0)
{
}

C_RenderTextureCube::~C_RenderTextureCube()
{
}

// C_ContextBase

C_ContextBase::C_ContextBase() :
	m_pd3dDevice(NULL)
{
}

C_ContextBase::C_ContextBase(GPUDevice* pd3dDevice) :
	m_pd3dDevice(pd3dDevice)
{
}

// C_Context

C_Context::C_Context() :
	m_pState(NULL)
{
}

C_Context::C_Context(GPUDevice* pd3dDevice) :
	C_ContextBase(pd3dDevice),
	m_pState(NULL)
{
}

// C_ContextNoCheck

C_ContextNoCheck::C_ContextNoCheck() :
	m_pState(NULL)
{
}

C_ContextNoCheck::C_ContextNoCheck(GPUDevice* pd3dDevice) :
	C_ContextBase(pd3dDevice),
	m_pState(NULL)
{
}

// C_ContextNoCheckNoTrack

C_ContextNoCheckNoTrack::C_ContextNoCheckNoTrack()
{
}

C_ContextNoCheckNoTrack::C_ContextNoCheckNoTrack(GPUDevice* pd3dDevice) :
	C_ContextBase(pd3dDevice)
{
}


} // namespace Display2
