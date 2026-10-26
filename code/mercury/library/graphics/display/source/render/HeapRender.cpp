#include "stdafx.h"
#include "Display/Render/HeapRender.h"
#include "Display/Render/BasicRender.h"
#include "Display/Render/FontRender.h"
#include "Display/Device/Device.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

#ifdef PLATFORM_PS3

enum E_HeapRender
{
	E_HeapRender_CmdBuf,
	E_HeapRender_Video,
	E_HeapRender_System,
};

DEBUG_ENUM(E_HeapRender)
	DE_OPTION(E_HeapRender_CmdBuf,	"CmdBuf")
	DE_OPTION(E_HeapRender_Video,	"Video")
	DE_OPTION(E_HeapRender_System,	"System")
END_DEBUG_ENUM

Debug::Enum<E_HeapRender> g_debugDisplayHeapRenderShowHeap("Graphics.Display.Render.HeapRender.ShowHeap", E_HeapRender_Video);

#endif // PLATFORM_PS3


Debug::Int	g_debugDisplayHeapRenderTransparency("Graphics.Display.Render.HeapRender.Transparency", 220, Debug::Range(0, 255));
Debug::Int	g_debugDisplayHeapRenderSizePerLine("Graphics.Display.Render.HeapRender.SizePerLine", 50, Debug::Range(5, 500));

namespace Display
{

C_HeapRender::C_HeapRender()
{
	assert(g_pHeapRender == NULL);
	g_pHeapRender = this;
}

C_HeapRender::~C_HeapRender()
{
	assert(g_pHeapRender == this);
	g_pHeapRender = NULL; 
}

void C_HeapRender::Draw()
{
	Util::C_String bufferName = "";
	C_MemoryBuffer* pBuffer = NULL;

	#ifdef PLATFORM_PS3
	switch(g_debugDisplayHeapRenderShowHeap)
	{
	case E_HeapRender_CmdBuf:
		bufferName = "CMDBUF";
		pBuffer = Device()->GetCmdBufMemory();
		break;
	case E_HeapRender_Video:
		bufferName = "VIDEO";
		pBuffer = Device()->GetVideoMemory();
		break;
	case E_HeapRender_System:
		bufferName = "SYSTEM";
		pBuffer = Device()->GetSystemMemory();
		break;
	}
	#endif


	Draw(pBuffer, bufferName);
}

void C_HeapRender::Draw(C_MemoryBuffer* pBuffer, const Util::C_String& bufferName)
{
	//BasicRender()->DrawColoured2DRect(20.f, 20.f, 984.f, 728.f, Maths::RGBA(0, 0, 0, g_debugDisplayHeapRenderTransparency));
	//FontRender()->Draw(":: Heap Usage ::", 50.f, 60.f, 20.f, 20.f, GREEN);

	F32 x = 70.f;
	F32 y = 110.f;

	if(pBuffer)
	{
	#if defined(PLATFORM_PS3)
		F32 usedSize = (F32)pBuffer->GetUsed() / (1024.f * 1024.f);
		F32 freeSize = (F32)pBuffer->GetFree() / (1024.f * 1024.f);
		
		F32 widthSize = (F32)g_debugDisplayHeapRenderSizePerLine;
		F32 totalSize = (F32)pBuffer->GetSize() / (1024.f * 1024.f);
		F32 scaleSize = 800.f / widthSize;

		Device()->SetDepthEnable(E_Bool_False);
		Device()->SetDepthWriteEnable(E_Bool_False);

		{
			F32 lineCount = (F32)((S32)((totalSize + widthSize - 1) / widthSize));
			BasicRender()->DrawColoured2DRect(x - 10.f, y - 10.f, 825.f, 45.f + lineCount * 25.f, Maths::RGBA(0, 0, 0, g_debugDisplayHeapRenderTransparency));
		}

		Btl::Vector<Sys::MemoryBlock> blocks;
		pBuffer->GetFreeList(blocks);

		char temp[256];
		sprintf_s(temp, sizeof(temp), "%s: size %.2fmb / used %.2fmb  / free %.2fmb / %i / %i",
			bufferName.Chars(),
			totalSize,
			usedSize,
			freeSize,
			pBuffer->GetAllocs(),
			blocks.Size());
		FontRender()->Draw(temp, x, y, 15, 15, Maths::RGBA(255, 255, 255, 255));
		y += 25.f;

		Device()->SetDepthEnable(E_Bool_False);
		Device()->SetDepthWriteEnable(E_Bool_False);

		F32 y1 = y;
		F32 remanSize = totalSize;
		while(1)
		{
			F32 w = (remanSize > widthSize ? widthSize : remanSize) * scaleSize;
			BasicRender()->DrawColoured2DRect(x, y1, w, 20.f, Maths::RGBA(0, 0, 128, g_debugDisplayHeapRenderTransparency));
			y1 += 25.f;
			remanSize -= widthSize;
			if(remanSize < 0.f)
				break;
		}

		F32 tempSize = 0.f;

		const U8* baseAddress = pBuffer->GetAddress();

		//for(C_MemoryMap::Iterator it = pBuffer->m_FreeList.Begin(); it != pBuffer->m_FreeList.End(); ++it)
		for (int i = 0; i < blocks.Size(); i++)
		{
			//F32 blockSize = ((F32)it->GetValue() / (1024.f * 1024.f));
			//F32 blockOffset = ((F32)((U32)(it->GetKey() - pBuffer->GetAddress())) / (1024.f * 1024.f));

			F32 blockSize = blocks[i].size / (1024.f * 1024.f);
			F32 blockOffset = (blocks[i].address - baseAddress) / (1024.f * 1024.f);
			Maths::RGBA colour = Maths::RGBA(0, 255, 0, g_debugDisplayHeapRenderTransparency);

			while(1)
			{
				F32 lineCount = (F32)((S32)((blockOffset / widthSize) + 0.0001f));
				F32 y1 = lineCount * 25.f;
				F32 lineOffset = blockOffset - (lineCount * widthSize);
				F32 x1 = lineOffset * scaleSize;
				F32 tempSize = lineOffset + blockSize > widthSize ? widthSize - lineOffset : blockSize;
				F32 w1 = tempSize * scaleSize;
				BasicRender()->DrawColoured2DRect(x + x1, y + y1, w1, 20.f, colour);
				blockSize -= tempSize;
				blockOffset += tempSize;
				if(blockSize <= 0.f)
					break;
			}
		}
	#endif
	}
	else
	{
		FontRender()->Draw("There is no heap...", x, y, 20, 20, Maths::RGBA(255, 255, 255, 255));
	}
}

C_HeapRender* g_pHeapRender = NULL;

void HeapRender(C_HeapRender* pHeapRender)
{
	g_pHeapRender = pHeapRender;
}

}; // namespace Display
