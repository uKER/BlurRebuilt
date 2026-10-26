#include "stdafx.h"

#include "Dynamic/Strip/Manager.h"

#include "Dynamic/Strip/Emitter.h"

#include "Dynamic/Renderer.h"
#include "Dynamic/TypeIndex.h"

#include "Render/State/RenderState.h"

EXTERN_SHADER(vsStrip);
EXTERN_SHADER(psStrip);
EXTERN_SHADER(psStripHDR);
EXTERN_SHADER(psStripHDRU32);

using namespace DynamicEffect;
using namespace DynamicEffect::Strip;

namespace DynamicEffect
{
	extern Debug::Logger &g_Log;

namespace Strip
{
	Debug::Bool g_debugStripEnable("Graphics.Dynamic.Strip.Enable", true);
	Debug::Bool g_debugStripDebugInfo("Graphics.Dynamic.Strip.DebugInfo", false);
	Debug::Bool g_debugStripDebugGraphics("Graphics.Dynamic.Strip.DebugGraphics.Render", true);
}

}

C_Manager::C_Manager(C_Renderer* pRenderer) : I_DynamicEffectManager(pRenderer, E_ManagerTypeIndex_Particle)
{
	MEMORY_TRACKER_PUSH("graphics.dynamic.strip");

	m_BlockManager16.SetBlockSize(16);
	m_BlockManager32.SetBlockSize(64);
	m_BlockManager48.SetBlockSize(64);
	m_BlockManager64.SetBlockSize(64);
	m_BlockManager80.SetBlockSize(64);

	m_BlockManager16.SetElementSize(16);
	m_BlockManager32.SetElementSize(32);
	m_BlockManager48.SetElementSize(48);
	m_BlockManager64.SetElementSize(64);
	m_BlockManager80.SetElementSize(80);
	
	m_BlockManager16.SetMaxBlocks(64);
	m_BlockManager32.SetMaxBlocks(64);
	m_BlockManager48.SetMaxBlocks(64);
	m_BlockManager64.SetMaxBlocks(64);
	m_BlockManager80.SetMaxBlocks(64);

	m_nEmitters = 0;
	
	MEMORY_TRACKER_POP("graphics.dynamic.strip");
}

C_Manager::~C_Manager()
{
	m_EmitterList.Free();
	
	while(m_Emitters.Begin() != m_Emitters.End())
	{
		C_Emitter* pEmitter = (*m_Emitters.Begin());
		delete pEmitter;
	}
}

void C_Manager::ReserveEmitters(const U32 nEmitters)
{
	if(nEmitters > m_nEmitters)
	{
		for(U32 i = m_nEmitters; i < nEmitters; ++i)
		{
			m_EmitterList.Cache(new C_Emitter(this));
		}
		m_nEmitters = nEmitters;
	}
}

bool C_Manager::Create()
{
	RESOURCE_MEMORY_ID("DynamicEffect");

	if (!Display::CreateVertexShaderFromFunc("vsStrip", 0, &m_VertexShader, REF_SHADER(vsStrip)))
		return false;

	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		// choose which set of shaders we need to use - we only have depth fog for shader tonemap at the moment
		if(eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psStripHDRU32", 0, &m_PixelShader, REF_SHADER(psStripHDRU32)))
				return false;
		}
		else if(eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psStripHDR", 0, &m_PixelShader, REF_SHADER(psStripHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psStrip", 0, &m_PixelShader, REF_SHADER(psStrip)))
			return false;
	}

	return true;
}

void C_Manager::Destroy()
{
	if(m_VertexShader.IsCreated())
		Display::Device()->Destroy(&m_VertexShader);
	if(m_PixelShader.IsCreated())
		Display::Device()->Destroy(&m_PixelShader);
}

bool C_Manager::CreateVolatile()
{
	bool bCreated = true;
	
	return bCreated;
}

bool C_Manager::Initialise()
{
	bool bInitialised = true;
	
	return bInitialised;
}

void C_Manager::Shutdown()
{
	
}

void C_Manager::DestroyVolatile()
{
	
}

MathsStorage::Int2 C_Manager::DisplayDebugInfo(MathsStorage::Int2 position) const
{
//#ifndef CFG_SPEED
	
	if(g_debugStripDebugInfo)
	{
		Util::C_String128	str;
		str.Printf("Emitters: %d/%d\0", m_EmitterList.m_Active.Size(), m_EmitterList.m_Active.Size() + m_EmitterList.m_Cache.Size());
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
		position.y += 25;

		str.Printf("BlockManagers:\0");
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
		position.y += 15;

		position = DisplayBlockManagerDebugInfo(position, &m_BlockManager16);
		position = DisplayBlockManagerDebugInfo(position, &m_BlockManager32);
		position = DisplayBlockManagerDebugInfo(position, &m_BlockManager48);
		position = DisplayBlockManagerDebugInfo(position, &m_BlockManager64);
		position = DisplayBlockManagerDebugInfo(position, &m_BlockManager80);
	
		position.y += 15;
	}
//#endif

	return position;
}

MathsStorage::Int2 C_Manager::DisplayBlockManagerDebugInfo(MathsStorage::Int2 position, const BlockManagerBase* pBlockManager) const
{
	Util::C_String128	str;
	str.Printf("%d:\0", pBlockManager->GetElementSize());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y + 10), str);
	str.Printf("%d/%d\0", pBlockManager->GetAllocatedBlocks(), pBlockManager->GetMaxBlocks());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x + 60, position.y + 10), str);
	
	position.x += 140;
	F32 maxWidth = 1024.f - (F32)position.x - 100.f;
	F32 usedWidth = ((F32)maxWidth * ((F32)pBlockManager->GetAllocatedBlocks() / (F32)pBlockManager->GetMaxBlocks()));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(Maths::HVec2(position.x, position.y), Maths::HVec2(usedWidth, 20.f), Maths::RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(Maths::HVec2(position.x + usedWidth, position.y), Maths::HVec2(maxWidth - usedWidth, 20.f), Maths::RGBA(0, 255, 0, 255));
	position.x -= 140;

	position.y += 25;

	return position;
}


// virtual
void C_Manager::CleanEffect(const I_DynamicEffect *pEffect)
{
	for(EmitterList::Iterator pIterator = m_EmitterList.m_Active.Begin(); pIterator != m_EmitterList.m_Active.End(); )
	{
		C_Emitter* pEmitter(&(*pIterator));
		++pIterator;
		//if(pEmitter->GetDefinition()->pEffect == pEffect)
		//	FreeEmitter(pEmitter);
	}
}

// virtual
void C_Manager::CleanNode(const C_Node* pNode)
{
	// remove any emitters which are using this node for their parent transformation
	for(EmitterList::Iterator pIterator = m_EmitterList.m_Active.Begin(); pIterator != m_EmitterList.m_Active.End(); )
	{
		C_Emitter* pEmitter(&(*pIterator));
		++pIterator;
		if(pEmitter->GetParentTransform() == pNode)
			pEmitter->Free();
	}
}

void C_Manager::AddEmitter(C_Emitter *pEmitter)
{
	if(pEmitter->m_pManager == NULL)
		pEmitter->m_pManager = this;

	m_Emitters.Insert(pEmitter);
}

void C_Manager::RemoveEmitter(C_Emitter *pEffect)
{
	Emitters::Iterator it = m_Emitters.Find(pEffect);
	assert(it != m_Emitters.End());
	if(it != m_Emitters.End())
		m_Emitters.Erase(it);
}

void C_Manager::DisplayDebugGraphics() const
{
#ifndef CFG_SPEED
	if(g_debugStripDebugGraphics)
	{
		PERF_EVENT("Debug Graphics");
			
		for(EmitterList::ConstIterator pIterator = m_EmitterList.m_Active.Begin(); pIterator != m_EmitterList.m_Active.End(); ++pIterator)
		{
		} // emitters
	}
#endif
}

void C_Manager::Reset()
{
	U32 nLocalBlocks = 0;
	
	// go through all active emitters and remove them - need to do this last!
	for(EmitterList::Iterator pIterator = m_EmitterList.m_Active.Begin(); pIterator != m_EmitterList.m_Active.End(); )
	{
		C_Emitter* pEmitter(&(*pIterator));
		++pIterator;
	}
}

void C_Manager::Update(const F32 fFrameTime, const F32 fFrameRate)
{
	for(EmitterList::Iterator pIterator = m_EmitterList.m_Active.Begin(); pIterator != m_EmitterList.m_Active.End(); )
	{
		C_Emitter* pEmitter(&(*pIterator));
		++pIterator;

		pEmitter->Update(fFrameTime, fFrameRate);
	}
}

C_Emitter* C_Manager::NewEmitter()
{
	C_Emitter* pEmitter = NULL;
	if(!(pEmitter = m_EmitterList.Allocate()))
		LOGWARN(g_Log, "StripManager: EmitterCache empty!\n");
	else
		pEmitter->Reset();

	return pEmitter;
}

void C_Manager::FreeEmitter(C_Emitter* pEmitter)
{
	pEmitter->m_iSalt += 1;
	m_EmitterList.Free(pEmitter);
}


		


	


















