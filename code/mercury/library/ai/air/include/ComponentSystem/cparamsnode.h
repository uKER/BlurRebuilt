#ifndef __CPARAMETERNODE_H
#define __CPARAMETERNODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <ComponentSystem/Globals.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif
namespace airacingtech
{
	class CParameterNode
	{
	public:

		CParameterNode():/*mType(0),mName(0),m_ObjectName(0),*/mData(0) {m_childNodes.Destroy();}

		CHash	GetName() const {return mName; }
		CHash   GetObjectName() const {return m_ObjectName;}
		CHash	GetParameterType() const {return mType;}
		void	*GetData() const {return mData;}
		void	SetData(void *pdata) {mData = pdata;}
		void	SetType(CHash t) {mType = t;}
		void	SetName(CHash t) {mName = t;}
		void	SetObjectName(CHash t) {m_ObjectName = t;}
		
		inline EComponentTypeId	GetComponentType() const
		{
			return GlobalsComponent::GetObjectManager().GetComponentTypeFromHash(mName);
		}

		void AddChildNode(CParameterNode node)
		{
			assert(GetParameterType() == CHash("OBJECT")); // do not add a component to a component
			m_childNodes.PushBack(node);
		}



		inline int GetChildNodeSize() {return (int)m_childNodes.Size();}

		inline CParameterNode *GetNode(int index)

		{
			assert(index >=0 && index < GetChildNodeSize());
			
			return &m_childNodes[index];
		}


	private:

		CHash			mType;
		CHash			mName;
		CHash			m_ObjectName;
		void			*mData; 

	
		Btl::Vector<CParameterNode> m_childNodes;
		//std::vector<CParameterNode>		m_childNodes;

	};
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

#endif //__CPARAMETERNODE_H
