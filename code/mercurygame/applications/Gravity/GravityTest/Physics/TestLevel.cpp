#include "StdAfx.h"
#include "SectionPhysics.h"

#pragma optimize("", off)
void C_SectionPhysics::TestLevel()
{

	using namespace Maths;

	static U32 num =0;
	static bool bInitialised = false;

	Input::C_GameInput* gameInput = Input::GetGameInput(0);

	const HVec3 center(865.500, -53.709, -1151.523);
	const HF32 sqRadius(500*500);
	if(!bInitialised)
	{

		Btl::String filename;
		filename = Btl::String::Format("game:\\levelmesh.dat");
		Sys::I_BinaryStream *pFile = Sys::C_FileSystem::Instance().Open( filename.Chars() );
		 
		S32 vertexCount;
		if(pFile)
		{

			pFile->SetEndianSwapFlag( false );
			{
				Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);

				ar  >> vertexCount;

				HVec3 a(Maths::UNDEFINED);
				HVec3 b(Maths::UNDEFINED);
				HVec3 c(Maths::UNDEFINED);

				for(S32 i=0 ;i<vertexCount; ++i)
				{
				
					ar >> a >> b >> c;
					
					const HVec3 v1(center - a);
					const HVec3 v2(center - b);
					const HVec3 v3(center - c);

					const HF32 sv1(v1|v1);
					const HF32 sv2(v2|v2);
					const HF32 sv3(v3|v3);
					if(sv1.LT(sqRadius) || sv2.LT(sqRadius) ||sv3.LT(sqRadius))
					{
						Gravity::C_ConverterTriangle tri;
						tri.vertexA = a;
						tri.vertexB = b;
						tri.vertexC = c;
						tris.AddToEnd(&tri);
					}
				}
			}
		}
		delete pFile;
		bInitialised = true;
	}


	/*static U32 blockSize = 0;
	static U32 preBlockSize = blockSize;

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonA))
	{
		preBlockSize = blockSize;
		blockSize +=10000;
	}

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonB))
	{
		blockSize = preBlockSize;
		preBlockSize -=10000;
	}

	if(blockSize > tris.m_numElements)
	{
		blockSize = tris.m_numElements;
	}

	for(U32 i=preBlockSize; i< blockSize; ++i)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleLit(tris[i].vertexA, tris[i].vertexB, tris[i].vertexC, RGBA(0, 0, 0));
	}*/

	for(U32 i=0; i< tris.m_numElements; ++i)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleLit(tris[i].vertexA, tris[i].vertexB, tris[i].vertexC, RGBA(0, 0, 0));
	}

	if(gameInput->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonB))
	{
		bInitialised = false;
	}
}
#pragma optimize("", on)