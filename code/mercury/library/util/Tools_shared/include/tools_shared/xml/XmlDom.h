#pragma once

namespace Tools
{
	class XmlBaseNode
	{
	public:
		enum E_NodeType
		{
			E_NodeType_BaseNode	= 0,
			E_NodeType_Element,
		};

		typedef Ut::UtVector<Ut::DynamicVector<XmlBaseNode*> > Array;

		XmlBaseNode() : m_type(E_NodeType_BaseNode), m_refCount(1) {}

		XmlBaseNode(E_NodeType type) : m_type(type), m_refCount(1) {}

		virtual ~XmlBaseNode();

		int AddRef();

		void DecRef();

		void AddChild(XmlBaseNode* pChild);

		const Array& GetChildren() const;

		const E_NodeType GetType() const;

		Util::C_String& GetName();
		
		const Util::C_String& GetName() const;

	protected:
		int			m_refCount;
		E_NodeType	m_type;

		Util::C_String m_name;
		Array		m_children;
	};

	class XmlElement : public XmlBaseNode
	{
	public:
		XmlElement();

		Sys::C_XmlAttributeList attributes;
	};

	// callbacks for reading the ini file using XMLParser.cpp
	class XmlContent : public Sys::C_XmlContent
	{
	public:
		virtual ~XmlContent();

		bool StartDocument();

		bool EndDocument();

		bool StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes);

		bool EndElement(const Util::C_String& name);

		bool Read(const char* filename);

	protected:
		Ut::UtList<XmlBaseNode*>	m_stack;
	};

	class XmlDom : public XmlContent
	{
	public:
		XmlDom();

		bool Read(const char* filename);

		XmlBaseNode* GetRootNodePtr();

		const XmlBaseNode* GetRootNodePtr() const;

	private:
		XmlBaseNode*				m_pRootNode;
	};

	class XmlUtility
	{
	public:
		//Recursive node search.
		static XmlBaseNode* FindXmlNodeByName(XmlBaseNode* pNode, const Util::C_String& name);

		static const XmlBaseNode* FindXmlNodeByName(const XmlBaseNode* pNode, const Util::C_String& name);

		static XmlBaseNode* FindXmlNodeByNameAttribute(
			XmlBaseNode* pNode, 
			const Util::C_String& name);

		static XmlBaseNode* FindXmlNodeByTypeNameAttribute(
			XmlBaseNode* pNode, 
			const Util::C_String& name);

		static XmlBaseNode* FindXmlNodeByTypeNameAttributeSubStr(
			XmlBaseNode* pNode, 
			const Util::C_String& name);
	};
}
