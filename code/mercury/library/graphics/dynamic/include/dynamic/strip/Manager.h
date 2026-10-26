#ifndef RENDER_DYNAMICEFFECT_STRIP_MANAGER_H
#define RENDER_DYNAMICEFFECT_STRIP_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Render/Utils/ListPool.h"

// dynamic effect includes
#include "Dynamic/ManagerInterface.h"
#include "Dynamic/Defines.h"
#include "Dynamic/BlockManager.h"
#include "Dynamic/Strip/Block.h"

namespace Render
{
	class C_RenderState;
}

namespace DynamicEffect
{
struct RenderStates;
class I_DynamicEffect;




namespace Strip
{
class C_Emitter;
typedef Ut::UtSet<C_Emitter*>		Emitters;

// particle manager class manages shaders and particle storage for all particle effects
class C_Manager : public I_DynamicEffectManager
{
public:
	C_Manager(C_Renderer* pRenderer);
	~C_Manager();

	void							ReserveEmitters(const U32 nEmitters);

	bool							Create();
	bool							CreateVolatile();
	bool							Initialise();
	void							Destroy();
	void							DestroyVolatile();
	void							Shutdown();

	void							Reset();

	// update
	void							Update(const F32 fFrameTime, const F32 fFrameRate);

	void							DisplayDebugGraphics() const;
	MathsStorage::Int2				DisplayDebugInfo(MathsStorage::Int2 position) const;

	const Display::C_PixelShader&	GetPixelShader() const { return m_PixelShader; }
	const Display::C_VertexShader&	GetVertexShader() const { return m_VertexShader; }

	friend class I_DynamicEffect;
	friend class C_Emitter;

	// virtuals
	void							CleanNode(const C_Node* pNode);
	void							CleanEffect(const I_DynamicEffect* pEffect);
	
	C_Emitter*						NewEmitter();
	void							FreeEmitter(C_Emitter* pEmitter);

protected:
	// manage all objects in the particle system
	void							AddEmitter(C_Emitter* pEmitter);
	void							RemoveEmitter(C_Emitter* pEmitter);

	Emitters						m_Emitters;

	MathsStorage::Int2				DisplayBlockManagerDebugInfo(MathsStorage::Int2 position, const BlockManagerBase* pBlockManager) const;

	typedef Ut::ListPool<C_Emitter>	EmitterList;
	EmitterList						m_EmitterList;

	/*template<class T>
	friend T* NewBlock(C_Manager* pManager);

	template<class T>
	friend void FreeBlock(C_Manager* pManager, T* pBlock);

	template<class T>
	friend U32 GetBlockSize(C_Manager* pManager);*/

	friend ElementBlock* NewBlock(C_Manager* pManager, const U32 iElementSize);

	//template<class T>
	friend void FreeBlock(C_Manager* pManager, ElementBlock* pBlock, const U32 iElementSize);

	//template<class T>
	friend U32 GetBlockSize(C_Manager* pManager, const U32 iElementSize);

	VectorBlockManager<ElementBlock >		m_BlockManager8;
	VectorBlockManager<ElementBlock >		m_BlockManager16;
	VectorBlockManager<ElementBlock >		m_BlockManager32;
	VectorBlockManager<ElementBlock >		m_BlockManager48;
	VectorBlockManager<ElementBlock >		m_BlockManager64;
	VectorBlockManager<ElementBlock >		m_BlockManager80;
	
	U32								m_nEmitters;

	Display::C_PixelShader			m_PixelShader;
	Display::C_VertexShader			m_VertexShader;
	const Render::C_RenderState*	m_pRenderState;
};

__forceinline ElementBlock* NewBlock(C_Manager* pManager, const U32 iElementSize)
{
	switch(iElementSize)
	{
		case 8:
			return pManager->m_BlockManager8.NewBlock();
		case 16:
			return pManager->m_BlockManager16.NewBlock();
		case 32:
			return pManager->m_BlockManager32.NewBlock();
		case 48:
			return pManager->m_BlockManager48.NewBlock();
		case 64:
			return pManager->m_BlockManager64.NewBlock();
		case 80:
			return pManager->m_BlockManager80.NewBlock();
		default:
			return NULL;
	}
}

__forceinline void FreeBlock(C_Manager* pManager, ElementBlock* pBlock, const U32 iElementSize)
{
	switch(iElementSize)
	{
		case 8:
			pManager->m_BlockManager8.FreeBlock(pBlock);
			break;
		case 16:
			pManager->m_BlockManager16.FreeBlock(pBlock);
			break;
		case 32:
			pManager->m_BlockManager32.FreeBlock(pBlock);
			break;
		case 48:
			pManager->m_BlockManager48.FreeBlock(pBlock);
			break;
		case 64:
			pManager->m_BlockManager64.FreeBlock(pBlock);
			break;
		case 80:
			pManager->m_BlockManager80.FreeBlock(pBlock);
			break;
		default:
			break;
	}
}

__forceinline U32 GetBlockSize(C_Manager* pManager, const U32 iElementSize)
{
	switch(iElementSize)
	{
		case 8:
			return pManager->m_BlockManager8.GetBlockSize();
		case 16:
			return pManager->m_BlockManager16.GetBlockSize();
		case 32:
			return pManager->m_BlockManager32.GetBlockSize();
		case 48:
			return pManager->m_BlockManager48.GetBlockSize();
		case 64:
			return pManager->m_BlockManager64.GetBlockSize();
		case 80:
			return pManager->m_BlockManager80.GetBlockSize();
		default:
			return 0;
	}
}

} // Strip
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_RENDERER_H
