#ifndef XML_DOM_BASE_NODE_H
#define XML_DOM_BASE_NODE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class C_XmlDomBaseNode
	{
	public:
		typedef Ut::UtVector<Ut::DynamicVector<C_XmlDomBaseNode*> > Array;

		C_XmlDomBaseNode(const C_XmlDomBaseNode& node);

		C_XmlDomBaseNode() : m_type(E_XmlDomNodeType_Base), m_refCount(1), m_parent(0) {}

		C_XmlDomBaseNode(E_XmlDomNodeType type) : m_type(type), m_refCount(1) {}

		virtual ~C_XmlDomBaseNode();

		int AddRef();

		void DecRef();

		void AddChild(C_XmlDomBaseNode* pChild);

		const Array& GetChildren() const;

		const C_XmlDomBaseNode* GetParent() const;

		const E_XmlDomNodeType GetType() const;

		Util::C_String& GetName();
		
		const Util::C_String& GetName() const;

	protected:
		int					m_refCount;
		E_XmlDomNodeType	m_type;

		Util::C_String m_name;
		C_XmlDomBaseNode* m_parent;
		Array		m_children;
	};
}

#endif
