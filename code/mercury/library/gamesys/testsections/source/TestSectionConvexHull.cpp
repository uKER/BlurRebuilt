
#include "stdafx.h"

#include "TestSections\TestSectionConvexHull.h"

#include "System\Misc\Random.h"

#include "Framework\Application\UserControls.h"

#include "Input\Input.h"

Util::C_Random g_GameRand;

enum E_ConvexHullTestShape
{
	E_ConvexHullTestShape_InCube = 0,
	E_ConvexHullTestShape_OnCube,
	E_ConvexHullTestShape_InSphere,
	E_ConvexHullTestShape_OnSphere,
	E_ConvexHullTestShape_File,
	E_ConvexHullTestShape_Random
};

DEBUG_ENUM(E_ConvexHullTestShape)
	DE_OPTION(E_ConvexHullTestShape_InCube, "InCube")
	DE_OPTION(E_ConvexHullTestShape_OnCube, "OnCube")
	DE_OPTION(E_ConvexHullTestShape_InSphere, "InSphere")
	DE_OPTION(E_ConvexHullTestShape_OnSphere, "OnSphere")
	DE_OPTION(E_ConvexHullTestShape_File, "File")
	DE_OPTION(E_ConvexHullTestShape_Random, "Random")
END_DEBUG_ENUM


Debug::Enum<E_ConvexHullTestShape> g_DebugConvexHullTestShape("Test.ConvexHull.TestShape", E_ConvexHullTestShape_InCube);
Debug::Int g_DebugConvexHullNoPoints("Test.ConvexHull.NoPoints", 100, Debug::Range(0, MAX_CONVEX_HULL_TEST_SOURCE_VERTICES));
Debug::Bool g_DebugConvexHullSavePoints("Test.ConvexHull.SavePoints", false);
Debug::Float g_DebugConvexHullJitter("Test.ConvexHull.Jitter", 0.0f);
Debug::Bool g_DebugConvexHullAutoMode("Test.ConvexHull.AutoMode", false);
Debug::Bool g_DebugConvexHullResetPoints("Test.ConvexHull.ResetPoints", false);
Debug::Bool g_DebugConvexHullStepBuilding("Test.ConvexHull.StepBuilding", false);
Debug::Bool g_DebugConvexHullRunAlgorithm("Test.ConvexHull.RunAlgorithm", false);

namespace Framework
{

void C_TestSectionConvexHull::WriteDebugVertexFile()
{
	if(g_DebugConvexHullSavePoints)
	{
		Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open("ConvexHullPoints.dat", Sys::E_FileAccess_Write );

		if(pFile)
		{
			Sys::BinarySerialiser<Ut::NoEndianSwap> ar( *pFile );

			ar << m_SourceVertexCount;

			for(S32 i=0; i<m_SourceVertexCount; i++)
			{
				F32 x = m_SourceVertices[i].X();
				F32 y = m_SourceVertices[i].Y();
				F32 z = m_SourceVertices[i].Z();
				ar << x;
				ar << y;
				ar << z;
			}

			SAFE_DELETE_PTR( pFile );
		}
	}
}

void C_TestSectionConvexHull::ReadDebugVertexFile()
{
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open("ConvexHullPoints.dat", Sys::E_FileAccess_Read );

	if(pFile)
	{
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( *pFile );

		ar >> m_SourceVertexCount;

		for(S32 i=0; i<m_SourceVertexCount; i++)
		{
			F32 x, y, z;

			ar >> x;
			ar >> y;
			ar >> z;

			m_SourceVertices[i] = Maths::HVec3(x, y, z);
		}

		SAFE_DELETE_PTR( pFile );
	}

}

void C_TestSectionConvexHull::Randomise()
{
	E_ConvexHullTestShape shapeType = g_DebugConvexHullTestShape;
	m_SourceVertexCount = g_DebugConvexHullNoPoints;

	if(shapeType == E_ConvexHullTestShape_Random)
	{
		shapeType = (E_ConvexHullTestShape)g_GameRand.Rand(E_ConvexHullTestShape_Random);
	}

	switch(shapeType)
	{
		case E_ConvexHullTestShape_InCube:
		for(S32 i=0; i<MAX_CONVEX_HULL_TEST_SOURCE_VERTICES; i++)
		{
			m_SourceVertices[i] = g_GameRand.CubeRand() * 2.0f - Maths::HVec3(1.0f, 1.0f, 1.0f);
		}
		break;

		case E_ConvexHullTestShape_OnCube:
		for(S32 i=0; i<MAX_CONVEX_HULL_TEST_SOURCE_VERTICES; i++)
		{
			switch(i%6)
			{
				case 0: m_SourceVertices[i] = Maths::HVec3(g_GameRand.SignedFRand(), 0.0f, g_GameRand.SignedFRand()) -  Maths::HVec3( 0.0f, -1.0f, 0.0f); break;
				case 1: m_SourceVertices[i] = Maths::HVec3(g_GameRand.SignedFRand(), 0.0f, g_GameRand.SignedFRand()) -  Maths::HVec3( 0.0f,  1.0f, 0.0f); break;
				case 2: m_SourceVertices[i] = Maths::HVec3(0.0f, g_GameRand.SignedFRand(), g_GameRand.SignedFRand()) -  Maths::HVec3(-1.0f,  0.0f, 0.0f); break;
				case 3: m_SourceVertices[i] = Maths::HVec3(0.0f, g_GameRand.SignedFRand(), g_GameRand.SignedFRand()) -  Maths::HVec3( 1.0f,  0.0f, 0.0f); break;
				case 4: m_SourceVertices[i] = Maths::HVec3(g_GameRand.SignedFRand(), g_GameRand.SignedFRand(), 0.0f ) -  Maths::HVec3( 0.0f, 0.0f, -1.0f); break;
				case 5: m_SourceVertices[i] = Maths::HVec3(g_GameRand.SignedFRand(), g_GameRand.SignedFRand(), 0.0f ) -  Maths::HVec3( 0.0f, 0.0f,  1.0f); break;
			}
			//m_SourceVertices[i] = g_GameRand.CubeRand() * 2.0f - Maths::HVec3(1.0f, 1.0f, 1.0f);
		}
		break;

		case E_ConvexHullTestShape_InSphere:
		for(S32 i=0; i<MAX_CONVEX_HULL_TEST_SOURCE_VERTICES; i++)
		{
			m_SourceVertices[i] = g_GameRand.SphereRand();
		}
		break;

		case E_ConvexHullTestShape_OnSphere:
		for(S32 i=0; i<MAX_CONVEX_HULL_TEST_SOURCE_VERTICES; i++)
		{
			m_SourceVertices[i] = g_GameRand.SphereRand();
			m_SourceVertices[i].Normalise();
		}
		break;

		case E_ConvexHullTestShape_File:
			ReadDebugVertexFile();
		break;
	}

	if( g_DebugConvexHullJitter > 0.0f )
	{
		for(S32 i=0; i<m_SourceVertexCount; i++)
		{
			m_SourceVertices[i] += g_GameRand.SignedCubeRand( g_DebugConvexHullJitter );
		}
	}
}


C_TestSectionConvexHull::C_TestSectionConvexHull( const Util::C_String& name )
: C_TestSection(name)
{
	m_pMesh = NULL;

	Randomise();
}

C_TestSectionConvexHull::~C_TestSectionConvexHull()
{
}

bool C_TestSectionConvexHull::ShouldResetPoints()
{
	if(g_DebugConvexHullResetPoints || Input::GetGameInput( 0 )->GetButtonPressedDebounced( Framework::E_Button_Debug_ButtonRightTrigger))
	{
		g_DebugConvexHullResetPoints = false;

		return true;
	}

	return false;
}


bool C_TestSectionConvexHull::ShouldStepBuilding()
{
	if(g_DebugConvexHullStepBuilding || Input::GetGameInput( 0 )->GetButtonAutoRepeat( Framework::E_Button_Debug_ButtonA, 16, 2 ))
	{
		g_DebugConvexHullStepBuilding = false;

		return true;
	}

	return false;
}

bool C_TestSectionConvexHull::ShouldRunAlgorithm()
{
	if(g_DebugConvexHullRunAlgorithm || Input::GetGameInput( 0 )->GetButtonPressedDebounced( Framework::E_Button_Debug_ButtonLeftTrigger))
	{
		g_DebugConvexHullRunAlgorithm = false;

		return true;
	}

	return false;
}

void C_TestSectionConvexHull::OnUpdate()
{
	/*
	{
		
		F32 padYRot = Input::GetGameInput( 0 )->GetAxisSignedValue( Framework::E_Axis_FlyRotateX );
		F32 padXRot = Input::GetGameInput( 0 )->GetAxisSignedValue( Framework::E_Axis_FlyRotateY );

	//Maths::HVec3 pos = m.AW();

		padYRot = padYRot * 0.05f;
		padXRot = padXRot * -0.05f;

		Maths::HMat33 yRotM = Maths::RotY(padYRot);
		Maths::HMat33 xRotM = Maths::RotX(padXRot);

		Maths::HMat43 transformM = (Maths::HMat43)(xRotM * yRotM);

		m_WorldMatrix = m_WorldMatrix * transformM;
	}
	*/

	//if(Input::GetGameInput( 0 )->GetButtonPressedDebounced( Framework::E_Button_Debug_ButtonRightStickClick ))
	//{
	//	if(m_AutoMode)
	//		m_AutoMode = false;
	//	else
	//		m_AutoMode = true;
	//}

	if(g_DebugConvexHullAutoMode)
	{
		if(!m_pMesh)
		{
			WriteDebugVertexFile();

			m_pMesh = new ConvexHull::C_Mesh();			
			m_pMesh->AddSourceVertexArray(m_SourceVertices, m_SourceVertexCount);
			m_Timeout = 10;
		}
		else
		{
			if(m_pMesh->m_BuildState == ConvexHull::E_BuildState_Finished)
			{
				m_pMesh->StepBuildingConvexHull();

				m_Timeout--;

				if(m_Timeout == 0)
				{
					delete m_pMesh;
					m_pMesh = NULL;
					Randomise();
				}
			}
			else
			{
				m_pMesh->StepBuildingConvexHull();
			}
		}
	}
	else
	{
		
		if(ShouldStepBuilding())
		{
			if(!m_pMesh)
			{			
				WriteDebugVertexFile();

				m_pMesh = new ConvexHull::C_Mesh();			
				m_pMesh->AddSourceVertexArray(m_SourceVertices, m_SourceVertexCount);
			}
			else
			{
				m_pMesh->StepBuildingConvexHull();
			}
		}

		if(ShouldResetPoints())
		{
			if(m_pMesh)
			{
				delete m_pMesh;
				m_pMesh = NULL;
			}

			Randomise();
		}

		if(ShouldRunAlgorithm())
		{
			if(!m_pMesh)
			{			
				WriteDebugVertexFile();

				m_pMesh = new ConvexHull::C_Mesh();			
				m_pMesh->AddSourceVertexArray(m_SourceVertices, m_SourceVertexCount);
			}

			m_pMesh->Build();
		}
	}
}

void C_TestSectionConvexHull::OnRender3DViewWithoutAA( const S32 viewIndex )
{
    DbUtil::I_DebugRenderPtr debugRenderPtr = DbUtil::DefaultDebugRenderSys()->Immediate();
    
    debugRenderPtr->Draw3dGrid( Maths::HMat43() );

	Maths::HMat43 renderMatrix;

	renderMatrix = m_WorldMatrix;
	renderMatrix.AW( renderMatrix.AW() + Maths::HVec3(0.0f, 1.0f, 0.0f));

	if(m_pMesh == NULL)
	{
		// draw the source vertices
		for(S32 i=0; i<m_SourceVertexCount; i++)
		{
			debugRenderPtr->Draw3dCross( m_SourceVertices[i] * renderMatrix, 0.05f, Maths::RGBA(0xff, 0xff,0xff, 0xff) );
		}
	}
	else
	{
		m_pMesh->DebugRender( debugRenderPtr, renderMatrix );
		
		Util::C_String string;
		string.Printf("Step %d", m_pMesh->m_StepNo);
		debugRenderPtr->Draw2dText( Maths::HVec2(64.0f, 96.0f), string);

		if(!m_pMesh->Closed())
		{
			debugRenderPtr->Draw2dText( Maths::HVec2(64.0f, 128.0f), "Object Not Closed");
		}

		if(m_pMesh->m_BuildState == ConvexHull::E_BuildState_Error)
		{
			debugRenderPtr->Draw2dText( Maths::HVec2(64.0f, 192.0f), "Build Error");
		}
	}
}
    

}
