#ifndef RD_SCENE_H
#define RD_SCENE_H
#ifdef _MSC_VER
#pragma once
#endif

#include "node.h"
#include "resourceblock.h"
#include "material.h"

namespace RenderingData
{
	enum PlatformType
	{
		PLATFORM_TYPE_UNKNOWN = -1,
		PLATFORM_TYPE_360,
		PLATFORM_TYPE_PS3,
		PLATFORM_TYPE_PC
	};

	bool GetEndianSwap(PlatformType platformType);

	class Scene
	{
	public:
		Scene();
		~Scene();

		const Btl::String& GetName() const;

		virtual const Node* GetNodePtr(const ResourceIndex& i) const = 0;
		virtual Node* GetNodePtr(const ResourceIndex& i) = 0;

		FRIEND_SERIALISE(Scene);

		Scene(const Scene& scene);
		Scene& operator = (const Scene& scene);

		void RegisterNode(Node* node);
		void DeregisterNode(Node* node);
		bool IsActive(const Node* node) const;

	protected:
		
		struct NodeData
		{
			Node* pNode;
			NodeType cachedType;
			bool operator == (const Node* p) const {return pNode == p;} 
		};

		Btl::Vector<NodeData> m_activeNodes;
		Btl::String m_name;
		RootNode m_rootNode;
	};

	#include "noninterface/scene.inl"
}

#endif
