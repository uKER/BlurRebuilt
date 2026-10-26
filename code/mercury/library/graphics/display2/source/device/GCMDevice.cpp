#include "StdAfx.h"
#include "Display2/Device/GCMDevice.h"
#include "Display2/Debug/GCMDebug.h"

namespace Display2
{

void GCMGraphicsHandler(const uint32_t val)
{
	LOGFATAL(g_Log, "A serious error has thrown in libgcm. Hit continue to get a gpu dump.\n");
	DebugBreak();
}

void GCMDebugCheckCallback(uint32_t* cmd)
{
	LOGFATAL(g_Log, "### Bad command 0x%X at memory location 0x%x ###\n", *cmd, cmd);
	DebugBreak();
}

C_TileInfoManager::~C_TileInfoManager()
{
	BIZ_ASSERT(m_List.Size() == 0);
}

void C_TileInfoManager::InsertTiledMemory(const C_TileInfo* pInfo)
{
	m_Lock.Wait();
	
	BIZ_ASSERT(pInfo);
	m_List.Insert(pInfo);
	
	m_Lock.Release();
}

void C_TileInfoManager::RemoveTiledMemory(const C_TileInfo* pInfo)
{
	m_Lock.Wait();
	
	BIZ_ASSERT(pInfo);
	m_List.Erase(pInfo);
	
	m_Lock.Release();
}

bool C_TileInfoManager::IsTiledMemory(const U32 size, const U32 offset, const E_Location location)
{
	m_Lock.Wait();

	C_TileInfoList::Iterator it = m_List.Begin();
	while(it != m_List.End())
	{
		const C_TileInfo* pInfo = *it;

		if(location == pInfo->location && !(offset >= pInfo->offset+pInfo->size || offset+size <= pInfo->offset))
		{
			LOGERROR(g_Log, "Writing to tiled memory with size %i offset %i location %i\n", size, offset, location);

			DumpTiledMemory();

			m_Lock.Release();

			return true;
		}

		it++;
	}

	m_Lock.Release();

	return false;
}

void C_TileInfoManager::DumpTiledMemory()
{
	U32 i = 0;
	C_TileInfoList::Iterator it = m_List.Begin();
	while(it != m_List.End())
	{
		const C_TileInfo* pInfo = *it;

		LOGINFO(g_Log, "%2i : index %2i location %2i offset %9i size %8i pitch %5i comp %2i base %4i bank %2i name %s\n",
			i,
			pInfo->index,
			pInfo->location,
			pInfo->offset,
			pInfo->size,
			pInfo->pitch,
			pInfo->comp,
			pInfo->base,
			pInfo->bank,
			pInfo->name.Chars());

		i++;
		it++;
	}
}

bool C_ThingyManager::Create(const U32 indexCount, const U32 memorySize)
{
	BIZ_ASSERT(indexCount > 0);
	m_Indices.Resize(indexCount);
	for(S32 i=0; i<m_Indices.Size(); i++)
		m_Indices[i] = false;

	BIZ_ASSERT(memorySize > 0);
	if(!m_Tracker.Initialise((U8*)0x8, memorySize, 1))
		return false;

	#if !CFG_GOLD
	BIZ_ASSERT(indexCount > 0);
	m_Names.Resize(indexCount);
	for(S32 i=0; i<m_Names.Size(); i++)
		m_Names[i] = "";
	#endif

	return true;
}

void C_ThingyManager::Destroy()
{
	m_Indices.Clear();
	m_Memory.Clear();
	m_Tracker.Shutdown();

	#if !CFG_GOLD
	m_Names.Clear();
	#endif
}

U32 C_ThingyManager::AllocIndex()
{
	for(S32 i=0; i<m_Indices.Size(); i++)
	{
		if(!m_Indices[i])
		{
			m_Indices[i] = true;
			return i;
		}
	}

	return UINT_MAX;
}

void C_ThingyManager::FreeIndex(const U32 index)
{
	BIZ_ASSERT(index < m_Indices.Size());
	BIZ_ASSERT(m_Indices[index]);
	m_Indices[index] = false;
}

U32 C_ThingyManager::AllocMemory(const U32 size)
{
	BIZ_ASSERT(size > 0);
	U8* pMemory = m_Tracker.Allocate(size, 1);
	if(pMemory)
		return (U32)pMemory - 0x8;
	else
		return UINT_MAX;
}

void C_ThingyManager::FreeMemory(const U32 size, const U32 address)
{
	m_Tracker.Free((U8*)address + 0x8, size);
}

#if !CFG_GOLD
void C_ThingyManager::Dump()
{
	for(S32 i=0; i<m_Names.Size(); i++)
	{
		LOGINFO(g_Log, "%2i : %s\n", i, m_Names[i].Chars());
	}
}
#endif

U32 C_TileManager::AllocIndex()
{
	U32 tileIndex = C_ThingyManager::AllocIndex();
	
	if(tileIndex != UINT_MAX)
	{
		BIZ_ASSERT(tileIndex < INVALID_TILE_INDEX);

		return tileIndex;
	}
	else
	{
		#if !CFG_GOLD
		Dump();
		#endif

		BIZ_ASSERT_FAIL("Out of tiled indices");

		return INVALID_TILE_INDEX;
	}
}

U32 C_TileManager::AllocMemory(const U32 sizeInBytes)
{
	BIZ_ASSERT(sizeInBytes % 64 == 0);
	
	U32 sizeInTiles = sizeInBytes / 65536;
	
	U32 startInTiles = C_ThingyManager::AllocMemory(sizeInTiles);
	
	if(startInTiles != UINT_MAX)
	{
		BIZ_ASSERT(startInTiles < INVALID_TILE_MEMORY);

		return startInTiles;
	}
	else
	{
		#if !CFG_GOLD
		Dump();
		#endif

		BIZ_ASSERT_FAIL("Out of tiled memory");

		return INVALID_TILE_MEMORY;
	}
}

void C_TileManager::FreeMemory(const U32 sizeInBytes, const U32 startInTiles)
{
	BIZ_ASSERT(sizeInBytes % 64 == 0);
	
	U32 sizeInTiles = sizeInBytes / 65536;
	
	C_ThingyManager::FreeMemory(sizeInTiles, startInTiles);
}

U32 C_ZcullManager::AllocIndex()
{
	U32 zcullIndex = C_ThingyManager::AllocIndex();
	
	if(zcullIndex != UINT_MAX)
	{
		BIZ_ASSERT(zcullIndex < INVALID_ZCULL_INDEX);

		return zcullIndex;
	}
	else
	{
		#if !CFG_GOLD
		Dump();
		#endif

		BIZ_ASSERT_FAIL("Out of zcull indices");

		return INVALID_ZCULL_INDEX;
	}
}

U32 C_ZcullManager::AllocMemory(const U32 sizeInBytes)
{
	U32 sizeInZcull = sizeInBytes / 4;
	
	U32 startInZcull = C_ThingyManager::AllocMemory(sizeInZcull);
	
	if(startInZcull != UINT_MAX)
	{
		BIZ_ASSERT(startInZcull < INVALID_ZCULL_MEMORY);

		return startInZcull;
	}
	else
	{
		#if !CFG_GOLD
		Dump();
		#endif

		BIZ_ASSERT_FAIL("Out of zcull memory");

		return INVALID_ZCULL_MEMORY;
	}
}

void C_ZcullManager::FreeMemory(const U32 sizeInBytes, const U32 startInZcull)
{
	U32 sizeInZcull = sizeInBytes / 4;
	
	C_ThingyManager::FreeMemory(sizeInZcull, startInZcull);
}

C_DeviceParams::C_DeviceParams()
{
	m_MainHeapSize					= 0;
	m_LocalHeapSize					= 0;
	m_CmdBufHeapSize				= 0;
	m_MainDeviceHeapSize			= 0;
	m_LocalDeviceHeapSize			= 0;
}

C_DeviceParams::~C_DeviceParams()
{
}

C_Device::C_Device()
{
	m_AllowOutOfMemory			= false;

	m_LastError					= GPU_ERROR_SUCCESS;

	m_Created						= 0;
	m_MainHeapSize					= 0;
	m_LocalHeapSize					= 0;
	m_CmdBufHeapSize				= 0;
	m_pCmdBufHeapAddress			= 0;
	m_MainDeviceHeapSize			= 0;
	m_pMainDeviceHeapAddress		= 0;
	m_LocalDeviceHeapSize			= 0;
	m_pLocalDeviceHeapAddress		= 0;

	m_Context.m_pState				= &m_ContextState;
	m_ContextNoCheck.m_pState		= &m_ContextState;
	
	InitBlendModes();
}

C_Device::~C_Device()
{
}

void C_Device::GetParams(C_DeviceParams& params)
{
	params.m_MainHeapSize			= m_MainHeapSize;
	params.m_LocalHeapSize			= m_LocalHeapSize;
	params.m_CmdBufHeapSize			= m_CmdBufHeapSize;
	params.m_MainDeviceHeapSize		= m_MainDeviceHeapSize;
	params.m_LocalDeviceHeapSize	= m_LocalDeviceHeapSize;
}

void C_Device::SetParams(const C_DeviceParams& params)
{
	m_MainHeapSize					= params.m_MainHeapSize;
	m_LocalHeapSize					= params.m_LocalHeapSize;
	m_CmdBufHeapSize				= params.m_CmdBufHeapSize;
	m_MainDeviceHeapSize			= params.m_MainDeviceHeapSize;
	m_LocalDeviceHeapSize			= params.m_LocalDeviceHeapSize;
}

bool C_Device::Create()
{
	BIZ_ASSERT(m_Created == 0);
	
	if(!CreateMemory())
		return false;

	if(!m_TileManager.Create(INVALID_TILE_INDEX, INVALID_TILE_MEMORY))
		return false;

	if(!m_ZcullManager.Create(INVALID_ZCULL_INDEX, INVALID_ZCULL_MEMORY))
		return false;

	m_Created = 1;

	return true;
}

bool C_Device::CreateVolatile()
{
	BIZ_ASSERT(m_Created == 1);

	m_Created = 2;

	return true;
}

void C_Device::DestroyVolatile()
{
	BIZ_ASSERT(m_Created == 2);

	m_Created = 1;
}

void C_Device::Destroy()
{
	BIZ_ASSERT(m_Created == 1);

	m_TileManager.Destroy();

	m_ZcullManager.Destroy();

	DestroyVertexDeclarations();

	DestroyMemory();

	m_Created = 0;
}

bool C_Device::CreateMemory()
{
	MEMORY_TRACKER_PUSH("graphics.display.device.heaps");

	if(m_MainHeapSize > 0)
	{
		if(!m_MainHeap.Create(m_MainHeapSize, 128, 0, E_MemoryBuffer_Main, "DisplayMain"))
		{
			MEMORY_TRACKER_POP("graphics.display.device.heaps");
			return false;
		}
	}

	if(m_LocalHeapSize > 0)
	{
		if(!m_LocalHeap.Create(m_LocalHeapSize, 128, 0, E_MemoryBuffer_Local, "DisplayLocal"))
		{
			MEMORY_TRACKER_POP("graphics.display.device.heaps");
			return false;
		}
	}

	if(m_CmdBufHeapSize > 0)
	{
		BIZ_ASSERT(m_MainHeapSize >= m_CmdBufHeapSize);

		if(!m_MainHeap.Alloc(128, m_CmdBufHeapSize, &m_pCmdBufHeapAddress, E_MemoryType_Indirect))
		{
			MEMORY_TRACKER_POP("graphics.display.device.heaps");
			LOGFATAL(g_Log, "Out of Memory! Failed to allocate command buffer heap of size %i\n", m_CmdBufHeapSize);
			return false;
		}

		if(!m_CmdBufHeap.Create(m_CmdBufHeapSize, 4, 0, E_MemoryBuffer_Indirect, "DisplayCmdBuf", m_pCmdBufHeapAddress))
		{
			MEMORY_TRACKER_POP("graphics.display.device.heaps");
			return false;
		}
	}

	if(m_MainDeviceHeapSize > 0)
	{
		BIZ_ASSERT(m_MainHeapSize >= m_MainDeviceHeapSize);

		if(!m_MainHeap.Alloc(128, m_MainDeviceHeapSize, &m_pMainDeviceHeapAddress, E_MemoryType_Indirect))
		{
			MEMORY_TRACKER_POP("graphics.display.device.heaps");
			LOGFATAL(g_Log, "Out of Memory! Failed to allocate main device heap of size %i\n", m_MainDeviceHeapSize);
			return false;
		}

		if(!m_MainDeviceHeap.Create(m_MainDeviceHeapSize, 128, 0, E_MemoryBuffer_Indirect, "DisplayMainDevice", m_pMainDeviceHeapAddress))
		{
			MEMORY_TRACKER_POP("graphics.display.device.heaps");
			return false;
		}

		#if TRACK_DISPLAY2_PROTECTED_MEMORY
		InsertProtectedMemory(m_MainDeviceHeapSize, m_pMainDeviceHeapAddress, "MainDeviceHeap");
		#endif
	}

	if(m_LocalDeviceHeapSize > 0)
	{
		BIZ_ASSERT(m_LocalHeapSize >= m_LocalDeviceHeapSize);

		if(!m_LocalHeap.Alloc(128, m_LocalDeviceHeapSize, &m_pLocalDeviceHeapAddress, E_MemoryType_Indirect))
		{
			MEMORY_TRACKER_POP("graphics.display.device.heaps");
			LOGFATAL(g_Log, "Out of Memory! Failed to allocate local device heap of size %i\n", m_LocalDeviceHeapSize);
			return false;
		}

		if(!m_LocalDeviceHeap.Create(m_LocalDeviceHeapSize, 128, 0, E_MemoryBuffer_Indirect, "DisplayLocalDevice", m_pLocalDeviceHeapAddress))
		{
			MEMORY_TRACKER_POP("graphics.display.device.heaps");
			return false;
		}

		#if TRACK_DISPLAY2_PROTECTED_MEMORY
		InsertProtectedMemory(m_LocalDeviceHeapSize, m_pLocalDeviceHeapAddress, "LocalDeviceHeap");
		#endif
	}

	MEMORY_TRACKER_POP("graphics.display.device.heaps");
	
	return true;
}

void C_Device::DestroyMemory()
{
	if(m_LocalDeviceHeapSize > 0 && m_pLocalDeviceHeapAddress)
	{
		#if TRACK_DISPLAY2_PROTECTED_MEMORY
		RemoveProtectedMemory(m_LocalDeviceHeapSize, m_pLocalDeviceHeapAddress);
		#endif
		
		m_LocalHeap.Free(m_LocalDeviceHeapSize, m_pLocalDeviceHeapAddress, E_MemoryType_Indirect);

		m_LocalDeviceHeapSize = 0;
		m_pLocalDeviceHeapAddress = NULL;
	}

	m_LocalDeviceHeap.Destroy();
	
	if(m_MainDeviceHeapSize > 0 && m_pMainDeviceHeapAddress)
	{
		#if TRACK_DISPLAY2_PROTECTED_MEMORY
		RemoveProtectedMemory(m_MainDeviceHeapSize, m_pMainDeviceHeapAddress);
		#endif

		m_MainHeap.Free(m_MainDeviceHeapSize, m_pMainDeviceHeapAddress, E_MemoryType_Indirect);

		m_MainDeviceHeapSize = 0;
		m_pMainDeviceHeapAddress = NULL;
	}

	m_MainDeviceHeap.Destroy();
	
	if(m_CmdBufHeapSize > 0 && m_pCmdBufHeapAddress)
	{
		m_MainHeap.Free(m_CmdBufHeapSize, m_pCmdBufHeapAddress, E_MemoryType_Indirect);

		m_CmdBufHeapSize = 0;
		m_pCmdBufHeapAddress = NULL;
	}

	m_CmdBufHeap.Destroy();

	m_LocalHeap.Destroy();

	m_MainHeap.Destroy();
}

C_VertexDeclaration* C_Device::FindVertexDeclaration(const S32 vertexElementCount, const C_VertexElement* pVertexElements)
{
	BIZ_ASSERT(vertexElementCount > 0);
	BIZ_ASSERT(vertexElementCount <= C_VertexDeclaration::m_MaxElementCount); // If you hit this increase m_MaxElementCount
	BIZ_ASSERT(pVertexElements);

	for(S32 i=0; i<m_VertexDeclarationList.Size(); i++)
	{
		C_VertexDeclaration* pOtherVertexDeclaration = m_VertexDeclarationList[i];

		if(vertexElementCount != pOtherVertexDeclaration->GetElementCount())
			continue;

		const C_VertexElement* pOtherVertexElements = pOtherVertexDeclaration->GetElements();

		bool equal = true;

		for(S32 j=0; j<vertexElementCount; j++)
		{
			const C_VertexElement& vertexElement = pVertexElements[j];
			const C_VertexElement& otherVertexElement = pOtherVertexElements[j];

			if(vertexElement != otherVertexElement)
			{
				equal = false;
				break;
			}
		}

		if(equal)
		{
			return pOtherVertexDeclaration;
		}
	}

	return NULL;
}

C_VertexDeclaration* C_Device::CreateVertexDeclaration(const C_VertexFormat& vertexFormat)
{
	return CreateVertexDeclaration(vertexFormat.m_Elements);
}

bool C_Device::CreateVertexDeclaration(const C_VertexFormat& vertexFormat, C_VertexDeclaration* pVertexDeclaration)
{
	return CreateVertexDeclaration(vertexFormat.m_Elements, pVertexDeclaration);
}

C_VertexDeclaration* C_Device::CreateVertexDeclaration(const C_VertexElement* pVertexElements)
{
	m_Mutex.Wait();

	U32 vertexElementCount = 0;

	while(pVertexElements[vertexElementCount].GetStream() != E_Stream_Count)
	{
		vertexElementCount++;

		BIZ_ASSERT(vertexElementCount <= C_VertexDeclaration::m_MaxElementCount);
	}

	C_VertexDeclaration* pVertexDeclaration = FindVertexDeclaration(vertexElementCount, pVertexElements);

	if(pVertexDeclaration)
	{
		m_Mutex.Release();

		return pVertexDeclaration;
	}
	else
	{
		pVertexDeclaration = new C_VertexDeclaration();

		if(CreateVertexDeclaration(pVertexElements, pVertexDeclaration))
		{
			m_VertexDeclarationList.PushBack(pVertexDeclaration);

			m_Mutex.Release();

			return pVertexDeclaration;
		}
		else
		{
			SAFE_DELETE_PTR(pVertexDeclaration);

			m_Mutex.Release();

			return NULL;
		}
	}
}

bool C_Device::CreateVertexDeclaration(const C_VertexElement* pVertexElements, C_VertexDeclaration* pVertexDeclaration)
{
	BIZ_ASSERT(pVertexDeclaration);
	BIZ_ASSERT(!pVertexDeclaration->IsCreated());

	// Compute vertex element count

	U32 vertexElementCount = 0;

	while(pVertexElements[vertexElementCount].GetStream() != E_Stream_Count)
	{
		vertexElementCount++;

		BIZ_ASSERT(vertexElementCount <= C_VertexDeclaration::m_MaxElementCount);
	}

	// Copy vertex elements to vertex declaration

	pVertexDeclaration->m_ElementCount = vertexElementCount;
	
	for(S32 i=0; i<vertexElementCount; i++)
	{
		pVertexDeclaration->m_ElementArray[i] = pVertexElements[i];

		pVertexDeclaration->m_ElementArray[i].m_gcmStride = 0; // Make sure this is reset as it gets computed below
	}

	// Compute vertex element strides

	for(S32 i=0; i<pVertexDeclaration->m_ElementCount; i++)
	{
		C_VertexElement& vertexElement = pVertexDeclaration->m_ElementArray[i];

		for(U32 j=0; j<pVertexDeclaration->m_ElementCount; j++)
		{
			C_VertexElement& otherVertexElement = pVertexDeclaration->m_ElementArray[j];

			if(vertexElement.m_Stream == otherVertexElement.m_Stream)
			{
				vertexElement.m_gcmStride += otherVertexElement.GetSize();
			}
		}
	}
	
	return true;
}

void C_Device::DestroyVertexDeclaration(C_VertexDeclaration* pVertexDeclaration)
{
	BIZ_ASSERT(pVertexDeclaration);
	if(pVertexDeclaration->IsCreated())
	{
		pVertexDeclaration->m_ElementCount = 0;
	}
}

void C_Device::DestroyVertexDeclarations()
{
	for(S32 i=0; i<m_VertexDeclarationList.Size(); i++)
	{
		DestroyVertexDeclaration(m_VertexDeclarationList[i]);
		SAFE_DELETE_PTR(m_VertexDeclarationList[i]);
	}
	m_VertexDeclarationList.Clear();
}

bool C_Device::CreateVertexBuffer(U32 size, U32 flags, C_VertexBuffer* pVertexBuffer, void* pAddress)
{
	BIZ_ASSERT(pVertexBuffer);
	BIZ_ASSERT(!pVertexBuffer->IsCreated());
	BIZ_ASSERT(size > 0);

	if(flags & E_Flags_System) // Ensure either video or system flag set
		flags &= ~E_Flags_Video;
	else
		flags |= E_Flags_Video;

	if(!pAddress)
	{
		if(!AllocResourceMemory(E_ResourceType_VertexBuffer, flags, size, (U8**)&pAddress))
		{
			if(!m_AllowOutOfMemory)
			{
				m_LastError = GPU_ERROR_OUT_OF_MEMORY;
				DumpMemoryHeaps();
				LOGFATAL(g_Log, "Out of Memory! Failed to create vertex buffer of size %i flags %i\n", size, flags);
			}
			return false;
		}

		flags &= ~E_Flags_Indirect;
	}
	else
	{
		flags |= E_Flags_Indirect;
	}

	BIZ_ASSERT_MSG(IsMainAddress(pAddress) ? flags & E_Flags_System : flags & E_Flags_Video, "Flags 0x%.8x do not match address 0x%.8x", flags, pAddress);

	U32 offset = 0;
	if(cellGcmAddressToOffset(pAddress, &offset) != CELL_OK)
	{
		if(!(flags & E_Flags_Indirect))
			FreeMemory(size, (U8*)pAddress, E_MemoryType_VertexBuffer, flags);

		LOGFATAL(g_Log, "Failed to convert vertex buffer address %i to offset\n", pAddress);
		return false;
	}

	pVertexBuffer->SetSize(size);
	pVertexBuffer->SetFlags(flags);
	pVertexBuffer->SetOffset(offset);
	
	#if TRACK_DISPLAY2_RESOURCES
	if(!(flags & E_Flags_DoNotTrack))
		InsertVertexBuffer(pVertexBuffer);
	#endif
	
	return true;
}

void C_Device::DestroyVertexBuffer(C_VertexBuffer* pVertexBuffer)
{
	BIZ_ASSERT(pVertexBuffer);
	if(pVertexBuffer->IsCreated())
	{
		#if TRACK_DISPLAY2_RESOURCES
		if(!(pVertexBuffer->GetFlags() & E_Flags_DoNotTrack))
			RemoveVertexBuffer(pVertexBuffer);
		#endif
		
		if(!(pVertexBuffer->m_Flags & E_Flags_Indirect))
			FreeMemory(pVertexBuffer->GetSize(), (U8*)pVertexBuffer->GetAddress(), E_MemoryType_VertexBuffer, pVertexBuffer->GetFlags());

		pVertexBuffer->SetSize(0);
		pVertexBuffer->SetFlags(0);
		pVertexBuffer->SetOffset(0);
	}
}

bool C_Device::CreateIndexBuffer(U32 size, U32 flags, E_IndexFormat format, C_IndexBuffer* pIndexBuffer, void* pAddress)
{
	BIZ_ASSERT(pIndexBuffer);
	BIZ_ASSERT(!pIndexBuffer->IsCreated());
	BIZ_ASSERT(size > 0);

	if(flags & E_Flags_System) // Ensure either video or system flag set
		flags &= ~E_Flags_Video;
	else
		flags |= E_Flags_Video;

	if(!pAddress)
	{
		if(!AllocResourceMemory(E_ResourceType_IndexBuffer, flags, size, (U8**)&pAddress))
		{
			if(!m_AllowOutOfMemory)
			{
				m_LastError = GPU_ERROR_OUT_OF_MEMORY;
				DumpMemoryHeaps();
				LOGFATAL(g_Log, "Out of Memory! Failed to create index buffer of size %i flags %i format %i\n", size, flags, format);
			}
			return false;
		}

		flags &= ~E_Flags_Indirect;
	}
	else
	{
		flags |= E_Flags_Indirect;
	}

	BIZ_ASSERT_MSG(IsMainAddress(pAddress) ? flags & E_Flags_System : flags & E_Flags_Video, "Flags 0x%.8x do not match address 0x%.8x", flags, pAddress);

	U32 offset = 0;
	if(cellGcmAddressToOffset(pAddress, &offset) != CELL_OK)
	{
		if(!(flags & E_Flags_Indirect))
			FreeMemory(size, (U8*)pAddress, E_MemoryType_IndexBuffer, flags);

		LOGFATAL(g_Log, "Failed to convert the index buffer address %i to offset\n", pAddress);
		return false;
	}
	
	pIndexBuffer->SetSize(size);
	pIndexBuffer->SetFlags(flags);
	pIndexBuffer->SetOffset(offset);
	pIndexBuffer->SetFormat(format);
	
	#if TRACK_DISPLAY2_RESOURCES
	if(!(flags & E_Flags_DoNotTrack))
		InsertIndexBuffer(pIndexBuffer);
	#endif
	
	return true;
}

void C_Device::DestroyIndexBuffer(C_IndexBuffer* pIndexBuffer)
{
	BIZ_ASSERT(pIndexBuffer);
	if(pIndexBuffer->IsCreated())
	{
		#if TRACK_DISPLAY2_RESOURCES
		if(!(pIndexBuffer->GetFlags() & E_Flags_DoNotTrack))
			RemoveIndexBuffer(pIndexBuffer);
		#endif
		
		if(!(pIndexBuffer->m_Flags & E_Flags_Indirect))
			FreeMemory(pIndexBuffer->GetSize(), (U8*)pIndexBuffer->GetAddress(), E_MemoryType_IndexBuffer, pIndexBuffer->GetFlags());

		pIndexBuffer->SetSize(0);
		pIndexBuffer->SetFlags(0);
		pIndexBuffer->SetOffset(0);
		pIndexBuffer->SetFormat(E_IndexFormat_Unknown);
	}
}

bool C_Device::CreateVertexShader(U32 size, const U8* pData, U32 flags, C_VertexShader* pVertexShader, void* pAddress)
{
	BIZ_ASSERT(pVertexShader);
	BIZ_ASSERT(!pVertexShader->IsCreated());
	BIZ_ASSERT(size > 0);

	int32_t res = cellCgbRead(pData, size, &pVertexShader->m_Program);
	if(res != CELL_CGB_OK)
	{
		LOGFATAL(g_Log, "Failed to read the cgb vertex program binary of size %i\n", size);
		return false;
	}

	res = cellCgbGetVertexConfiguration(&pVertexShader->m_Program, &pVertexShader->m_ProgramConfig);
	if(res != CELL_CGB_OK)
	{
		LOGFATAL(g_Log, "Failed to get the cgb vertex program configuration\n");
		return false;
	}

	pVertexShader->SetSize(size);
	pVertexShader->SetFlags(flags);

	// Get the command chain
	C_CommandChain* pCommandChain = pVertexShader->GetCommandChain();

	// Get the size of the command chain
	pCommandChain->m_Size = GetVertexShaderCommandBufferSize(&pVertexShader->m_Program, &pVertexShader->m_ProgramConfig);
	
	#if 0 && defined(CELL_GCM_DEBUG)
	pCommandChain->m_Size += sizeof(U32) * 4; // HACK: Why is this needed for release?
	#endif

	// Allocate the main memory for the command 
	if(!m_CmdBufHeap.Alloc(4, pCommandChain->m_Size, (U8**)&pCommandChain->m_pAddress, E_MemoryType_CommandBuffer))
	{
		DumpMemoryHeaps();
		LOGFATAL(g_Log, "Out of Memory! Failed to allocate vertex shader command buffer of size %i\n", pCommandChain->m_Size);
		return false;
	}

	res = cellGcmAddressToOffset(pCommandChain->m_pAddress, &pCommandChain->m_Offset);
	BIZ_ASSERT(res == CELL_OK);

	// Build the command chain
	if(!BuildVertexShaderCommandBuffer(pCommandChain, &pVertexShader->m_Program, &pVertexShader->m_ProgramConfig))
	{
		m_CmdBufHeap.Free(pCommandChain->m_Size, (U8*)pCommandChain->m_pAddress, E_MemoryType_CommandBuffer);
		LOGFATAL(g_Log, "Failed to build vertex shader command buffer of size %i\n", pCommandChain->m_Size);
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	if(!(flags & E_Flags_DoNotTrack))
		InsertVertexShader(pVertexShader);
	#endif

	return true;
}

void C_Device::DestroyVertexShader(C_VertexShader* pVertexShader)
{
	BIZ_ASSERT(pVertexShader);
	if(pVertexShader->IsCreated())
	{
		#if TRACK_DISPLAY2_RESOURCES
		if(!(pVertexShader->GetFlags() & E_Flags_DoNotTrack))
			RemoveVertexShader(pVertexShader);
		#endif

		pVertexShader->SetSize(0);
		pVertexShader->SetFlags(0);

		SAFE_DELETE_ARY(pVertexShader->m_pAddressInMainMemory);

		m_CmdBufHeap.Free(pVertexShader->m_CommandChain.m_Size, (U8*)pVertexShader->m_CommandChain.m_pAddress, E_MemoryType_CommandBuffer);
	}
}

bool C_Device::CreatePixelShader(U32 size, const U8* pData, U32 flags, C_PixelShader* pPixelShader, void* pAddress)
{
	BIZ_ASSERT(pPixelShader);
	#if !TRACK_DISPLAY2_RESOURCES
	BIZ_ASSERT(!pPixelShader->IsCreated());
	#endif
	BIZ_ASSERT(size > 0);

	if(flags & E_Flags_System) // Ensure either video or system flag set
		flags &= ~E_Flags_Video;
	else
		flags |= E_Flags_Video;

	CellCgbProgram program;
	int32_t res = cellCgbRead(pData, size, &program);
	if(res != CELL_CGB_OK)
	{
		LOGFATAL(g_Log, "Failed to read the cgb pixel program binary of size %i\n", size);
		return false;
	}

	CellCgbFragmentProgramConfiguration programConfig;
	res = cellCgbGetFragmentConfiguration(&program, &programConfig);
	if(res != CELL_CGB_OK)
	{
		LOGFATAL(g_Log, "Failed to get the cgb pixel program configuration\n");
		return false;
	}

	const U32 ucodeSize = ((cellCgbGetUCodeSize(&program)+127)&~127);	// align size up to 128 bytes;
	const void* ucode = cellCgbGetUCode(&program);

	if(!AllocResourceMemory(E_ResourceType_PixelShader, flags, ucodeSize, (U8**)&pAddress))
	{
		DumpMemoryHeaps();
		LOGFATAL(g_Log, "Out of Memory! Failed to create pixel shader of size %i\n", ucodeSize);
		return false;
	}

	BIZ_ASSERT_MSG(IsMainAddress(pAddress) ? flags & E_Flags_System : flags & E_Flags_Video, "Flags 0x%.8x do not match address 0x%.8x", flags, pAddress);

	U32 offset = 0;
	if(cellGcmAddressToOffset(pAddress, &offset) != CELL_OK)
	{
		if(!(flags & E_Flags_Indirect))
			FreeMemory(size, (U8*)pAddress, E_MemoryType_PixelShader, flags);

		LOGFATAL(g_Log, "Failed to convert the pixel shader address %i to offset\n", pAddress);
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	if(pPixelShader->IsCreated() && !(pPixelShader->m_Flags & E_Flags_Indirect))
		FreeMemory(pPixelShader->GetSize(), (U8*)pPixelShader->GetAddress(), E_MemoryType_PixelShader, pPixelShader->GetFlags());
	if(!pPixelShader->IsCreated() && !(flags & E_Flags_DoNotTrack))
		InsertPixelShader(pPixelShader);
	#endif

	memcpy(pAddress, ucode, ucodeSize);
	
	pPixelShader->m_pAddress = (U8*)pAddress;
	pPixelShader->m_Program = program;
	pPixelShader->m_ProgramConfig = programConfig;
	
	pPixelShader->SetSize(ucodeSize);
	pPixelShader->SetFlags(flags);
	pPixelShader->SetOffset(offset);

	pPixelShader->CreateParams();

	return true;
}

void C_Device::DestroyPixelShader(C_PixelShader* pPixelShader)
{
	BIZ_ASSERT(pPixelShader);
	if(pPixelShader->IsCreated())
	{
		#if TRACK_DISPLAY2_RESOURCES
		if(!(pPixelShader->GetFlags() & E_Flags_DoNotTrack))
			RemovePixelShader(pPixelShader);
		#endif

		if(!(pPixelShader->m_Flags & E_Flags_Indirect))
			FreeMemory(pPixelShader->GetSize(), (U8*)pPixelShader->GetAddress(), E_MemoryType_PixelShader, pPixelShader->GetFlags());

		pPixelShader->SetSize(0);
		pPixelShader->SetFlags(0);
		pPixelShader->SetOffset(0);

		SAFE_DELETE_ARY(pPixelShader->m_pAddressInMainMemory);
	}
}

bool C_Device::CreateSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pAddress, const U32 addressSize)
{
	BIZ_ASSERT(pSurface);
	BIZ_ASSERT(!pSurface->IsCreated());
	BIZ_ASSERT(width > 0 && height > 0);

	if(flags & E_Flags_System) // Ensure either video or system flag set
		flags &= ~E_Flags_Video;
	else
		flags |= E_Flags_Video;

	U32 pitch = 0, size = 0;
	GetSurfaceSizeAndPitch(width, height, flags, format, multisample, size, pitch);

	if(!pAddress)
	{
		if(!AllocResourceMemory(E_ResourceType_Surface, flags, size, (U8**)&pAddress))
		{
			DumpMemoryHeaps();
			LOGFATAL(g_Log, "Out of Memory! Failed to create surface of size %i width %i height %i flags %i format %i multisample %i\n", size, width, height, flags, format, multisample);
			return false;
		}

		flags &= ~E_Flags_Indirect;
	}
	else
	{
		BIZ_ASSERT(size <= addressSize);

		flags |= E_Flags_Indirect;
	}

	if(m_MainHeap.Inside(size, (const U8*)pAddress))
	{
		flags &= ~E_Flags_Video;
		flags |= E_Flags_System;
	}
	else
	if(m_LocalHeap.Inside(size, (const U8*)pAddress))
	{
		flags &= ~E_Flags_System;
		flags |= E_Flags_Video;
	}
	else
	{
		BIZ_ASSERT_FAIL("Memory of size %i and address 0x%8x is not from the local or main heap", size, pAddress);
	}

	U32 offset = 0;
	if(cellGcmAddressToOffset(pAddress, &offset) != CELL_OK)
	{
		if(!(flags & E_Flags_Indirect))
			FreeMemory(size, (U8*)pAddress, E_MemoryType_Surface, flags);
		
		LOGFATAL(g_Log, "Failed to convert the surface address %i to offset\n", pAddress);
		return false;
	}

	pSurface->SetSize(size);
	pSurface->SetFlags(flags);
	pSurface->SetOffset(offset);
	pSurface->SetAddress((U8*)pAddress);
	
	pSurface->m_Pitch			= pitch;
	pSurface->m_Width			= width;
	pSurface->m_Height			= height;
	pSurface->m_Format			= format;
	pSurface->m_Multisample		= multisample;

	#if TRACK_DISPLAY2_RESOURCES
	if(!(flags & E_Flags_DoNotTrack))
		InsertSurface(pSurface);
	#endif
	
	return true;
}

void C_Device::DestroySurface(C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	if(pSurface->IsCreated())
	{
		#if TRACK_DISPLAY2_RESOURCES
		if(!(pSurface->GetFlags() & E_Flags_DoNotTrack))
			RemoveSurface(pSurface);
		#endif

		if(!(pSurface->m_Flags & E_Flags_Indirect))
			FreeMemory(pSurface->GetSize(), (U8*)pSurface->GetAddress(), E_MemoryType_Surface, pSurface->GetFlags());

		pSurface->SetSize(0);
		pSurface->SetFlags(0);
		pSurface->SetOffset(0);
		pSurface->SetAddress(NULL);
	}
}

bool C_Device::CreateTexture2D(U32 width, U32 height, U32 levels, U32 flags, E_TextureFormat format, C_Texture* pTexture, void* pAddress, U32 addressSize)
{
	BIZ_ASSERT(pTexture);
	BIZ_ASSERT(!pTexture->IsCreated());
	BIZ_ASSERT(width > 0 && height > 0 && levels > 0);

	if(flags & E_Flags_System) // Ensure either video or system flag set
		flags &= ~E_Flags_Video;
	else
		flags |= E_Flags_Video;
	
	const U32 pitch = GetTexturePitchFromWidth(width, flags, format);

	const U32 size = SetTexture2DHeader(width, height, levels, flags, format, pitch, &pTexture->m_gcmTexture);

	if(!pAddress)
	{
		if(!AllocResourceMemory(E_ResourceType_Texture, flags, size, (U8**)&pAddress))
		{
			DumpMemoryHeaps();
			LOGFATAL(g_Log, "Out of Memory! Failed to create 2d texture of size %i width %i height %i levels %i flags %i format %i\n", size, width, height, levels, flags, format);
			return false;
		}

		flags &= ~E_Flags_Indirect;
	}
	else
	{
		BIZ_ASSERT(size <= addressSize); // I guess its ok to use less?

		flags |= E_Flags_Indirect;
	}

	if(m_MainHeap.Inside(size, (const U8*)pAddress))
	{
		flags &= ~E_Flags_Video;
		flags |= E_Flags_System;

		pTexture->m_gcmTexture.location = CELL_GCM_LOCATION_MAIN;
	}
	else
	if(m_LocalHeap.Inside(size, (const U8*)pAddress))
	{
		flags &= ~E_Flags_System;
		flags |= E_Flags_Video;

		pTexture->m_gcmTexture.location = CELL_GCM_LOCATION_LOCAL;
	}
	else
	{
		BIZ_ASSERT_FAIL("Memory of size %i and address 0x%8x is not from the local or main heap", size, pAddress);
	}

	U32 offset = 0;
	if(cellGcmAddressToOffset(pAddress, &offset) != CELL_OK)
	{
		if(!(flags & E_Flags_Indirect))
			FreeMemory(size, (U8*)pAddress, E_MemoryType_Texture, flags);

		LOGFATAL(g_Log, "Failed to convert the 2d texture address %i to offset\n", pAddress);
		return false;
	}

	pTexture->SetSize(size);
	pTexture->SetFlags(flags);
	pTexture->SetOffset(offset);

	#if TRACK_DISPLAY2_RESOURCES
	if(!(flags & E_Flags_DoNotTrack))
		InsertTexture(pTexture);
	#endif

	return true;
}

bool C_Device::CreateTexture3D(const U32 width, const U32 height, const U32 depth, const U32 levels, U32 flags, const E_TextureFormat format, C_Texture* pTexture, void* pAddress, U32 addressSize)
{
	BIZ_ASSERT(pTexture);
	BIZ_ASSERT(!pTexture->IsCreated());
	BIZ_ASSERT(width > 0 && height > 0 && depth > 0 && levels > 0);

	if(flags & E_Flags_System) // Ensure either video or system flag set
		flags &= ~E_Flags_Video;
	else
		flags |= E_Flags_Video;
	
	const U32 pitch = GetTexturePitchFromWidth(width, flags, format);

	const U32 size = SetTexture3DHeader(width, height, depth, levels, flags, format, pitch, &pTexture->m_gcmTexture);

	if(!pAddress)
	{
		if(!AllocResourceMemory(E_ResourceType_Texture, flags, size, (U8**)&pAddress))
		{
			DumpMemoryHeaps();
			LOGFATAL(g_Log, "Out of Memory! Failed to create 3d texture of size %i width %i height %i depth %i levels %i flags %i format %i\n", size, width, height, depth, levels, flags, format);
			return false;
		}

		flags &= ~E_Flags_Indirect;
	}
	else
	{
		BIZ_ASSERT(size <= addressSize);

		flags |= E_Flags_Indirect;
	}

	if(m_MainHeap.Inside(size, (const U8*)pAddress))
	{
		flags &= ~E_Flags_Video;
		flags |= E_Flags_System;

		pTexture->m_gcmTexture.location = CELL_GCM_LOCATION_MAIN;
	}
	else
	if(m_LocalHeap.Inside(size, (const U8*)pAddress))
	{
		flags &= ~E_Flags_System;
		flags |= E_Flags_Video;

		pTexture->m_gcmTexture.location = CELL_GCM_LOCATION_LOCAL;
	}
	else
	{
		BIZ_ASSERT_FAIL("Memory of size %i and address 0x%8x is not from the local or main heap", size, pAddress);
	}

	U32 offset = 0;
	if(cellGcmAddressToOffset(pAddress, &offset) != CELL_OK)
	{
		if(!(flags & E_Flags_Indirect))
			FreeMemory(size, (U8*)pAddress, E_MemoryType_Texture, flags);

		LOGFATAL(g_Log, "Failed to convert the 3d texture address %i to offset\n", pAddress);
		return false;
	}

	pTexture->SetSize(size);
	pTexture->SetFlags(flags);
	pTexture->SetOffset(offset);

	#if TRACK_DISPLAY2_RESOURCES
	if(!(flags & E_Flags_DoNotTrack))
		InsertTexture(pTexture);
	#endif

	return true;
}

bool C_Device::CreateTextureCube(const U32 width, const U32 levels, U32 flags, const E_TextureFormat format, C_Texture* pTexture, void* pAddress, U32 addressSize)
{
	BIZ_ASSERT(pTexture);
	BIZ_ASSERT(!pTexture->IsCreated());
	BIZ_ASSERT(width > 0 && levels > 0);

	if(flags & E_Flags_System) // Ensure either video or system flag set
		flags &= ~E_Flags_Video;
	else
		flags |= E_Flags_Video;
	
	const U32 pitch = GetTexturePitchFromWidth(width, flags, format);

	const U32 size = SetTextureCubeHeader(width, levels, flags, format, pitch, &pTexture->m_gcmTexture);

	if(!pAddress)
	{
		if(!AllocResourceMemory(E_ResourceType_Texture, flags, size, (U8**)&pAddress))
		{
			DumpMemoryHeaps();
			LOGFATAL(g_Log, "Out of Memory! Failed to create cube texture of size %i width %i %i levels %i flags %i format %i\n", size, width, levels, flags, format);
			return false;
		}

		flags &= ~E_Flags_Indirect;
	}
	else
	{
		BIZ_ASSERT(size <= addressSize);

		flags |= E_Flags_Indirect;
	}

	if(m_MainHeap.Inside(size, (const U8*)pAddress))
	{
		flags &= ~E_Flags_Video;
		flags |= E_Flags_System;

		pTexture->m_gcmTexture.location = CELL_GCM_LOCATION_MAIN;
	}
	else
	if(m_LocalHeap.Inside(size, (const U8*)pAddress))
	{
		flags &= ~E_Flags_System;
		flags |= E_Flags_Video;

		pTexture->m_gcmTexture.location = CELL_GCM_LOCATION_LOCAL;
	}
	else
	{
		BIZ_ASSERT_FAIL("Memory of size %i and address 0x%8x is not from the local or main heap", size, pAddress);
	}

	U32 offset = 0;
	if(cellGcmAddressToOffset(pAddress, &offset) != CELL_OK)
	{
		if(!(flags & E_Flags_Indirect))
			FreeMemory(size, (U8*)pAddress, E_MemoryType_Texture, flags);

		LOGFATAL(g_Log, "Failed to convert the cube texture address %i to offset\n", pAddress);
		return false;
	}

	pTexture->SetSize(size);
	pTexture->SetFlags(flags);
	pTexture->SetOffset(offset);

	#if TRACK_DISPLAY2_RESOURCES
	if(!(flags & E_Flags_DoNotTrack))
		InsertTexture(pTexture);
	#endif

	return true;
}

void C_Device::DestroyTexture(C_Texture* pTexture)
{
	BIZ_ASSERT(pTexture);
	if(pTexture->IsCreated())
	{
		#if TRACK_DISPLAY2_RESOURCES
		if(!(pTexture->GetFlags() & E_Flags_DoNotTrack))
			RemoveTexture(pTexture);
		#endif

		if(!(pTexture->m_Flags & E_Flags_Indirect))
			FreeMemory(pTexture->GetSize(), (U8*)pTexture->GetAddress(), E_MemoryType_Texture, pTexture->GetFlags());

		pTexture->SetSize(0);
		pTexture->SetFlags(0);
		pTexture->SetOffset(0);
	}
}

bool C_Device::CreateRenderTexture2D(const U32 width, const U32 height, const U32 levels, U32 flags, const E_TextureFormat textureFormat, const E_SurfaceFormat surfaceFormat, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTexture2D* pRenderTexture, U8* pAddress, const U32 addressSize)
{
	BIZ_ASSERT(pRenderTexture);
	BIZ_ASSERT(!pRenderTexture->IsCreated());

	if(!(flags & E_Flags_Colour) || !(flags & E_Flags_DepthStencil))
		flags |= E_Flags_Colour;

	if(!CreateRenderTexture2DIndex(0, width, height, levels, flags, textureFormat, surfaceFormat, multisample, pParams, pRenderTexture, pAddress, addressSize))
		return false;

	if(flags & E_Flags_ReadWrite)
	{
		const U32 textureSize = GetTexture2DSize(width, height, levels, flags, textureFormat);
		U32 remainingSize = addressSize - textureSize;

		if(!CreateRenderTexture2DIndex(1, width, height, levels, flags, textureFormat, surfaceFormat, multisample, pParams, pRenderTexture, pAddress, remainingSize))
			return false;

		pRenderTexture->m_TextureIndex = 1;
	}
	else
	{
		pRenderTexture->m_TextureIndex = 0;
	}
	
	pRenderTexture->m_SurfaceIndex = 0;

	#if TRACK_DISPLAY2_RESOURCES
	if(!(flags & E_Flags_DoNotTrack))
		InsertRenderTexture2D(pRenderTexture);
	#endif

	pRenderTexture->m_Flags = flags;
	
	return true;
}

void C_Device::DestroyRenderTexture2D(C_RenderTexture2D* pRenderTexture)
{
	BIZ_ASSERT(pRenderTexture);
	
	DestroyRenderTexture2DIndex(0, pRenderTexture);

	if(pRenderTexture->m_Flags & E_Flags_ReadWrite)
	{
		DestroyRenderTexture2DIndex(1, pRenderTexture);
	}
	
	pRenderTexture->m_TextureIndex = 0;
	pRenderTexture->m_SurfaceIndex = 0;

	#if TRACK_DISPLAY2_RESOURCES
	if(!(pRenderTexture->m_Flags & E_Flags_DoNotTrack))
		RemoveRenderTexture2D(pRenderTexture);
	#endif

	pRenderTexture->m_Flags = 0;
}

bool C_Device::CreateRenderTexture2DIndex(const U32 index, const U32 width, const U32 height, const U32 levels, U32 flags, const E_TextureFormat textureFormat, const E_SurfaceFormat surfaceFormat, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTexture2D* pRenderTexture, U8* pAddress, const U32 addressSize)
{
	BIZ_ASSERT(index < 2);
	BIZ_ASSERT(width > 0);
	BIZ_ASSERT(height > 0);
	BIZ_ASSERT(levels > 0);
	BIZ_ASSERT(pRenderTexture);

	C_Texture& texture = pRenderTexture->m_Texture[index];

	BIZ_ASSERT(!texture.IsCreated());

	const U32 textureWidth = GetTextureWidthFromMultisample(width, multisample);
	const U32 textureHeight = GetTextureHeightFromMultisample(height, multisample);

	if(index == 1 && pAddress)
	{
		const U32 textureSize = GetTexture2DSize(textureWidth, textureHeight, levels, flags, textureFormat);
		BIZ_ASSERT(textureSize == pRenderTexture->m_Texture[0].GetSize());
		pAddress += textureSize;
	}
	
	if(!CreateTexture2D(textureWidth, textureHeight, levels, flags, textureFormat, &texture, pAddress, addressSize))
		return false;

	U32 textureLevelWidth		= textureWidth;
	U32 textureLevelHeight		= textureHeight;

	U32 surfaceLevelWidth		= width;
	U32 surfaceLevelHeight		= height;
	const U32 surfaceFlags		= flags | E_Flags_Indirect;
	const U32 surfaceSize		= GetSurfaceSize(width, height, flags, surfaceFormat, multisample);
	U8* pSurfaceLevelAddress	= texture.GetAddress();

	BIZ_ASSERT(pRenderTexture->m_Surfaces[index].Size() == 0);
	pRenderTexture->m_Surfaces[index].Resize(levels);

	for(U32 level=0; level<levels; level++)
	{
		C_Surface& surface = pRenderTexture->m_Surfaces[index][level];

		const U32 textureLevelSize = GetTexture2DSize(textureLevelWidth, textureLevelHeight, flags, textureFormat);
		const U32 surfaceLevelSize = GetSurfaceSize(surfaceLevelWidth, surfaceLevelHeight, flags, surfaceFormat, multisample);
		BIZ_ASSERT(textureLevelSize == surfaceLevelSize);

		if(!CreateColourSurface(surfaceLevelWidth, surfaceLevelHeight, surfaceFlags, surfaceFormat, multisample, NULL, &surface, pSurfaceLevelAddress, surfaceLevelSize))
			return false;

		pSurfaceLevelAddress += surfaceLevelSize;

		if(textureLevelWidth > 1)
			textureLevelWidth /= 2;
		if(textureLevelHeight > 1)
			textureLevelHeight /= 2;

		if(surfaceLevelWidth > 1)
			surfaceLevelWidth /= 2;
		if(surfaceLevelHeight > 1)
			surfaceLevelHeight /= 2;
	}

	BIZ_ASSERT(pSurfaceLevelAddress == texture.GetAddress()+texture.GetSize());
	
	return true;
}

void C_Device::DestroyRenderTexture2DIndex(const U32 index, C_RenderTexture2D* pRenderTexture)
{
	BIZ_ASSERT(index < 2);
	BIZ_ASSERT(pRenderTexture);

	for(S32 level=0; level<pRenderTexture->m_Surfaces[index].Size(); level++)
	{
		DestroySurface(&pRenderTexture->m_Surfaces[index][level]);
	}

	pRenderTexture->m_Surfaces[index].Clear();

	DestroyTexture(&pRenderTexture->m_Texture[index]);
}

bool C_Device::CreateRenderTextureCube(const U32 width, const U32 levels, U32 flags, const E_TextureFormat textureFormat, const E_SurfaceFormat surfaceFormat, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTextureCube* pRenderTexture, U8* pAddress, const U32 addressSize)
{
	return true;
}

void C_Device::DestroyRenderTextureCube(C_RenderTextureCube* pRenderTexture)
{
}

bool C_Device::CreateSharedTiledArea(C_SharedTiledArea& tiledArea, const U32 width, const U32 height, U32 flags, const E_SurfaceFormat surfaceFormat, const E_Multisample multisample, const U32 buffers, const char* pName)
{
	flags |= Display2::E_Flags_Tiled;
	U32 pitch = GetSurfacePitchFromWidth(width, flags, surfaceFormat, multisample);
	tiledArea.size = GetSurfaceSize(width, height, flags, surfaceFormat, multisample);
	tiledArea.size = ALIGN_SIZE(tiledArea.size, pitch * 8);

	tiledArea.totalSize = tiledArea.size * buffers;
	tiledArea.totalSize = ALIGN_SIZE(tiledArea.totalSize, 0x10000);
	if(!AllocMemory(tiledArea.totalSize, 0x10000, flags, &tiledArea.pAddress, Display2::E_MemoryType_Texture))
		return false;
	U32 offset;
	cellGcmAddressToOffset(tiledArea.pAddress, &offset);

	if(flags & E_Flags_DepthStencil)
		CreateDepthTile(tiledArea.tileInfo, tiledArea.totalSize, Display2::FlagsToLocation(flags), offset, pitch, multisample, pName);
	else
		CreateColourTile(tiledArea.tileInfo, tiledArea.totalSize, Display2::FlagsToLocation(flags), offset, pitch, multisample, pName);

	return true;
}

void C_Device::DestroySharedTiledArea(C_SharedTiledArea& tiledArea)
{
	if(tiledArea.pAddress)
	{
		FreeMemory(tiledArea.totalSize, tiledArea.pAddress, Display2::E_MemoryType_Texture);
		tiledArea.pAddress = NULL;
		tiledArea.totalSize = 0;
		DestroyTile(tiledArea.tileInfo);
	}
}

bool C_Device::CreateColourTile(C_TileInfo& tileInfo, U32 size, U8 location, U32 offset, U32 pitch, E_Multisample multisample, const char* pName)
{
	if(!AllocTileInfo(&tileInfo, size))
		return false;

	tileInfo.location		= location;
	tileInfo.offset			= offset;
	tileInfo.size			= size;
	tileInfo.pitch			= pitch;

	switch(multisample)
	{
	case E_Multisample_2:
		tileInfo.comp		= CELL_GCM_COMPMODE_C32_2X1;
		break;
	case E_Multisample_4:
		tileInfo.comp		= CELL_GCM_COMPMODE_C32_2X2;
		break;
	default:
		tileInfo.comp		= CELL_GCM_COMPMODE_DISABLED;
		break;
	}

	if(!SetTileInfo(&tileInfo))
		return false;
	if(!BindTileInfo(&tileInfo))
		return false;

#if !CFG_GOLD
	m_TileManager.m_Names[tileInfo.index].Printf("%s", pName);
#endif

	return true;
}

bool C_Device::CreateDepthTile(C_TileInfo& tileInfo, U32 size, U8 location, U32 offset, U32 pitch, E_Multisample multisample, const char* pName)
{
	if(!AllocTileInfo(&tileInfo, size))
		return false;

	tileInfo.location		= location;
	tileInfo.offset			= offset;
	tileInfo.size			= size;
	tileInfo.pitch			= pitch;

	switch(multisample)
	{
	case E_Multisample_2:
		tileInfo.comp		= CELL_GCM_COMPMODE_Z32_SEPSTENCIL_DIAGONAL;
		break;
	case E_Multisample_4:
		tileInfo.comp		= CELL_GCM_COMPMODE_Z32_SEPSTENCIL_ROTATED;
		break;
	default:
		tileInfo.comp		= CELL_GCM_COMPMODE_DISABLED;
		break;
	}

	if(!SetTileInfo(&tileInfo))
		return false;
	if(!BindTileInfo(&tileInfo))
		return false;

#if !CFG_GOLD
	m_TileManager.m_Names[tileInfo.index].Printf("%s", pName);
#endif

	return true;
}

void C_Device::DestroyTile(C_TileInfo& tileInfo)
{
	UnbindTileInfo(&tileInfo);
	FreeTileInfo(&tileInfo);
}

bool C_Device::CreateRenderBuffer(C_RenderBuffer* pRenderBuffer, C_RenderBufferSetup& renderBufferSetup)
{
	BIZ_ASSERT(pRenderBuffer);

	if(renderBufferSetup.m_pColourMemory) // User specified render buffer address so make indirect
		pRenderBuffer->m_Flags |= E_Flags_Indirect;

	for(U32 i=0; i<4; i++)
	{
		if(pRenderBuffer->m_ColourSurfaceFormat[i] != E_SurfaceFormat_Unknown)
		{
			U32 surfaceFlags = pRenderBuffer->m_Flags | pRenderBuffer->m_ColourFlags[i] | E_Flags_Colour; // This is a colour render target

			if(pRenderBuffer->m_ColourTileInfo[i].enable) // We're using tiling
				surfaceFlags |= E_Flags_Tiled;
			
			U32 surfaceSize = GetSurfaceSize(
				pRenderBuffer->m_Width,
				pRenderBuffer->m_Height,
				surfaceFlags,
				pRenderBuffer->m_ColourSurfaceFormat[i],
				pRenderBuffer->m_Multisample);

			if (surfaceFlags & E_Flags_ReadWrite)
				surfaceSize *= 2;
			
			U8* pSurfaceAddress = NULL;
			
			if(pRenderBuffer->m_pColourAddress[i]) // User specified colour buffer address so make indirect
			{
				pSurfaceAddress = pRenderBuffer->m_pColourAddress[i];
				surfaceFlags |= E_Flags_Indirect;
			}
			else
			if(renderBufferSetup.m_pColourMemory) // User specified render buffer address so make indirect and update render buffer memory
			{
				BIZ_ASSERT(renderBufferSetup.m_ColourMemoryUsed < renderBufferSetup.m_ColourMemorySize);
				pSurfaceAddress = &renderBufferSetup.m_pColourMemory[renderBufferSetup.m_ColourMemoryUsed];
				surfaceFlags |= E_Flags_Indirect;

				renderBufferSetup.m_ColourMemoryUsed += surfaceSize;
				BIZ_ASSERT(renderBufferSetup.m_ColourMemoryUsed <= renderBufferSetup.m_ColourMemorySize);
			}

			if(!CreateRenderTexture2D(
				pRenderBuffer->m_Width,
				pRenderBuffer->m_Height,
				pRenderBuffer->m_Levels,
				surfaceFlags,
				pRenderBuffer->m_ColourTextureFormat[i],
				pRenderBuffer->m_ColourSurfaceFormat[i],
				pRenderBuffer->m_Multisample,
				NULL,
				&pRenderBuffer->m_ColourBuffer[i],
				pSurfaceAddress,
				surfaceSize))
			{
				return false;
			}

			if(pRenderBuffer->m_ColourTileInfo[i].enable && !pRenderBuffer->m_ColourTileInfo[i].indirect)
			{
				#if !CFG_GOLD
				Util::C_String colourName = "";
				colourName.Printf("%sColour%i", pRenderBuffer->m_Name.Chars(), i);
				const char* pName = colourName.Chars();
				#else
				const char* pName = NULL;
				#endif

				if(!CreateColourTile(pRenderBuffer->m_ColourTileInfo[i],
									surfaceSize,
									FlagsToLocation(pRenderBuffer->m_ColourBuffer[i].GetSurface().GetFlags()),
									pRenderBuffer->m_ColourBuffer[i].GetSurface().GetOffset(),
									pRenderBuffer->m_ColourBuffer[i].GetSurface().GetPitch(),
									pRenderBuffer->m_ColourBuffer[i].GetSurface().GetMultisample(),
									pName))
					return false;
			}

			#if TRACK_DISPLAY2_RESOURCES
			if(!(pRenderBuffer->m_Flags & E_Flags_DoNotTrack))
			{
				Util::C_String colourName = "";
				colourName.Printf("%sColour%iRB", pRenderBuffer->m_Name.Chars(), i);
				SetSurfaceName(pRenderBuffer->GetColourSurface(i), colourName);
				SetTextureName(pRenderBuffer->GetColourTexture(i), colourName);
				SetRenderTexture2DName(&pRenderBuffer->m_ColourBuffer[i], colourName);
			}
			#endif
		}
	}

	if(pRenderBuffer->m_DepthStencilSurfaceFormat != E_SurfaceFormat_Unknown)
	{
		U32 surfaceFlags = pRenderBuffer->m_Flags | pRenderBuffer->m_DepthStencilFlags | E_Flags_DepthStencil; // This is a depth stencil render target

		if(pRenderBuffer->m_DepthStencilTileInfo.enable) // We're using tiling
			surfaceFlags |= E_Flags_Tiled;

		U32 surfaceSize = GetSurfaceSize(
			pRenderBuffer->m_Width,
			pRenderBuffer->m_Height,
			surfaceFlags,
			pRenderBuffer->m_DepthStencilSurfaceFormat,
			pRenderBuffer->m_Multisample);

		U8* pSurfaceAddress = NULL;
			
		if(pRenderBuffer->m_pDepthStencilAddress) // User specified depth stencil buffer address so make indirect
		{
			pSurfaceAddress = pRenderBuffer->m_pDepthStencilAddress;
			surfaceFlags |= E_Flags_Indirect;
		}
		else
		if(renderBufferSetup.m_pDepthMemory) // User specified render buffer address so make indirect and update render buffer memory
		{
			BIZ_ASSERT(renderBufferSetup.m_DepthMemoryUsed < renderBufferSetup.m_DepthMemorySize);
			pSurfaceAddress = &renderBufferSetup.m_pDepthMemory[renderBufferSetup.m_DepthMemoryUsed];
			surfaceFlags |= E_Flags_Indirect;

			renderBufferSetup.m_DepthMemoryUsed += surfaceSize;
			BIZ_ASSERT(renderBufferSetup.m_DepthMemoryUsed <= renderBufferSetup.m_DepthMemorySize);
		}

		if(!CreateRenderTexture2D(
			pRenderBuffer->m_Width,
			pRenderBuffer->m_Height,
			pRenderBuffer->m_Levels,
			surfaceFlags,
			pRenderBuffer->m_DepthStencilTextureFormat,
			pRenderBuffer->m_DepthStencilSurfaceFormat,
			pRenderBuffer->m_Multisample,
			NULL,
			&pRenderBuffer->m_DepthStencilBuffer,
			pSurfaceAddress,
			surfaceSize))
		{
			return false;
		}

		// Shaders sampling depth texture in A8R8G8B8 format expect this remap...
		pRenderBuffer->m_DepthStencilBuffer.GetTexture().m_gcmTexture.remap =
			CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_A << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_B;

		if(pRenderBuffer->m_DepthStencilTileInfo.enable && !pRenderBuffer->m_DepthStencilTileInfo.indirect)
		{
			#if !CFG_GOLD
			Util::C_String colourName = "";
			colourName.Printf("%sDepthStencil", pRenderBuffer->m_Name.Chars());
			const char* pName = colourName.Chars();
			#else
			const char* pName = NULL;
			#endif

			if(!CreateDepthTile(pRenderBuffer->m_DepthStencilTileInfo,
								surfaceSize,
								FlagsToLocation(pRenderBuffer->m_DepthStencilBuffer.GetSurface().GetFlags()),
								pRenderBuffer->m_DepthStencilBuffer.GetSurface().GetOffset(),
								pRenderBuffer->m_DepthStencilBuffer.GetSurface().GetPitch(),
								pRenderBuffer->m_DepthStencilBuffer.GetSurface().GetMultisample(),
								pName))
				return false;
		}

		if(pRenderBuffer->m_DepthStencilZcullInfo.enable)
		{
			if(!AllocZcullInfo(&pRenderBuffer->m_DepthStencilZcullInfo, surfaceSize))
				return false;

			#if !CFG_GOLD
			Util::C_String depthStencilName = "";
			depthStencilName.Printf("%sDepthStencil", pRenderBuffer->m_Name.Chars());
			m_ZcullManager.m_Names[pRenderBuffer->m_DepthStencilZcullInfo.index] = depthStencilName;
			#endif

			BIZ_ASSERT(pRenderBuffer->m_DepthStencilBuffer.GetSurface().GetFormat() == E_SurfaceFormat_D24S8);

			pRenderBuffer->m_DepthStencilZcullInfo.size			= pRenderBuffer->m_DepthStencilBuffer.GetSurface().GetSize();
			pRenderBuffer->m_DepthStencilZcullInfo.offset		= pRenderBuffer->m_DepthStencilBuffer.GetSurface().GetOffset();
			pRenderBuffer->m_DepthStencilZcullInfo.width		= ALIGN_SIZE(pRenderBuffer->m_DepthStencilBuffer.GetSurface().GetWidth(), 64);
			pRenderBuffer->m_DepthStencilZcullInfo.height		= ALIGN_SIZE(pRenderBuffer->m_DepthStencilBuffer.GetSurface().GetHeight(), 64);
			pRenderBuffer->m_DepthStencilZcullInfo.aaFormat		= pRenderBuffer->m_DepthStencilBuffer.GetSurface().GetMultisample();

			if(!BindZcullInfo(&pRenderBuffer->m_DepthStencilZcullInfo))
				return false;
		}

		#if TRACK_DISPLAY2_RESOURCES
		if(!(pRenderBuffer->m_Flags & E_Flags_DoNotTrack))
		{
			Util::C_String depthStencilName = "";
			depthStencilName.Printf("%sDepthStencilRB", pRenderBuffer->m_Name.Chars());
			SetSurfaceName(pRenderBuffer->GetDepthStencilSurface(), depthStencilName);
			SetTextureName(pRenderBuffer->GetDepthStencilTexture(), depthStencilName);
			SetRenderTexture2DName(&pRenderBuffer->m_DepthStencilBuffer, depthStencilName);
		}
		#endif

		BIZ_ASSERT(surfaceSize == pRenderBuffer->m_DepthStencilBuffer.GetSurface().GetSize());
		BIZ_ASSERT(surfaceSize == pRenderBuffer->m_DepthStencilBuffer.GetTexture().GetSize());
	}

	#if TRACK_DISPLAY2_RESOURCES
	if(!(pRenderBuffer->m_Flags & E_Flags_DoNotTrack))
	{
		InsertRenderBuffer(pRenderBuffer);
		SetRenderBufferName(pRenderBuffer, pRenderBuffer->m_Name);
	}
	#endif
	
	return true;
}

void C_Device::DestroyRenderBuffer(C_RenderBuffer* pRenderBuffer)
{
	BIZ_ASSERT(pRenderBuffer);

	if(pRenderBuffer->IsCreated())
	{
		for(U32 i=0; i<4; i++)
		{
			if(pRenderBuffer->m_ColourSurfaceFormat[i] != E_SurfaceFormat_Unknown)
			{
				if(pRenderBuffer->m_ColourTileInfo[i].enable && !pRenderBuffer->m_ColourTileInfo[i].indirect)
				{
					DestroyTile(pRenderBuffer->m_ColourTileInfo[i]);
				}

				DestroyRenderTexture2D(&pRenderBuffer->m_ColourBuffer[i]);

				pRenderBuffer->m_ColourSurfaceFormat[i] = E_SurfaceFormat_Unknown;
			}
		}

		if(pRenderBuffer->m_DepthStencilSurfaceFormat != E_SurfaceFormat_Unknown)
		{
			if(pRenderBuffer->m_DepthStencilTileInfo.enable && !pRenderBuffer->m_DepthStencilTileInfo.indirect)
			{
				DestroyTile(pRenderBuffer->m_DepthStencilTileInfo);
			}
			
			if(pRenderBuffer->m_DepthStencilZcullInfo.enable)
			{
				UnbindZcullInfo(&pRenderBuffer->m_DepthStencilZcullInfo);

				FreeZcullInfo(&pRenderBuffer->m_DepthStencilZcullInfo);
			}

			DestroyRenderTexture2D(&pRenderBuffer->m_DepthStencilBuffer);

			pRenderBuffer->m_DepthStencilSurfaceFormat = E_SurfaceFormat_Unknown;
		}

		#if TRACK_DISPLAY2_RESOURCES
		if(!(pRenderBuffer->m_Flags & E_Flags_DoNotTrack))
			RemoveRenderBuffer(pRenderBuffer);
		#endif
	}
	
	pRenderBuffer->SetToDefaults(); // Make sure the render buffer gets cleared when destroyed
}

bool C_Device::AllocResourceMemory(const E_ResourceType eResourceType, U32& flags, const U32 size, U8** ppAddress)
{
	U32 align;
	E_MemoryType type;
	
	switch(eResourceType)
	{
	case E_ResourceType_Surface:
		align = ALIGNMENT_SURFACE;
		type = E_MemoryType_Surface;
		break;
	case E_ResourceType_Texture:
	case E_ResourceType_Texture2D:
	case E_ResourceType_Texture3D:
	case E_ResourceType_TextureCube:
		align = ALIGNMENT_TEXTURE;
		type = E_MemoryType_Texture;
		break;
	case E_ResourceType_VertexBuffer:
		align = ALIGNMENT_VERTEXBUFFER;
		type = E_MemoryType_VertexBuffer;
		break;
	case E_ResourceType_IndexBuffer:
		align = ALIGNMENT_INDEXBUFFER;
		type = E_MemoryType_IndexBuffer;
		break;
	case E_ResourceType_VertexShader:
		align = ALIGNMENT_VERTEXSHADER;
		type = E_MemoryType_VertexShader;
		break;
	case E_ResourceType_PixelShader:
		align = ALIGNMENT_PIXELSHADER;
		type = E_MemoryType_PixelShader;
		break;
	default:
		LOGERROR(g_Log, "Unsupported resource type %i when allocating memory of size %i\n", eResourceType, size);
		return false;
	}

	if(flags & E_Flags_Tiled)
		align = 65536;

	bool success = false;

	if(flags & E_Flags_System)
	{
		flags &= ~E_Flags_Video;
		
		success = AllocMemory(size, align, flags, ppAddress, type); // Alloc out of main

		if(!success && !(flags & E_Flags_Device))
		{
			flags &= ~E_Flags_System;
			flags |= E_Flags_Video;

			success = AllocMemory(size, align, flags, ppAddress, type); // Alloc out of main failed to try local
		}
	}
	else
	{
		flags &= ~E_Flags_System;
		flags |= E_Flags_Video;
		
		success = AllocMemory(size, align, flags, ppAddress, type); // Alloc out of local
	}

	return success;
}

bool C_Device::AllocMemory(const U32 size, const U32 align, const U32 flags, U8** ppAddress, const E_MemoryType type)
{
	bool success = false;
	
	if(flags & E_Flags_Device)
	{
		if(flags & E_Flags_System)
		{
			success = m_MainDeviceHeap.Alloc(align, size, ppAddress, type, flags);

			if(success)
			{
				BIZ_ASSERT(m_MainDeviceHeap.Inside(size, *ppAddress));
			}
		}
		else
		{
			success = m_LocalDeviceHeap.Alloc(align, size, ppAddress, type, flags);

			if(success)
			{
				BIZ_ASSERT(m_LocalDeviceHeap.Inside(size, *ppAddress));
			}
		}
	}
	else
	{
		if(flags & E_Flags_System)
		{
			success = m_MainHeap.Alloc(align, size, ppAddress, type, flags);

			if(success)
			{
				BIZ_ASSERT(m_MainHeap.Inside(size, *ppAddress));
			}
		}
		else
		{
			success = m_LocalHeap.Alloc(align, size, ppAddress, type, flags);

			if(success)
			{
				BIZ_ASSERT(m_LocalHeap.Inside(size, *ppAddress));
			}
		}

		#if TRACK_DISPLAY2_PROTECTED_MEMORY
		if(success)
		{
			m_ProtectedMemory.CheckMemory(size, *ppAddress);
		}
		#endif
	}

	return success;
}

void C_Device::FreeMemory(const U32 size, U8* pAddress, const E_MemoryType type, const U32 flags)
{
	if(!pAddress)
		return;

	BIZ_ASSERT(size > 0);

	if(m_LocalHeap.Inside(size, pAddress))
	{
		if(m_LocalDeviceHeap.Inside(size, pAddress))
		{
			m_LocalDeviceHeap.Free(size, pAddress, type, flags);
		}
		else
		{
			#if TRACK_DISPLAY2_PROTECTED_MEMORY
			m_ProtectedMemory.CheckMemory(size, pAddress);
			#endif

			m_LocalHeap.Free(size, pAddress, type, flags);
		}
	}
	else
	if(m_MainHeap.Inside(size, pAddress))
	{
		if(m_MainDeviceHeap.Inside(size, pAddress))
		{
			m_MainDeviceHeap.Free(size, pAddress, type, flags);
		}
		else
		{
			#if TRACK_DISPLAY2_PROTECTED_MEMORY
			m_ProtectedMemory.CheckMemory(size, pAddress);
			#endif

			m_MainHeap.Free(size, pAddress, type, flags);
		}
	}
	else
	{
		BIZ_ASSERT_FAIL("Memory of size %i and address 0x%8x is not from the local or main heap", size, pAddress);
	}
}

bool C_Device::IsProtectedMemory(const U32 size, U8* pAddress)
{
	#if TRACK_DISPLAY2_PROTECTED_MEMORY
	m_ProtectedMemory.CheckMemory(size, pAddress);
	#endif
	
	return false;
}

void C_Device::InitBlendModes()
{
	for(U32 i=0; i<E_BlendMode_Count; i++)
	{
		C_BlendMode& blendMode = m_BlendModes[i];

		C_AlphaTestState& alphaTestState = blendMode.m_AlphaTestState;
		C_BlendState& blendState = blendMode.m_BlendState;

		blendMode.m_AlphaTestState.SetDefaultStates();
		blendMode.m_BlendState.SetDefaultStates();

		switch(i)
		{
		case E_BlendMode_Solid:
			break;
		case E_BlendMode_SolidAlphaTest:
			blendMode.m_AlphaTestEnable						= E_Bool_True;
			blendMode.m_AlphaTestState.m_AlphaTestCmpFunc	= E_CmpFunc_Greater;
			blendMode.m_AlphaTestState.m_AlphaTestRef		= 0x80;
			break;
		case E_BlendMode_Alpha:
			blendMode.m_AlphaTestEnable						= E_Bool_True;
			blendMode.m_AlphaTestState.m_AlphaTestCmpFunc	= E_CmpFunc_Greater;
			blendMode.m_AlphaTestState.m_AlphaTestRef		= 0x00;
			blendMode.m_BlendEnable							= E_Bool_True;
			blendMode.m_BlendState.m_BlendColourSrc			= E_Blend_SrcAlpha;
			blendMode.m_BlendState.m_BlendColourDst			= E_Blend_InvSrcAlpha;
			blendMode.m_BlendState.m_BlendColourOp			= E_BlendOp_Add;
			break;
		case E_BlendMode_AlphaPreMultiply:
			blendMode.m_AlphaTestEnable						= E_Bool_True;
			blendMode.m_AlphaTestState.m_AlphaTestCmpFunc	= E_CmpFunc_Greater;
			blendMode.m_AlphaTestState.m_AlphaTestRef		= 0x00;
			blendMode.m_BlendEnable							= E_Bool_True;
			blendMode.m_BlendState.m_BlendColourSrc			= E_Blend_One;
			blendMode.m_BlendState.m_BlendColourDst			= E_Blend_InvSrcAlpha;
			blendMode.m_BlendState.m_BlendColourOp			= E_BlendOp_Add;
			break;
		case E_BlendMode_Additive:
			blendMode.m_AlphaTestEnable						= E_Bool_True;
			blendMode.m_AlphaTestState.m_AlphaTestCmpFunc	= E_CmpFunc_Greater;
			blendMode.m_AlphaTestState.m_AlphaTestRef		= 0x00;
			blendMode.m_BlendEnable							= E_Bool_True;
			blendMode.m_BlendState.m_BlendColourSrc			= E_Blend_One;
			blendMode.m_BlendState.m_BlendColourDst			= E_Blend_One;
			blendMode.m_BlendState.m_BlendColourOp			= E_BlendOp_Add;
			break;
		case E_BlendMode_AlphaAdditive:
			blendMode.m_AlphaTestEnable						= E_Bool_True;
			blendMode.m_AlphaTestState.m_AlphaTestCmpFunc	= E_CmpFunc_Greater;
			blendMode.m_AlphaTestState.m_AlphaTestRef		= 0x00;
			blendMode.m_BlendEnable							= E_Bool_True;
			blendMode.m_BlendState.m_BlendColourSrc			= E_Blend_SrcAlpha;
			blendMode.m_BlendState.m_BlendColourDst			= E_Blend_One;
			blendMode.m_BlendState.m_BlendColourOp			= E_BlendOp_Add;
			break;
		case E_BlendMode_AlphaSubtractive:
			blendMode.m_AlphaTestEnable						= E_Bool_True;
			blendMode.m_AlphaTestState.m_AlphaTestCmpFunc	= E_CmpFunc_Greater;
			blendMode.m_AlphaTestState.m_AlphaTestRef		= 0x00;
			blendMode.m_BlendEnable							= E_Bool_True;
			blendMode.m_BlendState.m_BlendColourSrc			= E_Blend_SrcAlpha;
			blendMode.m_BlendState.m_BlendColourDst			= E_Blend_One;
			blendMode.m_BlendState.m_BlendColourOp			= E_BlendOp_RevSubtract;
			break;
		case E_BlendMode_Subtractive:
			blendMode.m_BlendEnable							= E_Bool_True;
			blendMode.m_BlendState.m_BlendColourSrc			= E_Blend_One;
			blendMode.m_BlendState.m_BlendColourDst			= E_Blend_One;
			blendMode.m_BlendState.m_BlendColourOp			= E_BlendOp_RevSubtract;
			break;
		case E_BlendMode_Multiply:
			blendMode.m_BlendEnable							= E_Bool_True;
			blendMode.m_BlendState.m_BlendColourSrc			= E_Blend_Zero;
			blendMode.m_BlendState.m_BlendColourDst			= E_Blend_SrcColour;
			blendMode.m_BlendState.m_BlendColourOp			= E_BlendOp_Add;
			break;
		}
	}
}

void C_Device::BlockUntilIdle()
{
	static U32 labelValue = 1;

	GpuSetLabelOnRasterEnd(E_Label_BlockUntilIdle, labelValue);
	GpuWaitOnLabel(E_Label_BlockUntilIdle, labelValue);

	labelValue++;
	
	GpuSetLabelOnRasterEnd(E_Label_BlockUntilIdle, labelValue);
	cellGcmFlush(gCellGcmCurrentContext);
	CpuWaitOnLabel(E_Label_BlockUntilIdle, labelValue);
	
	labelValue++;
}

U32 C_Device::GetLastError() const
{
	return m_LastError;
}

void C_Device::AllowOutOfMemory(const bool allowOutOfMemory)
{
	m_AllowOutOfMemory = allowOutOfMemory;
}

void C_Device::DumpMemoryHeaps()
{
#if !CFG_GOLD
	m_LocalHeap.Dump("LocalHeap");
	m_MainHeap.Dump("MainHeap");
	m_CmdBufHeap.Dump("CmdBufHeap");
	m_MainDeviceHeap.Dump("MainDeviceHeap");
	m_LocalDeviceHeap.Dump("LocalDeviceHeap");
#endif
}

void C_Device::ShowMemoryHeaps()
{
#if !CFG_GOLD
	LOGINFO(g_Log, "Heaps : Local (%0.2fmb / %0.2fmb) Main (%0.2fmb / %0.2fmb) CmdBuf (%0.2fmb / %0.2fmb) MainDevice (%0.2fmb / %0.2fmb) LocalDevice (%0.2fmb / %0.2fmb)",
		(F32)(m_LocalHeap.GetUsed())		/ (1024.0f * 1024.0f),
		(F32)(m_LocalHeap.GetSize())		/ (1024.0f * 1024.0f),
		(F32)(m_MainHeap.GetUsed())			/ (1024.0f * 1024.0f),
		(F32)(m_MainHeap.GetSize())			/ (1024.0f * 1024.0f),
		(F32)(m_CmdBufHeap.GetUsed())		/ (1024.0f * 1024.0f),
		(F32)(m_CmdBufHeap.GetSize())		/ (1024.0f * 1024.0f),
		(F32)(m_MainDeviceHeap.GetUsed())	/ (1024.0f * 1024.0f),
		(F32)(m_MainDeviceHeap.GetSize())	/ (1024.0f * 1024.0f),
		(F32)(m_LocalDeviceHeap.GetUsed())	/ (1024.0f * 1024.0f),
		(F32)(m_LocalDeviceHeap.GetSize())	/ (1024.0f * 1024.0f));
#endif
}

bool C_Device::SetTileInfo(C_TileInfo* pTileInfo)
{
	BIZ_ASSERT(pTileInfo);
	BIZ_ASSERT(pTileInfo->index <= 14);
	BIZ_ASSERT(pTileInfo->location == CELL_GCM_LOCATION_MAIN || pTileInfo->location == CELL_GCM_LOCATION_LOCAL);
	BIZ_ASSERT(pTileInfo->offset % 65536 == 0);
	BIZ_ASSERT(pTileInfo->size % 65536 == 0);
	BIZ_ASSERT(pTileInfo->pitch % 256 == 0);
	BIZ_ASSERT(pTileInfo->base <= 2047);
	BIZ_ASSERT(pTileInfo->bank <= 3);

	S32 result = cellGcmSetTileInfo(
		pTileInfo->index,
		pTileInfo->location,
		pTileInfo->offset,
		pTileInfo->size,
		pTileInfo->pitch,
		pTileInfo->comp,
		pTileInfo->base,
		pTileInfo->bank);

	switch(result)
	{
	case CELL_OK:
		return true;
	case CELL_GCM_ERROR_INVALID_VALUE:
		BIZ_ASSERT_FAIL("cellGcmSetTileInfo - CELL_GCM_ERROR_INVALID_VALUE");
		return false;
	case CELL_GCM_ERROR_INVALID_ALIGNMENT:
		BIZ_ASSERT_FAIL("cellGcmSetTileInfo - CELL_GCM_INVALID_ALIGNMENT");
		return false;
	case CELL_GCM_ERROR_INVALID_ENUM:
		BIZ_ASSERT_FAIL("cellGcmSetTileInfo - CELL_GCM_INVALID_ENUM ");
		return false;
	default:
		BIZ_ASSERT_FAIL("cellGcmSetTileInfo");
		return false;
	}
}

bool C_Device::BindTileInfo(C_TileInfo* pTileInfo)
{
	BIZ_ASSERT(pTileInfo);
	BIZ_ASSERT(pTileInfo->index <= 14);

	S32 result = cellGcmBindTile(pTileInfo->index);

	switch(result)
	{
	case CELL_OK:
		return true;
	case CELL_GCM_ERROR_ADDRESS_OVERWRAP:
		BIZ_ASSERT_FAIL("cellGcmBindTile - CELL_GCM_ERROR_ADDRESS_OVERWRAP");
		return false;
	case CELL_GCM_ERROR_INVALID_VALUE:
		BIZ_ASSERT_FAIL("cellGcmBindTile - CELL_GCM_ERROR_INVALID_VALUE");
		return false;
	case EINVAL:
		BIZ_ASSERT_FAIL("cellGcmBindTile - EINVAL");
		return false;
	default:
		BIZ_ASSERT_FAIL("cellGcmBindTile");
		return false;
	}
}

bool C_Device::UnbindTileInfo(C_TileInfo* pTileInfo)
{
	BIZ_ASSERT(pTileInfo);
	BIZ_ASSERT(pTileInfo->index <= 14);

	S32 result = cellGcmUnbindTile(pTileInfo->index);

	switch(result)
	{
	case CELL_OK:
		return true;
	case CELL_GCM_ERROR_INVALID_VALUE :
		BIZ_ASSERT_FAIL("cellGcmUnbindTile - CELL_GCM_ERROR_INVALID_VALUE");
		return false;
	default:
		BIZ_ASSERT_FAIL("cellGcmUnbindTile");
		return false;
	}
}

bool C_Device::AllocTileInfo(C_TileInfo* pTileInfo, const U32 sizeInBytes)
{
	BIZ_ASSERT(pTileInfo);

	if(pTileInfo->index == INVALID_TILE_INDEX && !(pTileInfo->flags & E_TileFlags_UserIndex))
	{
		pTileInfo->index = m_TileManager.AllocIndex();
	}

	if(pTileInfo->base == INVALID_TILE_MEMORY && !(pTileInfo->flags & E_TileFlags_UserMemory))
	{
		pTileInfo->base = m_TileManager.AllocMemory(sizeInBytes);
	}
	
	if(pTileInfo->index == INVALID_TILE_INDEX || pTileInfo->base == INVALID_TILE_MEMORY)
	{
		FreeTileInfo(pTileInfo);
		return false;
	}
	
	return true;
}


void C_Device::FreeTileInfo(C_TileInfo* pTileInfo)
{
	BIZ_ASSERT(pTileInfo);

	if(pTileInfo->index != INVALID_TILE_INDEX && !(pTileInfo->flags & E_TileFlags_UserIndex))
	{
		m_TileManager.FreeIndex(pTileInfo->index);
		pTileInfo->index = INVALID_TILE_INDEX;
	}

	if(pTileInfo->base != INVALID_TILE_MEMORY && !(pTileInfo->flags & E_TileFlags_UserMemory))
	{
		BIZ_ASSERT(pTileInfo->size > 0);
		m_TileManager.FreeMemory(pTileInfo->size, pTileInfo->base);
		pTileInfo->base = INVALID_TILE_MEMORY;
	}
}

bool C_Device::BindZcullInfo(C_ZcullInfo* pZcullInfo)
{
	BIZ_ASSERT(pZcullInfo);
	BIZ_ASSERT(pZcullInfo->index <= 7);
	BIZ_ASSERT(pZcullInfo->offset % 4096 == 0);
	BIZ_ASSERT(pZcullInfo->width % 64 == 0);
	BIZ_ASSERT(pZcullInfo->height % 64 == 0);
	BIZ_ASSERT(pZcullInfo->cullStart % 4096 == 0);
	BIZ_ASSERT(pZcullInfo->cullStart <= 3145728);
	BIZ_ASSERT(pZcullInfo->sRef <= 0xFF);
	BIZ_ASSERT(pZcullInfo->sMask <= 0xFF);
	BIZ_ASSERT(
		pZcullInfo->zFormat == CELL_GCM_ZCULL_Z16 ||
		pZcullInfo->zFormat == CELL_GCM_ZCULL_Z24S8);
	BIZ_ASSERT(
		pZcullInfo->aaFormat == CELL_GCM_SURFACE_CENTER_1 ||
		pZcullInfo->aaFormat == CELL_GCM_SURFACE_DIAGONAL_CENTERED_2 ||
		pZcullInfo->aaFormat == CELL_GCM_SURFACE_SQUARE_CENTERED_4 ||
		pZcullInfo->aaFormat == CELL_GCM_SURFACE_SQUARE_ROTATED_4);
	BIZ_ASSERT(
		pZcullInfo->zcullDir == CELL_GCM_ZCULL_LESS ||
		pZcullInfo->zcullDir == CELL_GCM_ZCULL_GREATER);
	BIZ_ASSERT(
		pZcullInfo->zcullFormat == CELL_GCM_ZCULL_MSB ||
		pZcullInfo->zcullFormat == CELL_GCM_ZCULL_LONES);
	BIZ_ASSERT(
		pZcullInfo->zcullFormat == CELL_GCM_SCULL_SFUNC_NEVER ||
		pZcullInfo->zcullFormat == CELL_GCM_SCULL_SFUNC_LESS ||
		pZcullInfo->zcullFormat == CELL_GCM_SCULL_SFUNC_EQUAL ||
		pZcullInfo->zcullFormat == CELL_GCM_SCULL_SFUNC_LEQUAL ||
		pZcullInfo->zcullFormat == CELL_GCM_SCULL_SFUNC_GREATER ||
		pZcullInfo->zcullFormat == CELL_GCM_SCULL_SFUNC_NOTEQUAL ||
		pZcullInfo->zcullFormat == CELL_GCM_SCULL_SFUNC_GEQUAL ||
		pZcullInfo->zcullFormat == CELL_GCM_SCULL_SFUNC_ALWAYS);
	
	S32 result = cellGcmBindZcull(
		pZcullInfo->index,
		pZcullInfo->offset,
		pZcullInfo->width,
		pZcullInfo->height,
		pZcullInfo->cullStart,
		pZcullInfo->zFormat,
		pZcullInfo->aaFormat,
		pZcullInfo->zcullDir,
		pZcullInfo->zcullFormat,
		pZcullInfo->sFunc,
		pZcullInfo->sRef,
		pZcullInfo->sMask);

	switch(result)
	{
	case CELL_OK:
		return true;
	case CELL_GCM_ERROR_INVALID_VALUE:
		BIZ_ASSERT_FAIL("cellGcmBindZcull - CELL_GCM_ERROR_INVALID_VALUE");
		return false;
	case CELL_GCM_ERROR_INVALID_ALIGNMENT:
		BIZ_ASSERT_FAIL("cellGcmBindZcull - CELL_GCM_ERROR_INVALID_ALIGNMENT");
		return false;
	case CELL_GCM_ERROR_INVALID_ENUM:
		BIZ_ASSERT_FAIL("cellGcmBindZcull - CELL_GCM_ERROR_INVALID_ENUM");
		return false;
	case EINVAL:
		BIZ_ASSERT_FAIL("cellGcmBindZcull - EINVAL");
		return false;
	default:
		BIZ_ASSERT_FAIL("cellGcmBindZcull");
		return false;
	}
}

bool C_Device::UnbindZcullInfo(C_ZcullInfo* pZcullInfo)
{
	BIZ_ASSERT(pZcullInfo);
	BIZ_ASSERT(pZcullInfo->index <= 7);
	
	S32 result = cellGcmUnbindZcull(pZcullInfo->index);

	switch(result)
	{
	case CELL_OK:
		return true;
	case CELL_GCM_ERROR_INVALID_VALUE:
		BIZ_ASSERT_FAIL("cellGcmUnbindZcull - CELL_GCM_ERROR_INVALID_VALUE");
		return false;
	default:
		BIZ_ASSERT_FAIL("cellGcmUnbindZcull");
		return false;
	}
}

bool C_Device::AllocZcullInfo(C_ZcullInfo* pZcullInfo, const U32 sizeInBytes)
{
	BIZ_ASSERT(pZcullInfo);

	if(pZcullInfo->index == INVALID_ZCULL_INDEX && !(pZcullInfo->flags & E_ZcullFlags_UserIndex))
	{
		pZcullInfo->index = m_ZcullManager.AllocIndex();
	}

	if(pZcullInfo->cullStart == INVALID_ZCULL_MEMORY && !(pZcullInfo->flags & E_ZcullFlags_UserMemory))
	{
		pZcullInfo->cullStart = m_ZcullManager.AllocMemory(sizeInBytes);
	}
	
	if(pZcullInfo->index == INVALID_ZCULL_INDEX || pZcullInfo->cullStart == INVALID_ZCULL_MEMORY)
	{
		FreeZcullInfo(pZcullInfo);
		return false;
	}
	
	return true;
}

void C_Device::FreeZcullInfo(C_ZcullInfo* pZcullInfo)
{
	BIZ_ASSERT(pZcullInfo);

	if(pZcullInfo->index != INVALID_ZCULL_INDEX && !(pZcullInfo->flags & E_ZcullFlags_UserIndex))
	{
		m_ZcullManager.FreeIndex(pZcullInfo->index);
		pZcullInfo->index = INVALID_ZCULL_INDEX;
	}

	if(pZcullInfo->cullStart != INVALID_ZCULL_MEMORY && !(pZcullInfo->flags & E_ZcullFlags_UserMemory))
	{
		BIZ_ASSERT(pZcullInfo->size > 0);
		m_ZcullManager.FreeMemory(pZcullInfo->size, pZcullInfo->cullStart);
		pZcullInfo->cullStart = INVALID_ZCULL_MEMORY;
	}
}

bool C_Device::CheckMemoryGuards() const
{
	if(!m_MainHeap.CheckMemoryGuards())
		return false;
	if(!m_LocalHeap.CheckMemoryGuards())
		return false;
	if(!m_MainDeviceHeap.CheckMemoryGuards())
		return false;
	if(!m_LocalDeviceHeap.CheckMemoryGuards())
		return false;
	return true;
}

C_TileManager& C_Device::GetTileManager()
{
	return m_TileManager;
}

C_ZcullManager& C_Device::GetZcullManager()
{
	return m_ZcullManager;
}

C_DeviceManagerParams::C_DeviceManagerParams()
{
	m_cbSize					= 1024*256;
	#if USE_GCM_USER_COMMAND_BUFFER
	m_ioSize					= 1024*1024*8;
	#else
	m_ioSize					= 1024*1024*2;
	#endif
}

C_DeviceManagerParams::~C_DeviceManagerParams()
{
}

C_DeviceManager::C_DeviceManager() :
	m_cbSize(0),
	m_ioSize(0),
	m_ioAddress(NULL)
{
}

C_DeviceManager::~C_DeviceManager()
{
	BIZ_ASSERT(!m_ioAddress);
}

bool C_DeviceManager::Create(const C_DeviceManagerParams& params)
{
	BIZ_ASSERT(!m_ioAddress);

	m_cbSize = params.m_cbSize;
	m_ioSize = params.m_ioSize;

	m_ioAddress = BizAlignedMalloc(m_ioSize, 1024*1024);
	if(!m_ioAddress)
	{
		LOGFATAL(g_Log, "Failed to alloc gcm io memory %i\n", m_ioSize);
		return false;
	}

	#if USE_GCM_USER_COMMAND_BUFFER
	if(cellGcmInit(m_cbSize, m_ioSize, m_ioAddress) != CELL_OK)
	#else
	if(cellGcmInit(m_ioSize-4096, m_ioSize, m_ioAddress) != CELL_OK)
	#endif
	{
		BizAlignedFree(m_ioAddress);
		m_ioAddress = NULL;

		LOGFATAL(g_Log, "Failed to initialise gcm\n");
		return false;
	}

	#if USE_GCM_USER_COMMAND_BUFFER
	U32 userCommandBufferSize = (m_ioSize - m_cbSize - 0x2000) / 2;
	U32* pUserCommandBufferAddress = (U32*)((U32)m_ioAddress + m_cbSize);
	g_UserCommandBuffer.Init(userCommandBufferSize, pUserCommandBufferAddress);
	std::fstream file;
	file.open("/app_home/GCMCommandBufferOffset.txt", std::ios::out);
	if(file.is_open())
	{
		file << (U32)pUserCommandBufferAddress;
		file.close();
	}
	#else
	/*std::fstream file;
	file.open("/app_home/GCMCommandBufferOffset.txt", std::ios::out);
	if(file.is_open())
	{
		file << (U32)m_ioAddress;
		file.close();
	}*/
	#endif

	cellGcmGetConfiguration(&g_gcmConfig);

	SetupDebugging();
	
	return true;
}

void C_DeviceManager::Destroy()
{
	if(m_ioAddress)
	{
		BizAlignedFree(m_ioAddress);
		m_ioAddress = NULL;
	}
}

void C_DeviceManager::SetupDefaultDeviceParams(C_DeviceParams& params)
{
#if 0
	U32 backBufferWidth = 1280;
	U32 backBufferHeight = 720;

	CellVideoOutState gcmVideoState;
	if(cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &gcmVideoState) == CELL_OK)
	{
		CellVideoOutResolution gcmResolution;
		if(cellVideoOutGetResolution(gcmVideoState.displayMode.resolutionId, &gcmResolution) == CELL_OK)
		{
			backBufferWidth = gcmResolution.width;
			backBufferHeight = gcmResolution.height;
		}
		else
		{
			LOGFATAL(g_Log, "SetupDefaultDeviceParams - Failed to get resolution - cellVideoOutGetResolution\n");
		}
	}
	else
	{
		LOGFATAL(g_Log, "SetupDefaultDeviceParams - Failed to get resolution - cellVideoOutGetState\n");
	}

	params.m_BackBufferCount				= 2;
	params.m_BackBufferWidth				= backBufferWidth;
	params.m_BackBufferHeight				= backBufferHeight;
	params.m_BackBufferFormat				= E_SurfaceFormat_A8R8G8B8;
	params.m_DepthStencilFormat				= E_SurfaceFormat_D24S8;
	params.m_MultisampleType				= E_Multisample_None;
	params.m_PresentInterval				= E_PresentInterval_One;
#endif
}

bool C_DeviceManager::GetVideoMode(C_VideoMode* pVideoMode)
{
	BIZ_ASSERT(pVideoMode);

	CellVideoOutState gcmVideoState;
	if(cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &gcmVideoState) != CELL_OK)
	{
		LOGFATAL(g_Log, "Failed to get resolution - cellVideoOutGetState\n");
		return false;
	}

	CellVideoOutResolution gcmResolution;
	if(cellVideoOutGetResolution(gcmVideoState.displayMode.resolutionId, &gcmResolution) != CELL_OK)
	{
		LOGFATAL(g_Log, "Failed to get resolution - cellVideoOutGetResolution\n");
		return false;
	}
	
	pVideoMode->m_Width		= gcmResolution.width;
	pVideoMode->m_Height	= gcmResolution.height;

	switch(gcmVideoState.displayMode.resolutionId)
	{
	case CELL_VIDEO_OUT_RESOLUTION_480:
		pVideoMode->m_HiDef			= false;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_576:
		pVideoMode->m_HiDef			= false;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_720:
		pVideoMode->m_HiDef			= true;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_1080:
		pVideoMode->m_HiDef			= true;;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_960x1080:
		pVideoMode->m_HiDef			= true;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_1280x1080:
		pVideoMode->m_HiDef			= true;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_1440x1080:
		pVideoMode->m_HiDef			= true;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_1600x1080:
		pVideoMode->m_HiDef			= true;
		break;
	default:
		LOGFATAL(g_Log, "Failed to get resolution - Unsupported resolution\n");
		return false;
	}

	switch(gcmVideoState.displayMode.aspect)
	{
	case CELL_VIDEO_OUT_ASPECT_4_3:
		pVideoMode->m_WideScreen = false;
		break;
	default:
		pVideoMode->m_WideScreen = true;
		break;
	}
	
	return true;
}

void C_DeviceManager::SetupDebugging()
{
#if defined(CFG_MASTER)
	cellGcmSetDebugOutputLevel(CELL_GCM_DEBUG_LEVEL0);
	cellGcmSetGraphicsHandler(NULL);
#elif defined(CFG_DEBUG)
	cellGcmSetDebugOutputLevel(CELL_GCM_DEBUG_LEVEL2);
	cellGcmSetGraphicsHandler(GCMGraphicsHandler);
#else
	cellGcmSetDebugOutputLevel(CELL_GCM_DEBUG_LEVEL1);
	cellGcmSetGraphicsHandler(GCMGraphicsHandler);
#endif

#if defined(CELL_GCM_DEBUG)
	gCellGcmDebugCheckCallback = GCMDebugCheckCallback;
	cellGcmDebugCheckEnable(CELL_GCM_TRUE);
#else
	//gCellGcmDebugCheckCallback = NULL;
	//cellGcmDebugCheckEnable(CELL_GCM_FALSE);
#endif
}

} // namespace Display2
