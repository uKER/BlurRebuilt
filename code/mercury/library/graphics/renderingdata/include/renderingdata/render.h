#ifndef RD_RENDER_H
#define RD_RENDER_H
#ifdef _MSC_VER
#pragma once
#endif

#include "node.h"
#include "renderenums.h"
	
namespace RenderingData
{
	// Base type for rendering.
	class RenderObject
	{
	public:
		RenderObject() {}
		virtual ~RenderObject() {}
		virtual RenderObjectType GetObjectType() const;
		virtual RenderObject* Clone() const;

		VIRTUAL_SERIALISATION_KEY_DECL
	};

	// Node containing a renderable object.
	class RenderNode : public CullNode
	{
	public:
		RenderNode() {}
		RenderNode(const RenderNode& renderNode); 
		RenderNode& operator = (const RenderNode& renderNode);

		virtual NodeType GetNodeType() const;
		virtual bool IsNode(NodeType t) const;
		virtual RenderNode* Clone() const;

		void SetRenderObject(Btl::AutoPointer<RenderObject> p);
		const RenderObject* GetRenderObjectPtr() const;
		RenderObject* GetRenderObjectPtr();

		// If p is a RenderNode, return a pointer to it, otherwise return a null pointer.
		static const RenderNode* GetPointer(const Node* p);
		static RenderNode* GetPointer(Node* p);

		FRIEND_SERIALISE(RenderNode)
		VIRTUAL_SERIALISATION_KEY_DECL

	private:
		Btl::AutoPointer<RenderObject> m_object;
	};

	#include "noninterface/render.inl"
}

#endif
