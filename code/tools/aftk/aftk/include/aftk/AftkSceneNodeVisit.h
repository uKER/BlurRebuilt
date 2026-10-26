#ifndef AFTK_SCENE_NODE_VISITOR_H
#define AFTK_SCENE_NODE_VISITOR_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_SceneNodeVisit
	{
	public:
		template<typename AftkSceneOrNode, typename Function>
		static C_Status Traverse(
			AftkSceneOrNode* pSceneOrNode, 
			Function function,
			I_Progress* pProgress = 0)
		{
			C_NullMemoryManager mgr;

			C_Status bResult = Traverse(
				pSceneOrNode,
				function,
				mgr,
				pProgress);

			return bResult;
		}

		template<typename AftkSceneOrNode, typename Function, typename MemoryManager>
		static C_Status Traverse(
			AftkSceneOrNode* pSceneOrNode, 
			Function function,
			MemoryManager& memoryManager,
			I_Progress* pProgress = 0)
		{
			C_Status status;

			U32 nodesToProcess = 0;
			if (pProgress)
			{
				for (Aftk::C_SceneNodeIterator it(pSceneOrNode); it; ++it)
					if (function.IsInteresting(it))
						nodesToProcess++;					
			}

			U32 nodesProcessed = 0;
			for (Aftk::C_SceneNodeIterator it(pSceneOrNode); it; ++it)
			{
				if (!function.IsInteresting(it))
					continue;
				//Display the name of the node we're about to visit.
				if (pProgress)
					pProgress->OnPrintProgress((*it).GetName());

				//Visit.
				status = function.Visit(&(*it), memoryManager);
				if (!status) return status;

				//Account for the fact we've done some work.
				nodesProcessed++;
				F32 progress = ((F32)nodesProcessed / (F32)nodesToProcess) * 100.0f;

				//Update progress and check for cancellation.
				if (pProgress && pProgress->OnProgress(progress))
					return C_Status::E_Status_Cancelled;
			}

			return C_Status::E_Status_Success;
		}
	};
}

#endif
