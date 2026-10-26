#include "DebugSystemsPch.h"
#include <DebugSystems/DebugHUDMemory.h>
#include <Render/font/Font.h>
#include <Render/debug/DebugRender.h>
#include <System/memory/MemoryTracker.h>
#include <algorithm>

Debug::Bool g_MemoryStatsShowMemoryUsage("System.MemoryHUD.ShowMemoryUsage", false );
Debug::Int g_MemoryStatsScale("System.MemoryHUD.Scale", 96 * 5, Debug::Range(1, 1000) );
Debug::Float g_MemoryStatsOffsetX("System.MemoryHUD.OffsetX", 0.f, Debug::Range(-3000.0f, 800.f) );
Debug::Float g_MemoryStatsOffsetY("System.MemoryHUD.OffsetY", 0.f, Debug::Range(-3000.0f, 600.f) );
Debug::Int g_MemoryStatsDisplayLevel("System.MemoryHUD.Level", 4, Debug::Range(0, 255));
Debug::Float g_MemoryStatsThreshold("System.MemoryHUD.Threshold",	0.0f, Debug::Range(0.0f, 1500.0f) );
Debug::Bool g_MemoryStatsShowAsKb("System.MemoryHUD.AlwaysShowAsKB", false );
Debug::Bool g_MemoryStatsShowBlocks("System.MemoryHUD.ShowBlocks", false );
Debug::Bool g_MemoryStatsStack("System.MemoryHUD.Stack", false );
Debug::Int g_MemoryFontScale("System.MemoryHUD.FontScale", 24, Debug::Range(5, 40));

DebugSystems::C_DebugHUDMemory::C_DebugHUDMemory()
{
	m_pFontRenderer = NULL;
}

DebugSystems::C_DebugHUDMemory::~C_DebugHUDMemory()
{
	if (m_pFontRenderer)
		delete m_pFontRenderer;
}

Maths::RGBA g_MemoryItemColours[6] = 
{
Maths::RGBA((U8)0xff,(U8)0x00, (U8)0x00, (U8)0x80),
Maths::RGBA((U8)0xff,(U8)0xff, (U8)0x00, (U8)0x80),
Maths::RGBA((U8)0x00,(U8)0xff, (U8)0x00, (U8)0x80),
Maths::RGBA((U8)0x00,(U8)0xff, (U8)0xff, (U8)0x80),
Maths::RGBA((U8)0x00,(U8)0x00, (U8)0xff, (U8)0x80),
Maths::RGBA((U8)0xff,(U8)0x00, (U8)0xff, (U8)0x80)
};

void DebugSystems::C_DebugHUDMemory::Render()
{
	if (g_MemoryStatsShowMemoryUsage)
	{
		PERF_EVENT("DebugHUDMemory.Render.Memory");

		if (m_pFontRenderer == NULL)
		{
			//Create the font renderer
			m_pFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");

			m_pFontRenderer->SetShadowEnable( true );
			m_pFontRenderer->SetColourOutline(Maths::RGBA(0,0,25,255));
			m_pFontRenderer->SetOutlineSize(0.35f);
		}
		//Set Font Size
		m_pFontRenderer->SetScaleX(F32(g_MemoryFontScale));
		m_pFontRenderer->SetScaleY(F32(g_MemoryFontScale));

		Maths::HVec2 barOffset( (F32)g_MemoryStatsOffsetX + 128.0f, 0.f);
		const U32 oneK = 1024;
		const U32 oneMeg = oneK * 1024;
		const U32 hundredMeg = oneMeg * 100;    
		const F32 hundredMegPixelSize = (F32)g_MemoryStatsScale;  
	        
		Maths::HVec2 pos( 64.0f, 96.0f + (F32)g_MemoryStatsOffsetY );
	                           
		double currOffset = 0.0f;

		U32 count = Memory::TrackNodes::Instance().GetNumContextNodes();

		// draw 'grid'
		for(U32 i=0; i<6; i++)
		{
			Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
			Render::Debug::Immediate()->Draw2dBar( pos + barOffset + Maths::HVec2( hundredMegPixelSize * (F32)i, 0.0f ), Maths::HVec2( 4.0f, (F32)count * m_pFontRenderer->GetScaleY() ), Maths::RGBA(0x00, 0x00, 0x00, 0xff));
			Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);
		}
		
		//Collect Nodes and sort them
		m_MemoryContextNodeVector.Clear();

		float fTotal = 0;
		float fTotalWaste = 0;
		m_MemoryContextNodeVector.Resize(count);
		for (U32 i=0; i<count; i++)
		{
			m_MemoryContextNodeVector[i].pNodeName = &Memory::TrackNodes::Instance().GetContextNode(i);
			m_MemoryContextNodeVector[i].pNodePtr = &Memory::Tracker::Instance().GetContextNode(i);
			if (!m_MemoryContextNodeVector[i].pNodePtr->m_pParent)
			{
				fTotal += m_MemoryContextNodeVector[i].pNodePtr->m_iAllocated;
				fTotalWaste += m_MemoryContextNodeVector[i].pNodePtr->m_iWasted;
			}
		}

		// sort nodes
		std::sort(&m_MemoryContextNodeVector[0], &m_MemoryContextNodeVector[count - 1]);
		 
		//Draw nodes
		F32 start = 0;
		U32 itemCount = 0;
		U32 totalMem = 0;
		U32 totalHeader = 0;
		for(U32 id=0; id<count+1; id++)
		{        
			const Memory::ContextNode *pNode;
			const Memory::ContextNodeName *pNodeName;

			// if there were any calls to this code section
			U32 usedMem;
			U32 headerMem;
			Util::C_String idName;
	        
			if (id == count)
			{
					usedMem = (U32)fTotal;
					headerMem = (U32)fTotalWaste;
					idName = "Total";
			}
			else
			{
				pNode = m_MemoryContextNodeVector[id].pNodePtr;
				pNodeName = m_MemoryContextNodeVector[id].pNodeName;
				usedMem		= pNode->m_iAllocated;
				headerMem	= pNode->m_iWasted;                
				idName		= pNodeName->m_name;
			
				totalMem += usedMem;
				totalHeader += headerMem;
				//if ((usedMem + headerMem) <= (g_MemoryStatsThreshold * 1024)) continue;
			}
			int dotCount = 0;
			for (int i = 0 ; i < idName.Length() ; i++)
			{
				if (idName.Chars()[i] == '.') dotCount++;
			}
			if (dotCount > g_MemoryStatsDisplayLevel) continue;

			const F32 drawValue = (F32)(hundredMegPixelSize * (double)usedMem / (double)hundredMeg);
			const F32 drawValue2 = (F32)(hundredMegPixelSize * (double)headerMem / (double)hundredMeg);
	        
			Util::C_String usedMemString;
			F32 mbUsed = (F32)usedMem / (F32)oneMeg;
			U32 kbUsed = usedMem / oneK;
			U32 kbHeader = headerMem / oneK;             
	            	
			F32 mid = start + drawValue;
			F32 end = mid + drawValue2;
	        
    		Maths::RGBA colour = g_MemoryItemColours[id % 6]; 
    		
			Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
			Render::Debug::Immediate()->Draw2dBar( Maths::HVec2(0.f, pos.Y() + barOffset.Y()), Maths::HVec2( barOffset.X() + pos.X(), m_pFontRenderer->GetScaleY() ), Maths::RGBA(0,0,0,80));

			//Odd bar
			if (id%2)
			{
				Render::Debug::Immediate()->Draw2dBar( Maths::HVec2(0.f, pos.Y() + barOffset.Y()), Maths::HVec2( 1024.f, m_pFontRenderer->GetScaleY() ), Maths::RGBA(0,0,0,80));
			}

    		Render::Debug::Immediate()->Draw2dBar( pos + barOffset + Maths::HVec2( start, 4.0f ), Maths::HVec2( mid - start, m_pFontRenderer->GetScaleY() - 8.f ), colour);
    		Render::Debug::Immediate()->Draw2dBar( pos + barOffset + Maths::HVec2( mid, 4.0f ), Maths::HVec2( end - mid, m_pFontRenderer->GetScaleY() - 8.0f ), colour);
			
			Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);
			
			if (g_MemoryStatsShowAsKb) mbUsed = 0;

			if(!g_MemoryStatsShowBlocks)
			{
				if(((U32)mbUsed) == 0)
				{
					usedMemString.Printf( "%dKb", kbUsed );
				}
				else
				{
					usedMemString.Printf( "%.1fMb", mbUsed );
				}
			}
			else
			{
				if(((U32)mbUsed) == 0)
				{
					usedMemString.Printf( "%dKb (+%dKb)", kbUsed, kbHeader );
				}
				else
				{
					usedMemString.Printf( "%.1fMb (+%dKb)", mbUsed, kbHeader );
				}
			}   
			m_pFontRenderer->Print( pos + barOffset + Maths::HVec2( end + 8.0f, 0.0f ), usedMemString.Chars() );
			m_pFontRenderer->Print( pos + barOffset + Maths::HVec2( -16.0f, 0.0f ), idName.Chars(), Render::Font::E_Justification_Right );
	   	
    		pos += Maths::HVec2( 0.0f, m_pFontRenderer->GetScaleY() );
	    	
    		if(g_MemoryStatsStack)
    		{
    			start = end;
    		}

			itemCount++;
		}

#ifndef PLATFORM_PS3
		MEMORYSTATUS stat;
		GlobalMemoryStatus( &stat );
		SIZE_T usedMem = g_MemoryStatsShowAsKb ? (stat.dwTotalPhys - stat.dwAvailPhys) / 1024 : (stat.dwTotalPhys - stat.dwAvailPhys);
		SIZE_T freeMem = g_MemoryStatsShowAsKb ? stat.dwTotalPhys / 1024 : stat.dwTotalPhys;
		U32 dwTotalPhys = stat.dwTotalPhys;
		U32 dwAvailPhys = stat.dwAvailPhys;

#else
		sys_memory_info_t memInfo;
		sys_memory_get_user_memory_size( &memInfo );
		U32 usedMem = g_MemoryStatsShowAsKb ? (memInfo.total_user_memory - memInfo.available_user_memory) / 1024 : (memInfo.total_user_memory - memInfo.available_user_memory);
		U32 freeMem = g_MemoryStatsShowAsKb ? memInfo.total_user_memory / 1024 : memInfo.total_user_memory;
		U32 dwTotalPhys = memInfo.total_user_memory;
		U32 dwAvailPhys = memInfo.available_user_memory;
#endif
		char temp_buffer[512];
		static int previousMemory = 0;

		sprintf(temp_buffer, "%8d / %8d USED. Diff. Last (%8d) Free(%8d)\n", usedMem, freeMem, (dwTotalPhys - dwAvailPhys) - previousMemory ,freeMem - usedMem);
		previousMemory = (dwTotalPhys - dwAvailPhys);
		m_pFontRenderer->Print( Maths::HVec2( 120.0f, 32.0f ), temp_buffer );

		//Util::C_String tempString;
		//tempString.Printf("Alloc Count %d\n",gOtherAllocCount);
		//m_pFontRenderer->Print(Print( pos + Maths::HVec2( -16.0f, 0.0f ), tempString );

	}
}
