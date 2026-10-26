#include "StdAfx.h"
#include "Display2/Debug/GCMDebug.h"
#include "Display2/Context/GCMContext.h"
#include <Btl/Containers/Vector.h>

namespace Display2
{

void ToBinary(U32 method, const bool spacing, const Util::C_String name)
{
	/*for(U32 i=0; i<=31; i++)
	{
		if(spacing && (i == METHODOFFSET_END+1 || i == METHODCOUNT_START || i == METHODCOUNT_END+1))
			CORE_PRINT("    ");
		CORE_PRINT("%2i ", i);
	}
	CORE_PRINT("\n");*/
	for(U32 i=0; i<=31; i++)
	{
		if(spacing && (i == METHODOFFSET_END+1 || i == METHODCOUNT_START || i == METHODCOUNT_END+1))
			CORE_PRINT("    ");
		CORE_PRINT("%2i ", (method >> i) & 1);
	}
	CORE_PRINT(" %s\n", name.Chars());
}

#if TRACK_DISPLAY2_COMMAND_BUFFERS
C_CommandBufferTracker g_CommandBufferTracker;
C_CommandBufferWalker g_CommandBufferWalker;
#endif

C_CommandBufferWalkerLogger g_CommandBufferWalkerLogger;

C_CommandBufferInstance::C_CommandBufferInstance()
{
	m_Name[0]		= '\0';
	m_pHeadAddress	= NULL;
	m_pTailAddress	= NULL;
}

C_CommandBufferTracker::C_CommandBufferTracker()
{
	m_CurInstance	= 0;
}

C_CommandBufferInstance* C_CommandBufferTracker::AllocInstance(char* name)
{
	m_Mutex.Wait();

	BIZ_ASSERT(m_CurInstance < m_MaxInstances);

	C_CommandBufferInstance* pInstance = &m_Instances[m_CurInstance];

	strcpy_s(pInstance->m_Name, sizeof(pInstance->m_Name), name);

	m_CurInstance = (m_CurInstance + 1) % m_MaxInstances;

	m_Mutex.Release();

	return pInstance;
}

C_CommandBufferInstance* C_CommandBufferTracker::FindInstance(const U32 offset)
{
	for(U32 i=0; i<m_MaxInstances; i++)
	{
		C_CommandBufferInstance* pInstance = &m_Instances[i];

		S32 result;

		U32 headOffset;
		result = cellGcmAddressToOffset(pInstance->m_pHeadAddress, &headOffset);
		BIZ_ASSERT(result == CELL_OK);
		
		U32 tailOffset;
		result = cellGcmAddressToOffset(pInstance->m_pTailAddress, &tailOffset);
		BIZ_ASSERT(result == CELL_OK);

		CORE_PRINT("%2i %.8x %.8x %.8x\n", i, offset, headOffset, tailOffset);

		if(offset >= headOffset && offset <= tailOffset)
		{
			CORE_PRINT("Found %.8x\n", offset);

			return pInstance;
		}
	}

	return NULL;
}

void C_CommandBufferTracker::Dump()
{
	/*C_CommandBufferWalkerInput input;
	input.m_OutputToLog = true;
	
	C_CommandBufferWalkerOutput output;
	
	for(U32 i=0; i<m_MaxInstances; i++)
	{
		const C_CommandBufferInstance& instance = m_Instances[i];

		WalkCommandBuffer((U32*)instance.m_pHeadAddress, (U32*)instance.m_pTailAddress, input, output);
	}*/

	for(U32 i=0; i<m_MaxInstances; i++)
	{
		const C_CommandBufferInstance& instance = m_Instances[i];
		
		U32 headOffset;
		cellGcmAddressToOffset(instance.m_pHeadAddress, &headOffset);
		
		U32 tailOffset;
		cellGcmAddressToOffset(instance.m_pTailAddress, &tailOffset);

		CORE_PRINT("CmdBuffer : %2i : %.8x %.8x (%.8i %.8i) %s %s\n", i,
			headOffset,
			tailOffset,
			instance.m_pHeadAddress,
			instance.m_pTailAddress,
			i == m_CurInstance ? "c" : " ",
			instance.m_Name);
	}
}

bool GetMethodNameFromAddress(const U32 methodAddress, Util::C_String& methodName);

C_CommandBufferWalkerInput::C_CommandBufferWalkerInput()
{
	m_OutputToLog						= false;
	m_OutputToFile						= false;
	m_MaxSizeInBytes					= UINT_MAX;
}

C_CommandBufferWalkerOutput::C_CommandBufferWalkerOutput()
{
	m_SizeInBytes						= 0;
	m_pLastJumpAddress					= NULL;
	m_pLastCallAddress					= NULL;
	m_pLastReturnAddress				= NULL;
	m_pLastExecutedAddress				= NULL;
	m_pLastExecutedBeforeJumpAddress	= NULL;
}

C_CommandBufferWalkerOutput::~C_CommandBufferWalkerOutput()
{
	m_JumpCallstack.Clear();
}

C_CommandBufferWalkerLogger::C_CommandBufferWalkerLogger()
{
	m_OutputToLog			= false;
	m_OutputToFile			= false;
	m_OutputBufferSize		= 1024*1024*5;
	m_OutputBufferMaxSize	= 0;
	m_pOutputBuffer			= NULL;
}

C_CommandBufferWalkerLogger::~C_CommandBufferWalkerLogger()
{
	SAFE_DELETE_ARY(m_pOutputBuffer);
}

void C_CommandBufferWalkerLogger::Init(const U32 outputBufferMaxSize)
{
	SAFE_DELETE_ARY(m_pOutputBuffer);
	
	m_OutputBufferSize		= 0;
	m_OutputBufferMaxSize	= outputBufferMaxSize;

	if(m_OutputBufferMaxSize > 0)
		m_pOutputBuffer = new char[m_OutputBufferMaxSize];
}

void C_CommandBufferWalkerLogger::Flush()
{
	if(m_OutputToFile)
	{
		FILE* pOutputFile = NULL;

		static bool firstTime = true;
		
		if(firstTime)
		{
			pOutputFile = fopen("/app_home/walkcommandbuffer.txt", "w");

			firstTime = false;
		}
		else
		{
			pOutputFile = fopen("/app_home/walkcommandbuffer.txt", "a");
		}

		if(pOutputFile)
		{
			fwrite(m_pOutputBuffer, sizeof(char), m_OutputBufferSize, pOutputFile);

			fclose(pOutputFile);
		}

		m_OutputBufferSize = 0;
	}
}

void C_CommandBufferWalkerLogger::Output(const char* str)
{
	if(m_OutputToLog)
	{
		LOGINFO(g_Log, str);
	}

#if 0
	if(m_OutputToFile)
	{
		U32 strLen = strlen(str);

		BIZ_ASSERT(strLen <= m_OutputBufferMaxSize);

		if(m_OutputBufferSize + strLen > m_OutputBufferMaxSize)
		{
			U32 outputBufferUsedSize = m_OutputBufferMaxSize - m_OutputBufferSize;
			U32 outputBufferOverSize = m_OutputBufferSize + strLen - m_OutputBufferMaxSize;

			memcpy(&m_pOutputBuffer[m_OutputBufferSize], str, outputBufferUsedSize);

			m_OutputBufferSize += outputBufferUsedSize;

			Flush();

			memcpy(&m_pOutputBuffer[m_OutputBufferSize], str, outputBufferOverSize);

			m_OutputBufferSize += outputBufferOverSize;
		}
		else
		{
			memcpy(&m_pOutputBuffer[m_OutputBufferSize], str, strLen+1);

			m_OutputBufferSize += strLen+1;
		}
	}
#endif
}

C_CommandBufferWalker::C_CommandBufferWalker()
{
	m_pCurrHeadAddress = NULL;
	m_pCurrTailAddress = NULL;
	m_pPrevHeadAddress = NULL;
	m_pPrevTailAddress = NULL;
}

void C_CommandBufferWalker::Walk()
{
	C_CommandBufferWalkerInput input;
	input.m_OutputToLog = true;
	
	C_CommandBufferWalkerOutput output;
	
	WalkCommandBuffer((U32*)m_pCurrHeadAddress, (U32*)m_pCurrTailAddress, input, output);
}

bool WalkCommandBuffer(const U32 headOffset, const U32 tailOffset, const bool outputToLog, const bool outputToFile, const U32 maxSizeInBytes)
{
	return WalkCommandBufferOffsets(headOffset, tailOffset, outputToLog, outputToFile, maxSizeInBytes);
}

bool WalkCommandBufferOffsets(const U32 headOffset, const U32 tailOffset, const bool outputToLog, const bool outputToFile, const U32 maxSizeInBytes)
{
	S32 result;

	void* pHeadAddress = NULL;
	result = cellGcmIoOffsetToAddress(headOffset, &pHeadAddress);
	BIZ_ASSERT(result == CELL_OK);
	
	void* pTailAddress = NULL;
	result = cellGcmIoOffsetToAddress(tailOffset, &pTailAddress);
	BIZ_ASSERT(result == CELL_OK);
	
	C_CommandBufferWalkerInput input;
	input.m_OutputToLog			= outputToLog;
	input.m_OutputToFile		= outputToFile;
	input.m_MaxSizeInBytes		= maxSizeInBytes;
	C_CommandBufferWalkerOutput output;
	
	return WalkCommandBuffer((U32*)pHeadAddress, (U32*)pTailAddress, input, output);
}

bool WalkCommandBufferAddress(const U8* pHeadAddress, const U8* pTailAddress, const bool outputToLog, const bool outputToFile, const U32 maxSizeInBytes)
{
	C_CommandBufferWalkerInput input;
	input.m_OutputToLog			= outputToLog;
	input.m_OutputToFile		= outputToFile;
	input.m_MaxSizeInBytes		= maxSizeInBytes;
	C_CommandBufferWalkerOutput output;
	
	return WalkCommandBuffer((U32*)pHeadAddress, (U32*)pTailAddress, input, output);
}

bool WalkCommandBuffer(U32* pHeadAddress, U32* pTailAddress, const C_CommandBufferWalkerInput& input, C_CommandBufferWalkerOutput& output)
{
	g_CommandBufferWalkerLogger.m_OutputToLog = input.m_OutputToLog;
	g_CommandBufferWalkerLogger.m_OutputToFile = input.m_OutputToFile;

	char str1[4096];
	char str2[1024];

	sprintf_s(str1, sizeof(str1), "\nWalking command buffer from %.8x to %.8x\n", AddressToOffset(pHeadAddress), AddressToOffset(pTailAddress));
	g_CommandBufferWalkerLogger.Output(str1);
	
	output.m_SizeInBytes = 0;
	output.m_JumpCallstack.Clear();

	U32* pReturnCallstack = NULL;
	
	U32* pCurrAddress = pHeadAddress;

	U32* pPrevAddress = pCurrAddress;

	bool done = false;
	
	while(!done)
	{
		output.m_pLastExecutedAddress = pPrevAddress = pCurrAddress;

		if(pCurrAddress == pTailAddress)
			done = true;

		U32 methodCommand = *pCurrAddress;
		pCurrAddress++;
		output.m_SizeInBytes += sizeof(U32);

		if(output.m_SizeInBytes >= input.m_MaxSizeInBytes)
		{
			sprintf_s(str1, sizeof(str1), "Possible infinte loop when walking command buffer\n");
			g_CommandBufferWalkerLogger.Output(str1);
			//g_CommandBufferWalkerLogger.Flush();
			BIZ_ASSERT_FAIL(str1);

			return false;
		}

		sprintf_s(str1, sizeof(str1), "%.8x : %5i : %.8x : ", AddressToOffset(pPrevAddress), output.m_SizeInBytes, methodCommand);

		if(methodCommand & CELL_GCM_METHOD_FLAG_JUMP)
		{
			U32 jumpOffset = methodCommand & ~CELL_GCM_METHOD_FLAG_JUMP;
			U32* pJumpAddress = (U32*)OffsetToAddress(E_Location_Main, jumpOffset);

			BIZ_ASSERT((jumpOffset & 3) == 0);
			BIZ_ASSERT(((U32)pJumpAddress & 3) == 0);

			sprintf_s(str1, sizeof(str1), "\n%sCELL_GCM_METHOD_FLAG_JUMP : %.8x\n\n", str1, AddressToOffset(pJumpAddress));
			g_CommandBufferWalkerLogger.Output(str1);
			
			if(pJumpAddress == output.m_pLastExecutedAddress)
			{
				sprintf_s(str1, sizeof(str1), "Jump to self encountered so lets bail out...\n");
				g_CommandBufferWalkerLogger.Output(str1);

				return true;
			}

			output.m_pLastExecutedBeforeJumpAddress = output.m_pLastExecutedAddress;

			pCurrAddress = output.m_pLastJumpAddress = pJumpAddress;

			output.m_JumpCallstack.PushBack(pCurrAddress);

			if(pCurrAddress == pTailAddress)
				done = true;
		}
		else
		if(methodCommand & CELL_GCM_METHOD_FLAG_CALL)
		{
			U32 callOffset = methodCommand & ~CELL_GCM_METHOD_FLAG_CALL;
			U32* pCallAddress = (U32*)OffsetToAddress(E_Location_Main, callOffset);

			BIZ_ASSERT((callOffset & 3) == 0);
			BIZ_ASSERT(((U32)pCallAddress & 3) == 0);

			sprintf_s(str1, sizeof(str1), "\n%sCELL_GCM_METHOD_FLAG_CALL : %.8x\n\n", str1, AddressToOffset(pCallAddress));
			g_CommandBufferWalkerLogger.Output(str1);
			
			BIZ_ASSERT(!pReturnCallstack);
			pReturnCallstack = pCurrAddress;

			pCurrAddress = output.m_pLastCallAddress = pCallAddress;
		}
		else
		if(methodCommand & CELL_GCM_METHOD_FLAG_RETURN)
		{
			sprintf_s(str1, sizeof(str1), "\n%sCELL_GCM_METHOD_FLAG_RETURN : %.8x\n\n", str1, AddressToOffset(pReturnCallstack));
			g_CommandBufferWalkerLogger.Output(str1);
			
			BIZ_ASSERT(pReturnCallstack);

			pCurrAddress = output.m_pLastReturnAddress = pReturnCallstack;

			pReturnCallstack = NULL;
		}
		else
		{
			U32 methodIncrement = (methodCommand & CELL_GCM_METHOD_FLAG_NON_INCREMENT) ? 0 : 4;

			U32 methodOffset = METHODEXTRACT(methodCommand, METHODOFFSET_START, METHODOFFSET_END);

			Util::C_String methodName = "";

			bool methodSupported = GetMethodNameFromAddress(methodOffset, methodName);

			U32 methodCount =  METHODEXTRACT(methodCommand, METHODCOUNT_START, METHODCOUNT_END);

			U32 methodAddress = methodOffset;

			sprintf_s(str2, sizeof(str2), "%s : %.8x : %i : ", methodName.Chars(), methodOffset, methodCount);
			
			for(U32 i=0; i<methodCount; i++)
			{
				if(pCurrAddress == pTailAddress)
					done = true;

				U32 methodData = *pCurrAddress;
				pCurrAddress++;
				output.m_SizeInBytes += sizeof(U32);

				if(output.m_SizeInBytes >= input.m_MaxSizeInBytes)
				{
					sprintf_s(str1, sizeof(str1), "Possible infinte loop when walking command buffer\n");
					g_CommandBufferWalkerLogger.Output(str1);
					//g_CommandBufferWalkerLogger.Flush();
					BIZ_ASSERT_FAIL(str1);
					
					return false;
				}

				sprintf_s(str2, sizeof(str2), "%s %i", str2, methodData);

				methodAddress += methodIncrement;
			}

			sprintf_s(str1, sizeof(str1), "%s%s\n", str1, str2);
			g_CommandBufferWalkerLogger.Output(str1);
		}
	}

	return true;
}

struct C_Method
{
	U32					m_Address;
	Util::C_String		m_Name;
};

bool GetMethodNameFromAddress(const U32 methodAddress, Util::C_String& methodName)
{
	static C_Method methods[] = 
	{
		0x00000050,"CELL_GCM_NV406E_SET_REFERENCE",
		0x00000060,"CELL_GCM_NV406E_SET_CONTEXT_DMA_SEMAPHORE",
		0x00000064,"CELL_GCM_NV406E_SEMAPHORE_OFFSET",
		0x00000068,"CELL_GCM_NV406E_SEMAPHORE_ACQUIRE",
		0x0000006c,"CELL_GCM_NV406E_SEMAPHORE_RELEASE",
		0x00000000,"CELL_GCM_NV4097_SET_OBJECT",
		0x00000100,"CELL_GCM_NV4097_NO_OPERATION",
		0x00000110,"CELL_GCM_NV4097_WAIT_FOR_IDLE",
		0x00000140,"CELL_GCM_NV4097_PM_TRIGGER",
		0x00000180,"CELL_GCM_NV4097_SET_CONTEXT_DMA_NOTIFIES",
		0x00000184,"CELL_GCM_NV4097_SET_CONTEXT_DMA_A",
		0x00000188,"CELL_GCM_NV4097_SET_CONTEXT_DMA_B",
		0x0000018c,"CELL_GCM_NV4097_SET_CONTEXT_DMA_COLOR_B",
		0x00000190,"CELL_GCM_NV4097_SET_CONTEXT_DMA_STATE",
		0x00000194,"CELL_GCM_NV4097_SET_CONTEXT_DMA_COLOR_A",
		0x00000198,"CELL_GCM_NV4097_SET_CONTEXT_DMA_ZETA",
		0x0000019c,"CELL_GCM_NV4097_SET_CONTEXT_DMA_VERTEX_A",
		0x000001a0,"CELL_GCM_NV4097_SET_CONTEXT_DMA_VERTEX_B",
		0x000001a4,"CELL_GCM_NV4097_SET_CONTEXT_DMA_SEMAPHORE",
		0x000001a8,"CELL_GCM_NV4097_SET_CONTEXT_DMA_REPORT",
		0x000001ac,"CELL_GCM_NV4097_SET_CONTEXT_DMA_CLIP_ID",
		0x000001b0,"CELL_GCM_NV4097_SET_CONTEXT_DMA_CULL_DATA",
		0x000001b4,"CELL_GCM_NV4097_SET_CONTEXT_DMA_COLOR_C",
		0x000001b8,"CELL_GCM_NV4097_SET_CONTEXT_DMA_COLOR_D",
		0x00000200,"CELL_GCM_NV4097_SET_SURFACE_CLIP_HORIZONTAL",
		0x00000204,"CELL_GCM_NV4097_SET_SURFACE_CLIP_VERTICAL",
		0x00000208,"CELL_GCM_NV4097_SET_SURFACE_FORMAT",
		0x0000020c,"CELL_GCM_NV4097_SET_SURFACE_PITCH_A",
		0x00000210,"CELL_GCM_NV4097_SET_SURFACE_COLOR_AOFFSET",
		0x00000214,"CELL_GCM_NV4097_SET_SURFACE_ZETA_OFFSET",
		0x00000218,"CELL_GCM_NV4097_SET_SURFACE_COLOR_BOFFSET",
		0x0000021c,"CELL_GCM_NV4097_SET_SURFACE_PITCH_B",
		0x00000220,"CELL_GCM_NV4097_SET_SURFACE_COLOR_TARGET",
		0x0000022c,"CELL_GCM_NV4097_SET_SURFACE_PITCH_Z",
		0x00000234,"CELL_GCM_NV4097_INVALIDATE_ZCULL",
		0x00000238,"CELL_GCM_NV4097_SET_CYLINDRICAL_WRAP",
		0x0000023c,"CELL_GCM_NV4097_SET_CYLINDRICAL_WRAP1",
		0x00000280,"CELL_GCM_NV4097_SET_SURFACE_PITCH_C",
		0x00000284,"CELL_GCM_NV4097_SET_SURFACE_PITCH_D",
		0x00000288,"CELL_GCM_NV4097_SET_SURFACE_COLOR_COFFSET",
		0x0000028c,"CELL_GCM_NV4097_SET_SURFACE_COLOR_DOFFSET",
		0x000002b8,"CELL_GCM_NV4097_SET_WINDOW_OFFSET",
		0x000002bc,"CELL_GCM_NV4097_SET_WINDOW_CLIP_TYPE",
		0x000002c0,"CELL_GCM_NV4097_SET_WINDOW_CLIP_HORIZONTAL",
		0x000002c4,"CELL_GCM_NV4097_SET_WINDOW_CLIP_VERTICAL",
		0x00000300,"CELL_GCM_NV4097_SET_DITHER_ENABLE",
		0x00000304,"CELL_GCM_NV4097_SET_ALPHA_TEST_ENABLE",
		0x00000308,"CELL_GCM_NV4097_SET_ALPHA_FUNC",
		0x0000030c,"CELL_GCM_NV4097_SET_ALPHA_REF",
		0x00000310,"CELL_GCM_NV4097_SET_BLEND_ENABLE",
		0x00000314,"CELL_GCM_NV4097_SET_BLEND_FUNC_SFACTOR",
		0x00000318,"CELL_GCM_NV4097_SET_BLEND_FUNC_DFACTOR",
		0x0000031c,"CELL_GCM_NV4097_SET_BLEND_COLOR",
		0x00000320,"CELL_GCM_NV4097_SET_BLEND_EQUATION",
		0x00000324,"CELL_GCM_NV4097_SET_COLOR_MASK",
		0x00000328,"CELL_GCM_NV4097_SET_STENCIL_TEST_ENABLE",
		0x0000032c,"CELL_GCM_NV4097_SET_STENCIL_MASK",
		0x00000330,"CELL_GCM_NV4097_SET_STENCIL_FUNC",
		0x00000334,"CELL_GCM_NV4097_SET_STENCIL_FUNC_REF",
		0x00000338,"CELL_GCM_NV4097_SET_STENCIL_FUNC_MASK",
		0x0000033c,"CELL_GCM_NV4097_SET_STENCIL_OP_FAIL",
		0x00000340,"CELL_GCM_NV4097_SET_STENCIL_OP_ZFAIL",
		0x00000344,"CELL_GCM_NV4097_SET_STENCIL_OP_ZPASS",
		0x00000348,"CELL_GCM_NV4097_SET_TWO_SIDED_STENCIL_TEST_ENABLE",
		0x0000034c,"CELL_GCM_NV4097_SET_BACK_STENCIL_MASK",
		0x00000350,"CELL_GCM_NV4097_SET_BACK_STENCIL_FUNC",
		0x00000354,"CELL_GCM_NV4097_SET_BACK_STENCIL_FUNC_REF",
		0x00000358,"CELL_GCM_NV4097_SET_BACK_STENCIL_FUNC_MASK",
		0x0000035c,"CELL_GCM_NV4097_SET_BACK_STENCIL_OP_FAIL",
		0x00000360,"CELL_GCM_NV4097_SET_BACK_STENCIL_OP_ZFAIL",
		0x00000364,"CELL_GCM_NV4097_SET_BACK_STENCIL_OP_ZPASS",
		0x00000368,"CELL_GCM_NV4097_SET_SHADE_MODE",
		0x0000036c,"CELL_GCM_NV4097_SET_BLEND_ENABLE_MRT",
		0x00000370,"CELL_GCM_NV4097_SET_COLOR_MASK_MRT",
		0x00000374,"CELL_GCM_NV4097_SET_LOGIC_OP_ENABLE",
		0x00000378,"CELL_GCM_NV4097_SET_LOGIC_OP",
		0x0000037c,"CELL_GCM_NV4097_SET_BLEND_COLOR2",
		0x00000380,"CELL_GCM_NV4097_SET_DEPTH_BOUNDS_TEST_ENABLE",
		0x00000384,"CELL_GCM_NV4097_SET_DEPTH_BOUNDS_MIN",
		0x00000388,"CELL_GCM_NV4097_SET_DEPTH_BOUNDS_MAX",
		0x00000394,"CELL_GCM_NV4097_SET_CLIP_MIN",
		0x00000398,"CELL_GCM_NV4097_SET_CLIP_MAX",
		0x000003b0,"CELL_GCM_NV4097_SET_CONTROL0",
		0x000003b8,"CELL_GCM_NV4097_SET_LINE_WIDTH",
		0x000003bc,"CELL_GCM_NV4097_SET_LINE_SMOOTH_ENABLE",
		0x000003c0,"CELL_GCM_NV4097_SET_ANISO_SPREAD       ",
		0x000008c0,"CELL_GCM_NV4097_SET_SCISSOR_HORIZONTAL",
		0x000008c4,"CELL_GCM_NV4097_SET_SCISSOR_VERTICAL",
		0x000008cc,"CELL_GCM_NV4097_SET_FOG_MODE",
		0x000008d0,"CELL_GCM_NV4097_SET_FOG_PARAMS",
		0x000008e4,"CELL_GCM_NV4097_SET_SHADER_PROGRAM",
		0x00000900,"CELL_GCM_NV4097_SET_VERTEX_TEXTURE_OFFSET",
		0x00000904,"CELL_GCM_NV4097_SET_VERTEX_TEXTURE_FORMAT",
		0x00000908,"CELL_GCM_NV4097_SET_VERTEX_TEXTURE_ADDRESS",
		0x0000090c,"CELL_GCM_NV4097_SET_VERTEX_TEXTURE_CONTROL0",
		0x00000910,"CELL_GCM_NV4097_SET_VERTEX_TEXTURE_CONTROL3",
		0x00000914,"CELL_GCM_NV4097_SET_VERTEX_TEXTURE_FILTER",
		0x00000918,"CELL_GCM_NV4097_SET_VERTEX_TEXTURE_IMAGE_RECT",
		0x0000091c,"CELL_GCM_NV4097_SET_VERTEX_TEXTURE_BORDER_COLOR",
		0x00000a00,"CELL_GCM_NV4097_SET_VIEWPORT_HORIZONTAL",
		0x00000a04,"CELL_GCM_NV4097_SET_VIEWPORT_VERTICAL",
		0x00000a0c,"CELL_GCM_NV4097_SET_POINT_CENTER_MODE",
		0x00000a1c,"CELL_GCM_NV4097_ZCULL_SYNC",
		0x00000a20,"CELL_GCM_NV4097_SET_VIEWPORT_OFFSET",
		0x00000a30,"CELL_GCM_NV4097_SET_VIEWPORT_SCALE",
		0x00000a60,"CELL_GCM_NV4097_SET_POLY_OFFSET_POINT_ENABLE",
		0x00000a64,"CELL_GCM_NV4097_SET_POLY_OFFSET_LINE_ENABLE",
		0x00000a68,"CELL_GCM_NV4097_SET_POLY_OFFSET_FILL_ENABLE",
		0x00000a6c,"CELL_GCM_NV4097_SET_DEPTH_FUNC",
		0x00000a70,"CELL_GCM_NV4097_SET_DEPTH_MASK",
		0x00000a74,"CELL_GCM_NV4097_SET_DEPTH_TEST_ENABLE",
		0x00000a78,"CELL_GCM_NV4097_SET_POLYGON_OFFSET_SCALE_FACTOR",
		0x00000a7c,"CELL_GCM_NV4097_SET_POLYGON_OFFSET_BIAS",
		0x00000a80,"CELL_GCM_NV4097_SET_VERTEX_DATA_SCALED4S_M",
		0x00000b00,"CELL_GCM_NV4097_SET_TEXTURE_CONTROL2",
		0x00000b40,"CELL_GCM_NV4097_SET_TEX_COORD_CONTROL",
		0x00000b80,"CELL_GCM_NV4097_SET_TRANSFORM_PROGRAM",
		0x00001428,"CELL_GCM_NV4097_SET_SPECULAR_ENABLE",
		0x0000142c,"CELL_GCM_NV4097_SET_TWO_SIDE_LIGHT_EN",
		0x00001438,"CELL_GCM_NV4097_CLEAR_ZCULL_SURFACE",
		0x00001450,"CELL_GCM_NV4097_SET_PERFORMANCE_PARAMS",
		0x00001454,"CELL_GCM_NV4097_SET_FLAT_SHADE_OP",
		0x0000145c,"CELL_GCM_NV4097_SET_EDGE_FLAG",
		0x00001478,"CELL_GCM_NV4097_SET_USER_CLIP_PLANE_CONTROL",
		0x0000147c,"CELL_GCM_NV4097_SET_POLYGON_STIPPLE",
		0x00001480,"CELL_GCM_NV4097_SET_POLYGON_STIPPLE_PATTERN",
		0x00001500,"CELL_GCM_NV4097_SET_VERTEX_DATA3F_M",
		0x00001680,"CELL_GCM_NV4097_SET_VERTEX_DATA_ARRAY_OFFSET",
		0x00001710,"CELL_GCM_NV4097_INVALIDATE_VERTEX_CACHE_FILE",
		0x00001714,"CELL_GCM_NV4097_INVALIDATE_VERTEX_FILE",
		0x00001718,"CELL_GCM_NV4097_PIPE_NOP",
		0x00001738,"CELL_GCM_NV4097_SET_VERTEX_DATA_BASE_OFFSET",
		0x0000173c,"CELL_GCM_NV4097_SET_VERTEX_DATA_BASE_INDEX",
		0x00001740,"CELL_GCM_NV4097_SET_VERTEX_DATA_ARRAY_FORMAT",
		0x000017c8,"CELL_GCM_NV4097_CLEAR_REPORT_VALUE",
		0x000017cc,"CELL_GCM_NV4097_SET_ZPASS_PIXEL_COUNT_ENABLE",
		0x00001800,"CELL_GCM_NV4097_GET_REPORT",
		0x00001804,"CELL_GCM_NV4097_SET_ZCULL_STATS_ENABLE",
		0x00001808,"CELL_GCM_NV4097_SET_BEGIN_END",
		0x0000180c,"CELL_GCM_NV4097_ARRAY_ELEMENT16",
		0x00001810,"CELL_GCM_NV4097_ARRAY_ELEMENT32",
		0x00001814,"CELL_GCM_NV4097_DRAW_ARRAYS",
		0x00001818,"CELL_GCM_NV4097_INLINE_ARRAY",
		0x0000181c,"CELL_GCM_NV4097_SET_INDEX_ARRAY_ADDRESS",
		0x00001820,"CELL_GCM_NV4097_SET_INDEX_ARRAY_DMA",
		0x00001824,"CELL_GCM_NV4097_DRAW_INDEX_ARRAY",
		0x00001828,"CELL_GCM_NV4097_SET_FRONT_POLYGON_MODE",
		0x0000182c,"CELL_GCM_NV4097_SET_BACK_POLYGON_MODE",
		0x00001830,"CELL_GCM_NV4097_SET_CULL_FACE",
		0x00001834,"CELL_GCM_NV4097_SET_FRONT_FACE",
		0x00001838,"CELL_GCM_NV4097_SET_POLY_SMOOTH_ENABLE",
		0x0000183c,"CELL_GCM_NV4097_SET_CULL_FACE_ENABLE",
		0x00001840,"CELL_GCM_NV4097_SET_TEXTURE_CONTROL3",
		0x00001880,"CELL_GCM_NV4097_SET_VERTEX_DATA2F_M",
		0x00001900,"CELL_GCM_NV4097_SET_VERTEX_DATA2S_M",
		0x00001940,"CELL_GCM_NV4097_SET_VERTEX_DATA4UB_M",
		0x00001980,"CELL_GCM_NV4097_SET_VERTEX_DATA4S_M",
		0x00001a00,"CELL_GCM_NV4097_SET_TEXTURE_OFFSET",
		0x00001a04,"CELL_GCM_NV4097_SET_TEXTURE_FORMAT",
		0x00001a08,"CELL_GCM_NV4097_SET_TEXTURE_ADDRESS",
		0x00001a0c,"CELL_GCM_NV4097_SET_TEXTURE_CONTROL0",
		0x00001a10,"CELL_GCM_NV4097_SET_TEXTURE_CONTROL1",
		0x00001a14,"CELL_GCM_NV4097_SET_TEXTURE_FILTER",
		0x00001a18,"CELL_GCM_NV4097_SET_TEXTURE_IMAGE_RECT",
		0x00001a1c,"CELL_GCM_NV4097_SET_TEXTURE_BORDER_COLOR",
		0x00001c00,"CELL_GCM_NV4097_SET_VERTEX_DATA4F_M",
		0x00001d00,"CELL_GCM_NV4097_SET_COLOR_KEY_COLOR",
		0x00001d60,"CELL_GCM_NV4097_SET_SHADER_CONTROL",
		0x00001d64,"CELL_GCM_NV4097_SET_INDEXED_CONSTANT_READ_LIMITS",
		0x00001d6c,"CELL_GCM_NV4097_SET_SEMAPHORE_OFFSET",
		0x00001d70,"CELL_GCM_NV4097_BACK_END_WRITE_SEMAPHORE_RELEASE",
		0x00001d74,"CELL_GCM_NV4097_TEXTURE_READ_SEMAPHORE_RELEASE",
		0x00001d78,"CELL_GCM_NV4097_SET_ZMIN_MAX_CONTROL",
		0x00001d7c,"CELL_GCM_NV4097_SET_ANTI_ALIASING_CONTROL",
		0x00001d80,"CELL_GCM_NV4097_SET_SURFACE_COMPRESSION",
		0x00001d84,"CELL_GCM_NV4097_SET_ZCULL_EN",
		0x00001d88,"CELL_GCM_NV4097_SET_SHADER_WINDOW",
		0x00001d8c,"CELL_GCM_NV4097_SET_ZSTENCIL_CLEAR_VALUE",
		0x00001d90,"CELL_GCM_NV4097_SET_COLOR_CLEAR_VALUE",
		0x00001d94,"CELL_GCM_NV4097_CLEAR_SURFACE",
		0x00001d98,"CELL_GCM_NV4097_SET_CLEAR_RECT_HORIZONTAL",
		0x00001d9c,"CELL_GCM_NV4097_SET_CLEAR_RECT_VERTICAL",
		0x00001da4,"CELL_GCM_NV4097_SET_CLIP_ID_TEST_ENABLE",
		0x00001dac,"CELL_GCM_NV4097_SET_RESTART_INDEX_ENABLE",
		0x00001db0,"CELL_GCM_NV4097_SET_RESTART_INDEX",
		0x00001db4,"CELL_GCM_NV4097_SET_LINE_STIPPLE",
		0x00001db8,"CELL_GCM_NV4097_SET_LINE_STIPPLE_PATTERN",
		0x00001e40,"CELL_GCM_NV4097_SET_VERTEX_DATA1F_M",
		0x00001e94,"CELL_GCM_NV4097_SET_TRANSFORM_EXECUTION_MODE",
		0x00001e98,"CELL_GCM_NV4097_SET_RENDER_ENABLE",
		0x00001e9c,"CELL_GCM_NV4097_SET_TRANSFORM_PROGRAM_LOAD",
		0x00001ea0,"CELL_GCM_NV4097_SET_TRANSFORM_PROGRAM_START",
		0x00001ea4,"CELL_GCM_NV4097_SET_ZCULL_CONTROL0",
		0x00001ea8,"CELL_GCM_NV4097_SET_ZCULL_CONTROL1",
		0x00001eac,"CELL_GCM_NV4097_SET_SCULL_CONTROL",
		0x00001ee0,"CELL_GCM_NV4097_SET_POINT_SIZE",
		0x00001ee4,"CELL_GCM_NV4097_SET_POINT_PARAMS_ENABLE",
		0x00001ee8,"CELL_GCM_NV4097_SET_POINT_SPRITE_CONTROL",
		0x00001ef8,"CELL_GCM_NV4097_SET_TRANSFORM_TIMEOUT",
		0x00001efc,"CELL_GCM_NV4097_SET_TRANSFORM_CONSTANT_LOAD",
		0x00001f00,"CELL_GCM_NV4097_SET_TRANSFORM_CONSTANT",
		0x00001fc0,"CELL_GCM_NV4097_SET_FREQUENCY_DIVIDER_OPERATION",
		0x00001fc4,"CELL_GCM_NV4097_SET_ATTRIB_COLOR",
		0x00001fc8,"CELL_GCM_NV4097_SET_ATTRIB_TEX_COORD",
		0x00001fcc,"CELL_GCM_NV4097_SET_ATTRIB_TEX_COORD_EX",
		0x00001fd0,"CELL_GCM_NV4097_SET_ATTRIB_UCLIP0",
		0x00001fd4,"CELL_GCM_NV4097_SET_ATTRIB_UCLIP1",
		0x00001fd8,"CELL_GCM_NV4097_INVALIDATE_L2",
		0x00001fe0,"CELL_GCM_NV4097_SET_REDUCE_DST_COLOR",
		0x00001fec,"CELL_GCM_NV4097_SET_SHADER_PACKER",
		0x00001ff0,"CELL_GCM_NV4097_SET_VERTEX_ATTRIB_INPUT_MASK",
		0x00001ff4,"CELL_GCM_NV4097_SET_VERTEX_ATTRIB_OUTPUT_MASK",
		0x00001ff8,"CELL_GCM_NV4097_SET_TRANSFORM_BRANCH_BITS",
		0x00002000,"CELL_GCM_NV0039_SET_OBJECT",
		0x00002180,"CELL_GCM_NV0039_SET_CONTEXT_DMA_NOTIFIES",
		0x00002184,"CELL_GCM_NV0039_SET_CONTEXT_DMA_BUFFER_IN",
		0x00002188,"CELL_GCM_NV0039_SET_CONTEXT_DMA_BUFFER_OUT",
		0x0000230C,"CELL_GCM_NV0039_OFFSET_IN",
		0x00002310,"CELL_GCM_NV0039_OFFSET_OUT",
		0x00002314,"CELL_GCM_NV0039_PITCH_IN",
		0x00002318,"CELL_GCM_NV0039_PITCH_OUT",
		0x0000231C,"CELL_GCM_NV0039_LINE_LENGTH_IN",
		0x00002320,"CELL_GCM_NV0039_LINE_COUNT",
		0x00002324,"CELL_GCM_NV0039_FORMAT",
		0x00002328,"CELL_GCM_NV0039_BUFFER_NOTIFY",
		0x00006000,"CELL_GCM_NV3062_SET_OBJECT",
		0x00006180,"CELL_GCM_NV3062_SET_CONTEXT_DMA_NOTIFIES",
		0x00006184,"CELL_GCM_NV3062_SET_CONTEXT_DMA_IMAGE_SOURCE",
		0x00006188,"CELL_GCM_NV3062_SET_CONTEXT_DMA_IMAGE_DESTIN",
		0x00006300,"CELL_GCM_NV3062_SET_COLOR_FORMAT",
		0x00006304,"CELL_GCM_NV3062_SET_PITCH",
		0x00006308,"CELL_GCM_NV3062_SET_OFFSET_SOURCE",
		0x0000630C,"CELL_GCM_NV3062_SET_OFFSET_DESTIN",
		0x00008000,"CELL_GCM_NV309E_SET_OBJECT",
		0x00008180,"CELL_GCM_NV309E_SET_CONTEXT_DMA_NOTIFIES",
		0x00008184,"CELL_GCM_NV309E_SET_CONTEXT_DMA_IMAGE",
		0x00008300,"CELL_GCM_NV309E_SET_FORMAT",
		0x00008304,"CELL_GCM_NV309E_SET_OFFSET",
		0x0000A000,"CELL_GCM_NV308A_SET_OBJECT",
		0x0000A180,"CELL_GCM_NV308A_SET_CONTEXT_DMA_NOTIFIES",
		0x0000A184,"CELL_GCM_NV308A_SET_CONTEXT_COLOR_KEY",
		0x0000A188,"CELL_GCM_NV308A_SET_CONTEXT_CLIP_RECTANGLE",
		0x0000A18C,"CELL_GCM_NV308A_SET_CONTEXT_PATTERN",
		0x0000A190,"CELL_GCM_NV308A_SET_CONTEXT_ROP",
		0x0000A194,"CELL_GCM_NV308A_SET_CONTEXT_BETA1",
		0x0000A198,"CELL_GCM_NV308A_SET_CONTEXT_BETA4",
		0x0000A19C,"CELL_GCM_NV308A_SET_CONTEXT_SURFACE",
		0x0000A2F8,"CELL_GCM_NV308A_SET_COLOR_CONVERSION",
		0x0000A2FC,"CELL_GCM_NV308A_SET_OPERATION",
		0x0000A300,"CELL_GCM_NV308A_SET_COLOR_FORMAT",
		0x0000A304,"CELL_GCM_NV308A_POINT",
		0x0000A308,"CELL_GCM_NV308A_SIZE_OUT",
		0x0000A30C,"CELL_GCM_NV308A_SIZE_IN",
		0x0000A400,"CELL_GCM_NV308A_COLOR",
		0x0000C000,"CELL_GCM_NV3089_SET_OBJECT",
		0x0000C180,"CELL_GCM_NV3089_SET_CONTEXT_DMA_NOTIFIES",
		0x0000C184,"CELL_GCM_NV3089_SET_CONTEXT_DMA_IMAGE",
		0x0000C188,"CELL_GCM_NV3089_SET_CONTEXT_PATTERN",
		0x0000C18C,"CELL_GCM_NV3089_SET_CONTEXT_ROP",
		0x0000C190,"CELL_GCM_NV3089_SET_CONTEXT_BETA1",
		0x0000C194,"CELL_GCM_NV3089_SET_CONTEXT_BETA4",
		0x0000C198,"CELL_GCM_NV3089_SET_CONTEXT_SURFACE",
		0x0000C2FC,"CELL_GCM_NV3089_SET_COLOR_CONVERSION",
		0x0000C300,"CELL_GCM_NV3089_SET_COLOR_FORMAT",
		0x0000C304,"CELL_GCM_NV3089_SET_OPERATION",
		0x0000C308,"CELL_GCM_NV3089_CLIP_POINT",
		0x0000C30C,"CELL_GCM_NV3089_CLIP_SIZE",
		0x0000C310,"CELL_GCM_NV3089_IMAGE_OUT_POINT",
		0x0000C314,"CELL_GCM_NV3089_IMAGE_OUT_SIZE",
		0x0000C318,"CELL_GCM_NV3089_DS_DX",
		0x0000C31C,"CELL_GCM_NV3089_DT_DY",
		0x0000C400,"CELL_GCM_NV3089_IMAGE_IN_SIZE",
		0x0000C404,"CELL_GCM_NV3089_IMAGE_IN_FORMAT",
		0x0000C408,"CELL_GCM_NV3089_IMAGE_IN_OFFSET",
		0x0000C40C,"CELL_GCM_NV3089_IMAGE_IN",
		0x0920,"VertexTextureOffset Corresponds to the vertex texture sampler index = 1",
		0x0924,"VertexTextureFormat Corresponds to the vertex texture sampler index = 1",
		0x0928,"VertexTextureAddress Corresponds to the vertex texture sampler index = 1",
		0x092c,"VertexTextureControl0 Corresponds to the vertex texture sampler index = 1",
		0x0930,"VertexTextureControl3 Corresponds to the vertex texture sampler index = 1",
		0x0934,"VertexTextureFilter Corresponds to the vertex texture sampler index = 1",
		0x0938,"VertexTextureImageRect Corresponds to the vertex texture sampler index = 1",
		0x093c,"VertexTextureBorderColor Corresponds to the vertex texture sampler index = 1",
		0x0940,"VertexTextureOffset Corresponds to the vertex texture sampler index = 2",
		0x0944,"VertexTextureFormat Corresponds to the vertex texture sampler index = 2",
		0x0948,"VertexTextureAddress Corresponds to the vertex texture sampler index = 2",
		0x094c,"VertexTextureControl0 Corresponds to the vertex texture sampler index = 2",
		0x0950,"VertexTextureControl3 Corresponds to the vertex texture sampler index = 2",
		0x0954,"VertexTextureFilter Corresponds to the vertex texture sampler index = 2",
		0x0958,"VertexTextureImageRect Corresponds to the vertex texture sampler index = 2",
		0x095c,"VertexTextureBorderColor Corresponds to the vertex texture sampler index = 2",
		0x0960,"VertexTextureOffset Corresponds to the vertex texture sampler index = 3",
		0x0964,"VertexTextureFormat Corresponds to the vertex texture sampler index = 3",
		0x0968,"VertexTextureAddress Corresponds to the vertex texture sampler index = 3",
		0x096c,"VertexTextureControl0 Corresponds to the vertex texture sampler index = 3",
		0x0970,"VertexTextureControl3 Corresponds to the vertex texture sampler index = 3",
		0x0974,"VertexTextureFilter Corresponds to the vertex texture sampler index = 3",
		0x0978,"VertexTextureImageRect Corresponds to the vertex texture sampler index = 3",
		0x097c,"VertexTextureBorderColor Corresponds to the vertex texture sampler index = 3",
		0x0b84,"VertexProgram 2nd word of the VertexProgram to be transferred",
		0x0b88,"VertexProgram 3rd word of the VertexProgram to be transferred",
		0x0b8c,"VertexProgram 4th word of the VertexProgram to be transferred",
		0x0b90,"VertexProgram 5th word of the VertexProgram to be transferred",
		0x0b94,"VertexProgram 6th word of the VertexProgram to be transferred",
		0x0b98,"VertexProgram 7th word of the VertexProgram to be transferred",
		0x0b9c,"VertexProgram 8th word of the VertexProgram to be transferred",
		0x0ba0,"VertexProgram 9th word of the VertexProgram to be transferred",
		0x0ba4,"VertexProgram 10th word of the VertexProgram to be transferred",
		0x0ba8,"VertexProgram 11th word of the VertexProgram to be transferred",
		0x0bac,"VertexProgram 12th word of the VertexProgram to be transferred",
		0x0bb0,"VertexProgram 13th word of the VertexProgram to be transferred",
		0x0bb4,"VertexProgram 14th word of the VertexProgram to be transferred",
		0x0bb8,"VertexProgram 15th word of the VertexProgram to be transferred",
		0x0bbc,"VertexProgram 16th word of the VertexProgram to be transferred",
		0x0bc0,"VertexProgram 17th word of the VertexProgram to be transferred",
		0x0bc4,"VertexProgram 18th word of the VertexProgram to be transferred",
		0x0bc8,"VertexProgram 19th word of the VertexProgram to be transferred",
		0x0bcc,"VertexProgram 20th word of the VertexProgram to be transferred",
		0x0bd0,"VertexProgram 21st word of the VertexProgram to be transferred",
		0x0bd4,"VertexProgram 22nd word of the VertexProgram to be transferred",
		0x0bd8,"VertexProgram 23rd word of the VertexProgram to be transferred",
		0x0bdc,"VertexProgram 24th word of the VertexProgram to be transferred",
		0x0be0,"VertexProgram 25th word of the VertexProgram to be transferred",
		0x0be4,"VertexProgram 26th word of the VertexProgram to be transferred",
		0x0be8,"VertexProgram 27th word of the VertexProgram to be transferred",
		0x0bec,"VertexProgram 28th word of the VertexProgram to be transferred",
		0x0bf0,"VertexProgram 29th word of the VertexProgram to be transferred",
		0x0bf4,"VertexProgram 30th word of the VertexProgram to be transferred",
		0x0bf8,"VertexProgram 31st word of the VertexProgram to be transferred",
		0x0bfc,"VertexProgram 32nd word of the VertexProgram to be transferred",
		0x1684,"VertexDataArrayOffset Corresponds to vertex attribute slot index=1 (ATTR1)",
		0x1688,"VertexDataArrayOffset Corresponds to vertex attribute slot index=2 (ATTR2)",
		0x168c,"VertexDataArrayOffset Corresponds to vertex attribute slot index=3 (ATTR3)",
		0x1690,"VertexDataArrayOffset Corresponds to vertex attribute slot index=4 (ATTR4)",
		0x1694,"VertexDataArrayOffset Corresponds to vertex attribute slot index=5 (ATTR5)",
		0x1698,"VertexDataArrayOffset Corresponds to vertex attribute slot index=6 (ATTR6)",
		0x169c,"VertexDataArrayOffset Corresponds to vertex attribute slot index=7 (ATTR7)",
		0x16a0,"VertexDataArrayOffset Corresponds to vertex attribute slot index=8 (ATTR8)",
		0x16a4,"VertexDataArrayOffset Corresponds to vertex attribute slot index=9 (ATTR9)",
		0x16a8,"VertexDataArrayOffset Corresponds to vertex attribute slot index=10 (ATTR10)",
		0x16ac,"VertexDataArrayOffset Corresponds to vertex attribute slot index=11 (ATTR11)",
		0x16b0,"VertexDataArrayOffset Corresponds to vertex attribute slot index=12 (ATTR12)",
		0x16b4,"VertexDataArrayOffset Corresponds to vertex attribute slot index=13 (ATTR13)",
		0x16b8,"VertexDataArrayOffset Corresponds to vertex attribute slot index=14 (ATTR14)",
		0x16bc,"VertexDataArrayOffset Corresponds to vertex attribute slot index=15 (ATTR15)",
		0x1744,"VertexDataArrayFormat Corresponds to vertex attribute slot index=1 (ATTR1)",
		0x1748,"VertexDataArrayFormat Corresponds to vertex attribute slot index=2 (ATTR2)",
		0x174c,"VertexDataArrayFormat Corresponds to vertex attribute slot index=3 (ATTR3)",
		0x1750,"VertexDataArrayFormat Corresponds to vertex attribute slot index=4 (ATTR4)",
		0x1754,"VertexDataArrayFormat Corresponds to vertex attribute slot index=5 (ATTR5)",
		0x1758,"VertexDataArrayFormat Corresponds to vertex attribute slot index=6 (ATTR6)",
		0x175c,"VertexDataArrayFormat Corresponds to vertex attribute slot index=7 (ATTR7)",
		0x1760,"VertexDataArrayFormat Corresponds to vertex attribute slot index=8 (ATTR8)",
		0x1764,"VertexDataArrayFormat Corresponds to vertex attribute slot index=9 (ATTR9)",
		0x1768,"VertexDataArrayFormat Corresponds to vertex attribute slot index=10 (ATTR10)",
		0x176c,"VertexDataArrayFormat Corresponds to vertex attribute slot index=11 (ATTR11)",
		0x1770,"VertexDataArrayFormat Corresponds to vertex attribute slot index=12 (ATTR12)",
		0x1774,"VertexDataArrayFormat Corresponds to vertex attribute slot index=13 (ATTR13)",
		0x1778,"VertexDataArrayFormat Corresponds to vertex attribute slot index=14 (ATTR14)",
		0x177c,"VertexDataArrayFormat Corresponds to vertex attribute slot index=15 (ATTR15)",
		0x0b04,"TextureControl2 Corresponds to the texture sampler index=1",
		0x0b08,"TextureControl2 Corresponds to the texture sampler index=2",
		0x0b0c,"TextureControl2 Corresponds to the texture sampler index=3",
		0x0b10,"TextureControl2 Corresponds to the texture sampler index=4",
		0x0b14,"TextureControl2 Corresponds to the texture sampler index=5",
		0x0b18,"TextureControl2 Corresponds to the texture sampler index=6",
		0x0b1c,"TextureControl2 Corresponds to the texture sampler index=7",
		0x0b20,"TextureControl2 Corresponds to the texture sampler index=8",
		0x0b24,"TextureControl2 Corresponds to the texture sampler index=9",
		0x0b28,"TextureControl2 Corresponds to the texture sampler index=10",
		0x0b2c,"TextureControl2 Corresponds to the texture sampler index=11",
		0x0b30,"TextureControl2 Corresponds to the texture sampler index=12",
		0x0b34,"TextureControl2 Corresponds to the texture sampler index=13",
		0x0b38,"TextureControl2 Corresponds to the texture sampler index=14",
		0x0b3c,"TextureControl2 Corresponds to the texture sampler index=15",
		0x1844,"TextureControl3 Corresponds to the texture sampler index=1",
		0x1848,"TextureControl3 Corresponds to the texture sampler index=2",
		0x184c,"TextureControl3 Corresponds to the texture sampler index=3",
		0x1850,"TextureControl3 Corresponds to the texture sampler index=4",
		0x1854,"TextureControl3 Corresponds to the texture sampler index=5",
		0x1858,"TextureControl3 Corresponds to the texture sampler index=6",
		0x185c,"TextureControl3 Corresponds to the texture sampler index=7",
		0x1860,"TextureControl3 Corresponds to the texture sampler index=8",
		0x1864,"TextureControl3 Corresponds to the texture sampler index=9",
		0x1868,"TextureControl3 Corresponds to the texture sampler index=10",
		0x186c,"TextureControl3 Corresponds to the texture sampler index=11",
		0x1870,"TextureControl3 Corresponds to the texture sampler index=12",
		0x1874,"TextureControl3 Corresponds to the texture sampler index=13",
		0x1878,"TextureControl3 Corresponds to the texture sampler index=14",
		0x187c,"TextureControl3 Corresponds to the texture sampler index=15",
		0x1a20,"TextureOffset Corresponds to the texture sampler index=1",
		0x1a24,"TextureFormat Corresponds to the texture sampler index=1",
		0x1a28,"TextureAddress Corresponds to the texture sampler index=1",
		0x1a2c,"TextureControl0 Corresponds to the texture sampler index=1",
		0x1a30,"TextureControl1 Corresponds to the texture sampler index=1",
		0x1a34,"TextureFilter Corresponds to the texture sampler index=1",
		0x1a38,"TextureImageRect Corresponds to the texture sampler index=1",
		0x1a3c,"TextureBorderColor Corresponds to the texture sampler index=1",
		0x1a40,"TextureOffset Corresponds to the texture sampler index=2",
		0x1a44,"TextureFormat Corresponds to the texture sampler index=2",
		0x1a48,"TextureAddress Corresponds to the texture sampler index=2",
		0x1a4c,"TextureControl0 Corresponds to the texture sampler index=2",
		0x1a50,"TextureControl1 Corresponds to the texture sampler index=2",
		0x1a54,"TextureFilter Corresponds to the texture sampler index=2",
		0x1a58,"TextureImageRect Corresponds to the texture sampler index=2",
		0x1a5c,"TextureBorderColor Corresponds to the texture sampler index=2",
		0x1a60,"TextureOffset Corresponds to the texture sampler index=3",
		0x1a64,"TextureFormat Corresponds to the texture sampler index=3",
		0x1a68,"TextureAddress Corresponds to the texture sampler index=3",
		0x1a6c,"TextureControl0 Corresponds to the texture sampler index=3",
		0x1a70,"TextureControl1 Corresponds to the texture sampler index=3",
		0x1a74,"TextureFilter Corresponds to the texture sampler index=3",
		0x1a78,"TextureImageRect Corresponds to the texture sampler index=3",
		0x1a7c,"TextureBorderColor Corresponds to the texture sampler index=3",
		0x1a80,"TextureOffset Corresponds to the texture sampler index=4",
		0x1a84,"TextureFormat Corresponds to the texture sampler index=4",
		0x1a88,"TextureAddress Corresponds to the texture sampler index=4",
		0x1a8c,"TextureControl0 Corresponds to the texture sampler index=4",
		0x1a90,"TextureControl1 Corresponds to the texture sampler index=4",
		0x1a94,"TextureFilter Corresponds to the texture sampler index=4",
		0x1a98,"TextureImageRect Corresponds to the texture sampler index=4",
		0x1a9c,"TextureBorderColor Corresponds to the texture sampler index=4",
		0x1aa0,"TextureOffset Corresponds to the texture sampler index=5",
		0x1aa4,"TextureFormat Corresponds to the texture sampler index=5",
		0x1aa8,"TextureAddress Corresponds to the texture sampler index=5",
		0x1aac,"TextureControl0 Corresponds to the texture sampler index=5",
		0x1ab0,"TextureControl1 Corresponds to the texture sampler index=5",
		0x1ab4,"TextureFilter Corresponds to the texture sampler index=5",
		0x1ab8,"TextureImageRect Corresponds to the texture sampler index=5",
		0x1abc,"TextureBorderColor Corresponds to the texture sampler index=5",
		0x1ac0,"TextureOffset Corresponds to the texture sampler index=6",
		0x1ac4,"TextureFormat Corresponds to the texture sampler index=6",
		0x1ac8,"TextureAddress Corresponds to the texture sampler index=6",
		0x1acc,"TextureControl0 Corresponds to the texture sampler index=6",
		0x1ad0,"TextureControl1 Corresponds to the texture sampler index=6",
		0x1ad4,"TextureFilter Corresponds to the texture sampler index=6",
		0x1ad8,"TextureImageRect Corresponds to the texture sampler index=6",
		0x1adc,"TextureBorderColor Corresponds to the texture sampler index=6",
		0x1ae0,"TextureOffset Corresponds to the texture sampler index=7",
		0x1ae4,"TextureFormat Corresponds to the texture sampler index=7",
		0x1ae8,"TextureAddress Corresponds to the texture sampler index=7",
		0x1aec,"TextureControl0 Corresponds to the texture sampler index=7",
		0x1af0,"TextureControl1 Corresponds to the texture sampler index=7",
		0x1af4,"TextureFilter Corresponds to the texture sampler index=7",
		0x1af8,"TextureImageRect Corresponds to the texture sampler index=7",
		0x1afc,"TextureBorderColor Corresponds to the texture sampler index=7",
		0x1b00,"TextureOffset Corresponds to the texture sampler index=8",
		0x1b04,"TextureFormat Corresponds to the texture sampler index=8",
		0x1b08,"TextureAddress Corresponds to the texture sampler index=8",
		0x1b0c,"TextureControl0 Corresponds to the texture sampler index=8",
		0x1b10,"TextureControl1 Corresponds to the texture sampler index=8",
		0x1b14,"TextureFilter Corresponds to the texture sampler index=8",
		0x1b18,"TextureImageRect Corresponds to the texture sampler index=8",
		0x1b1c,"TextureBorderColor Corresponds to the texture sampler index=8",
		0x1b20,"TextureOffset Corresponds to the texture sampler index=9",
		0x1b24,"TextureFormat Corresponds to the texture sampler index=9",
		0x1b28,"TextureAddress Corresponds to the texture sampler index=9",
		0x1b2c,"TextureControl0 Corresponds to the texture sampler index=9",
		0x1b30,"TextureControl1 Corresponds to the texture sampler index=9",
		0x1b34,"TextureFilter Corresponds to the texture sampler index=9",
		0x1b38,"TextureImageRect Corresponds to the texture sampler index=9",
		0x1b3c,"TextureBorderColor Corresponds to the texture sampler index=9",
		0x1b40,"TextureOffset Corresponds to the texture sampler index=10",
		0x1b44,"TextureFormat Corresponds to the texture sampler index=10",
		0x1b48,"TextureAddress Corresponds to the texture sampler index=10",
		0x1b4c,"TextureControl0 Corresponds to the texture sampler index=10",
		0x1b50,"TextureControl1 Corresponds to the texture sampler index=10",
		0x1b54,"TextureFilter Corresponds to the texture sampler index=10",
		0x1b58,"TextureImageRect Corresponds to the texture sampler index=10",
		0x1b5c,"TextureBorderColor Corresponds to the texture sampler index=10",
		0x1b60,"TextureOffset Corresponds to the texture sampler index=11",
		0x1b64,"TextureFormat Corresponds to the texture sampler index=11",
		0x1b68,"TextureAddress Corresponds to the texture sampler index=11",
		0x1b6c,"TextureControl0 Corresponds to the texture sampler index=11",
		0x1b70,"TextureControl1 Corresponds to the texture sampler index=11",
		0x1b74,"TextureFilter Corresponds to the texture sampler index=11",
		0x1b78,"TextureImageRect Corresponds to the texture sampler index=11",
		0x1b7c,"TextureBorderColor Corresponds to the texture sampler index=11",
		0x1b80,"TextureOffset Corresponds to the texture sampler index=12",
		0x1b84,"TextureFormat Corresponds to the texture sampler index=12",
		0x1b88,"TextureAddress Corresponds to the texture sampler index=12",
		0x1b8c,"TextureControl0 Corresponds to the texture sampler index=12",
		0x1b90,"TextureControl1 Corresponds to the texture sampler index=12",
		0x1b94,"TextureFilter Corresponds to the texture sampler index=12",
		0x1b98,"TextureImageRect Corresponds to the texture sampler index=12",
		0x1b9c,"TextureBorderColor Corresponds to the texture sampler index=12",
		0x1ba0,"TextureOffset Corresponds to the texture sampler index=13",
		0x1ba4,"TextureFormat Corresponds to the texture sampler index=13",
		0x1ba8,"TextureAddress Corresponds to the texture sampler index=13",
		0x1bac,"TextureControl0 Corresponds to the texture sampler index=13",
		0x1bb0,"TextureControl1 Corresponds to the texture sampler index=13",
		0x1bb4,"TextureFilter Corresponds to the texture sampler index=13",
		0x1bb8,"TextureImageRect Corresponds to the texture sampler index=13",
		0x1bbc,"TextureBorderColor Corresponds to the texture sampler index=13",
		0x1bc0,"TextureOffset Corresponds to the texture sampler index=14",
		0x1bc4,"TextureFormat Corresponds to the texture sampler index=14",
		0x1bc8,"TextureAddress Corresponds to the texture sampler index=14",
		0x1bcc,"TextureControl0 Corresponds to the texture sampler index=14",
		0x1bd0,"TextureControl1 Corresponds to the texture sampler index=14",
		0x1bd4,"TextureFilter Corresponds to the texture sampler index=14",
		0x1bd8,"TextureImageRect Corresponds to the texture sampler index=14",
		0x1bdc,"TextureBorderColor Corresponds to the texture sampler index=14",
		0x1be0,"TextureOffset Corresponds to the texture sampler index=15",
		0x1be4,"TextureFormat Corresponds to the texture sampler index=15",
		0x1be8,"TextureAddress Corresponds to the texture sampler index=15",
		0x1bec,"TextureControl0 Corresponds to the texture sampler index=15",
		0x1bf0,"TextureControl1 Corresponds to the texture sampler index=15",
		0x1bf4,"TextureFilter Corresponds to the texture sampler index=15",
		0x1bf8,"TextureImageRect Corresponds to the texture sampler index=15",
		0x1bfc,"TextureBorderColor Corresponds to the texture sampler index=15",
		0x1f04,"VertexConstant 2nd word of VertexConstant to be transferred",
		0x1f08,"VertexConstant 3rd word of VertexConstant to be transferred",
		0x1f0c,"VertexConstant 4th word of VertexConstant to be transferred",
		0x1f10,"VertexConstant 5th word of VertexConstant to be transferred",
		0x1f14,"VertexConstant 6th word of VertexConstant to be transferred",
		0x1f18,"VertexConstant 7th word of VertexConstant to be transferred",
		0x1f1c,"VertexConstant 8th word of VertexConstant to be transferred",
		0x1f20,"VertexConstant 9th word of VertexConstant to be transferred",
		0x1f24,"VertexConstant 10th word of VertexConstant to be transferred",
		0x1f28,"VertexConstant 11th word of VertexConstant to be transferred",
		0x1f2c,"VertexConstant 12th word of VertexConstant to be transferred",
		0x1f30,"VertexConstant 13th word of VertexConstant to be transferred",
		0x1f34,"VertexConstant 14th word of VertexConstant to be transferred",
		0x1f38,"VertexConstant 15th word of VertexConstant to be transferred",
		0x1f3c,"VertexConstant 16th word of VertexConstant to be transferred",
		0x1f40,"VertexConstant 17th word of VertexConstant to be transferred",
		0x1f44,"VertexConstant 18th word of VertexConstant to be transferred",
		0x1f48,"VertexConstant 19th word of VertexConstant to be transferred",
		0x1f4c,"VertexConstant 20th word of VertexConstant to be transferred",
		0x1f50,"VertexConstant 21st word of VertexConstant to be transferred",
		0x1f54,"VertexConstant 22nd word of VertexConstant to be transferred",
		0x1f58,"VertexConstant 23rd word of VertexConstant to be transferred",
		0x1f5c,"VertexConstant 24th word of VertexConstant to be transferred",
		0x1f60,"VertexConstant 25th word of VertexConstant to be transferred",
		0x1f64,"VertexConstant 26th word of VertexConstant to be transferred",
		0x1f68,"VertexConstant 27th word of VertexConstant to be transferred",
		0x1f6c,"VertexConstant 28th word of VertexConstant to be transferred",
		0x1f70,"VertexConstant 29th word of VertexConstant to be transferred",
		0x1f74,"VertexConstant 30th word of VertexConstant to be transferred",
		0x1f78,"VertexConstant 31st word of VertexConstant to be transferred",
		0x1f7c,"VertexConstant 32nd word of VertexConstant to be transferred",
	};

	const U32 methodCount = sizeof(methods) / sizeof(C_Method);

	for(U32 methodIndex=0; methodIndex<methodCount; methodIndex++)
	{
		const C_Method& method = methods[methodIndex];

		if(methodAddress == method.m_Address)
		{
			methodName = method.m_Name;

			return true;
		}
	}

	methodName = "UNKNOWN";

	return false;
}

} // namespace Display2
