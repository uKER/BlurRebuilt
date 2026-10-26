#ifndef AFTK_RENDER_QUEUE_INCLUDE
#define AFTK_RENDER_QUEUE_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	class C_RenderTask
	{
	public:
		C_RenderTask();
		C_RenderTask(U8 lod, I_RenderMeshNode* pRenderMeshNode, U32 cluster);
		~C_RenderTask();

		bool operator<(const C_RenderTask& rhs);

		U8 GetLodLevel() const;
		void SetLodLevel(U8 lodlevel);

		I_RenderMeshNode* GetRenderMeshNode() const;
		void SetRenderMeshNode(I_RenderMeshNode* pRenderMeshNode);

		C_RenderMeshCluster* GetRenderMeshClusterPtr() const;
		U32 GetRenderMeshClusterIndex() const;
		void SetRenderMeshClusterIndex(U32 clusterIndex);

	private:
		U32 m_RenderMeshClusterIndex;
		I_RenderMeshNode* m_pRenderMeshNode;
		U8 m_LodLevel;

		friend struct C_RenderTaskFrontToBackSort;
		friend struct C_RenderTaskBackToFrontSort;
	};


	struct C_RenderTaskFrontToBackSort
	{
		inline bool operator()(const C_RenderTask& lhs, const C_RenderTask& rhs);
	};


	struct C_RenderTaskBackToFrontSort
	{
		inline bool operator()(const C_RenderTask& lhs, const C_RenderTask& rhs);
	};

	class C_RenderQueue
	{
	public:
		C_RenderQueue();
		~C_RenderQueue();

		void PushTask(const C_RenderTask& task);

		void Clear();
		U32 GetSize() const;

		const C_RenderTask& GetTask(U32 i);

		const C_RenderTask& GetTask(U32 i) const;

		void Sort();

		void SortReverse();

	private:
		Ut::UtVector< Ut::DynamicVector<C_RenderTask> > m_RenderTasks;
	};

	inline const C_RenderTask& C_RenderQueue::GetTask(U32 i) const
	{
		return m_RenderTasks[i];
	}

	inline const C_RenderTask& C_RenderQueue::GetTask(U32 i)
	{
		return m_RenderTasks[i];
	}

	#include "RenderQueue.inl"
}
#endif