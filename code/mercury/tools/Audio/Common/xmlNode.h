#ifndef _XMLNODE_H_
#define _XMLNODE_H_

#include <string>
#include "common\linkedList.h"
//#include "vector.h"

namespace xml
{
	enum NodeType
	{
		eNodeType_Tag =0,
		eNodeType_Text,
		eNodeType_Comment,
	};

//------------------------------------------------------------------------------
//  forward class definitions
//------------------------------------------------------------------------------
	class TagNode;
	class Parser;

//------------------------------------------------------------------------------
//  Node
//------------------------------------------------------------------------------
	class Node
	{
	public:
		Node(TagNode* pParent, const char* pchFilename = NULL) { m_pParent = pParent; m_pchData = NULL; m_iLine = -1; m_strFilename = pchFilename; }
		virtual ~Node() { Release(); }

		NodeType GetNodeType() { return m_NodeType; }
		void SetNodeType(NodeType type) { m_NodeType=type; }

		TagNode* GetParent() { return m_pParent; }

        void SetParent(TagNode* pParent) { m_pParent = pParent; }

		virtual void SetData(const char* pchData);

		void SetLine(int iLine) { m_iLine = iLine; }

		const char* GetFilename() { return m_strFilename.data(); }

		int GetLine() { return m_iLine; }

        virtual void Release();

	private:
		NodeType m_NodeType;

        std::string m_strFilename;

		TagNode* m_pParent;

		char* m_pchData;

		int m_iLine;
	};

//------------------------------------------------------------------------------
//  TagNode
//------------------------------------------------------------------------------
	class TagNode : public Node
	{
	public:
		TagNode(TagNode* pParent, const char* pchFilename = NULL) : Node(pParent, pchFilename) { m_pchType = NULL; m_pchAttributes = NULL; }
		virtual ~TagNode() { Release(); }

		void SetType(const char* pchType);
		void SetAttributes(const char* pchAttributes);

		const char* GetType() { return m_pchType; }
		const char* GetAttributes() { return m_pchAttributes; }
		
		const char* GetAttributeValue(const char* pchKey, std::string& strValue);
		bool GetAttributeValueInt(const char* pchKey, int& iValue);
		bool GetAttributeValueFloat(const char* pchKey, float& fValue);

		void AddChild(Node*);

		Audio::List<Node*>& GetChildren() { return m_lChildren; }

        void Release();

	private:
		char* m_pchType;

		char* m_pchAttributes;

		Audio::List<Node*> m_lChildren;
	};

//------------------------------------------------------------------------------
//  TextNode
//------------------------------------------------------------------------------
	class TextNode : public Node
	{
	public:
		TextNode(TagNode* pParent, const char* pchFilename = NULL) : Node(pParent, pchFilename) { m_pchText = NULL; }
		virtual ~TextNode() { Release(); }

		void SetText(const char* pchText);

		const char* GetText() { return m_pchText; }

        void Release();

	private:
		char* m_pchText;
	};

//------------------------------------------------------------------------------
//  CommentNode
//------------------------------------------------------------------------------
	class CommentNode : public Node
	{
	public:
		CommentNode(TagNode* pParent, const char* pchFilename = NULL) : Node(pParent, pchFilename) { m_pchComment = NULL; }
		virtual ~CommentNode() { Release(); }

		void SetComment(const char* pchComment);

		const char* GetComment() { return m_pchComment; }

        void Release();

	private:
		char* m_pchComment;
	};
}

#endif // _XMLNODE_H_
