#ifndef _COMPONENT_CONTAINER_H_
#define _COMPONENT_CONTAINER_H_

namespace Audio
{

////////////////////////////////////////////////////////////////////////////////
//	ComponentContainer
////////////////////////////////////////////////////////////////////////////////
	class ComponentContainer
	{

	public:
		ComponentContainer();
		~ComponentContainer() { Clear(); }

		inline void		Reallocate(U32 u32NumComponents) { m_u32NumComponents = u32NumComponents; }
		inline U32		Size() { return m_u32NumComponents; }
		inline void		SetDataPointer(void* pData) { m_pComponentIndex = pData; }
		inline void		GetNumComponentsFromIndex() { m_u32NumComponents = *((U32*)m_pComponentIndex); }
		inline void		UpdateIndexWithNumComponents() { *((U32*)m_pComponentIndex) = m_u32NumComponents; }
		inline U32		GetIndexArraySize() { return (m_u32NumComponents + 1) * sizeof(U32); }
		inline void*	ComponentData() { return (void*)&(((U32*)m_pComponentIndex)[m_u32NumComponents + 1]); }
		inline void*	GetBasePointer() { return m_pComponentIndex; }
		U32				GetType(U32 u32Index);
		Component*		operator [] (U32 u32Index);
		void			PushBack(Component* pCom);
		void			Clear();

	private:
		U32				m_u32NumComponents;
		void*			m_pComponentIndex;
		U32				m_u32NextCellToPopulate;
	};

}

#endif	//	_COMPONENT_CONTAINER_H_
