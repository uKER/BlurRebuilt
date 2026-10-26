#ifndef _SYSTEM_PS3_SHARED_H_
#define _SYSTEM_PS3_SHARED_H_

namespace Audio
{
	class Instance;

	enum AudioSpuTaskId
	{
		eSetup = 0,
//		eObjectInit,
		eObjectUpdate,
		eObjectDestroy,
		eFrameUpdateFinishedMarker
	};

	struct AudioSpuTaskParams
	{
		AudioSpuTaskId eTaskId;
		void* pSrcAddress;
		void* pRetAddress;
		U32 u32Size;
	};

	struct AudioSpuObjectUpdateInfo
	{
		Object* pPpuObject;
		void* pPpuDefinitionData;
		U32 u32DefinitionDataSize;
		void* pPpuObjectComponents;
		U32 u32ObjectComponentsSize;
//		Instance* pPpuInstance;
		void* pPpuInstanceContext;
		U32 u32InstanceContextSize;
		void* pPpuInstanceCoreData;
		U32 u32InstanceCoreDataSize;
		void* pPpuInstanceBufferData;
		U32 u32InstanceBufferDataSize;
		void* pPpuSystemState;
//		void* pPpuDynamicBuffers;
		U32 u32DynamicBufferSize;
//		void* pPpuFileStreamArray;
		U32 u32ScratchDmaBufferSize;
//		void* pPpuAdpcmBufferBitfield;
//		U8* pAdpcmBufferBasePointer;
//		void* pPpuInstanceBitmaskArray;
//		bool bUpdate;
	};

	//struct ComponentInfo
	//{
	//	U32 u32Flags;
	//	U32 u32NumMessages;
	//};

	void CopyInfoFromSpuStruct(ISystem::Info* pInfo);
}

#endif	//	_SYSTEM_PS3_SHARED_H_
