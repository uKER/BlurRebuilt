#include <nexus_app/stdafx.h>
#include <nexus_app/NexusProgressDialog.h>
#include <nexus_app/NexusSystem.h>
#include <nexus_app/ViewportDisplay.h>
#include <nexus_app/DisplayManager.h>
#include <nexus_app/ViewportParent.h>

using namespace Nexus;

#define NEXUS_2D_TEXT_HEIGHT_SKIP 20

C_Overlay2D::C_Overlay2D(C_ViewportDisplay& viewport) : m_viewport(viewport)
{
}

bool C_Overlay2D::DrawAnimationInfo(S32& heightOffset)
{
	static const S32 column1Offset = 5;
	static const S32 column2Offset = 85;
	static const Maths::HVec4 colour(0.f, 0.2745f, 0.078f, 1.0f);
	static const Maths::HVec4 colour2(0.9f, 0.9f, 0.9f, 1.0f);

	const Aftk::C_Scene* pAftkScene = 
		Nexus::g_pNexusSystem->GetCorePtr()->GetAftkScenePtr();
	if (pAftkScene->GetDuration() < 0.001f)
	{
		return true;
	}

	{
		Print(0,column1Offset,heightOffset,	"Time", colour);
		std::stringstream stream;
		stream << pAftkScene->GetCurrentTime();
		Print(0,column2Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"Duration", colour);
		std::stringstream stream;
		stream << pAftkScene->GetDuration();
		Print(0, column2Offset,	heightOffset, stream.str().c_str(), colour2);
	}
	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"Start Time", colour);
		std::stringstream stream;
		stream << pAftkScene->GetStartTime();
		Print(0, column2Offset, heightOffset, stream.str().c_str(), colour2);
	}
	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"End Time", colour);
		std::stringstream stream;
		stream << pAftkScene->GetEndTime();
		Print(0, column2Offset, heightOffset, stream.str().c_str(), colour2);
	}
	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"Frame Rate", colour);
		std::stringstream stream;
		stream << pAftkScene->GetFrameRate();
		Print(0, column2Offset, heightOffset, stream.str().c_str(), colour2);
	}
	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	return true;
}

bool C_Overlay2D::DrawCameraInfo(S32& heightOffset)
{
	static const S32 column1Offset = 5;
	static const S32 column2Offset = 65;
	static const Maths::HVec4 colour(0.f, 0.2745f, 0.078f, 1.0f);
	static const Maths::HVec4 colour2(0.9f, 0.9f, 0.9f, 1.0f);

	NexusRender::Camera* pCamera = m_viewport.GetCameraPtr();
	{
		Print(0,column1Offset,heightOffset,	"Cam Pos", colour);
		const Maths::HVec3& position = pCamera->GetPosition();
		std::stringstream ss;
		ss << position.X() << " " << position.Y() << " " << position.Z();
		Print(0,column2Offset,heightOffset,	ss.str().c_str(), colour2);
	}
	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	if (m_viewport.GetCameraPtr()->GetType() == NexusRender::Camera::CAMERA_TYPE_TARGET)
	{
		NexusRender::TargetCamera* pTargetCamera = (NexusRender::TargetCamera*)m_viewport.GetCameraPtr();
		Print(0,column1Offset,heightOffset,	"Cam Dist", colour);
		std::stringstream ss;
		ss << pTargetCamera->GetZDistance();
		Print(0, column2Offset,	heightOffset, ss.str().c_str(), colour2);
		heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	}

	return true;
}

bool C_Overlay2D::DrawMeshInfo(S32& heightOffset)
{
	static const S32 column1Offset = 5;
	static const S32 column2Offset = 55;
	static const S32 column3Offset = 120;
	static const Maths::HVec4 colour(0.f, 0.2745f, 0.078f, 1.0f);
	static const Maths::HVec4 colour2(0.9f, 0.9f, 0.9f, 1.0f);

	const Aftk::C_Scene* pAftkScene =  
		Nexus::g_pNexusSystem->GetCorePtr()->GetAftkScenePtr();
	const Nexus::Core::MeshStatistics& globalMeshStatistics = 
		Nexus::g_pNexusSystem->GetCorePtr()->GetGlobalMeshStatistics();
	const Nexus::Core::MeshStatistics& selectedMeshStatistics = 
		Nexus::g_pNexusSystem->GetCorePtr()->GetSelectedMeshStatistics();

	{
		Print(0,column1Offset,heightOffset,	"Polys", colour);
		std::stringstream stream;
		stream << globalMeshStatistics.numPolygons;
		Print(0,column2Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	{
		std::stringstream stream;
		stream << selectedMeshStatistics.numPolygons;
		Print(0,column3Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"Tris", colour);
		std::stringstream stream;
		stream << globalMeshStatistics.numTriangles;
		Print(0,column2Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	{
		std::stringstream stream;
		stream << selectedMeshStatistics.numTriangles;
		Print(0,column3Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"Poly V", colour);
		std::stringstream stream;
		stream << globalMeshStatistics.numPolygonVertices;
		Print(0,column2Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	{
		std::stringstream stream;
		stream << selectedMeshStatistics.numPolygonVertices;
		Print(0,column3Offset,heightOffset,	stream.str().c_str(), colour2);
	}

	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"Pos", colour);
		std::stringstream stream;
		stream << globalMeshStatistics.numVertexPositions;
		Print(0,column2Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	{
		std::stringstream stream;
		stream << selectedMeshStatistics.numVertexPositions;
		Print(0,column3Offset,heightOffset,	stream.str().c_str(), colour2);
	}

	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"Norm", colour);
		std::stringstream stream;
		stream << globalMeshStatistics.numVertexNormals;
		Print(0,column2Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	{
		std::stringstream stream;
		stream << selectedMeshStatistics.numVertexNormals;
		Print(0,column3Offset,heightOffset,	stream.str().c_str(), colour2);
	}

	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"UVs", colour);
		std::stringstream stream;
		stream << globalMeshStatistics.numUVs;
		Print(0,column2Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	{
		std::stringstream stream;
		stream << selectedMeshStatistics.numUVs;
		Print(0,column3Offset,heightOffset,	stream.str().c_str(), colour2);
	}

	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"Tan", colour);
		std::stringstream stream;
		stream << globalMeshStatistics.numVertexTangents;
		Print(0,column2Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	{
		std::stringstream stream;
		stream << selectedMeshStatistics.numVertexTangents;
		Print(0,column3Offset,heightOffset,	stream.str().c_str(), colour2);
	}

	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"Bin", colour);
		std::stringstream stream;
		stream << globalMeshStatistics.numVertexBinormals;
		Print(0,column2Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	{
		std::stringstream stream;
		stream << selectedMeshStatistics.numVertexBinormals;
		Print(0,column3Offset,heightOffset,	stream.str().c_str(), colour2);
	}

	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;
	{
		Print(0,column1Offset,heightOffset,	"UVSets", colour);
		std::stringstream stream;
		stream << globalMeshStatistics.highestUVSet;
		Print(0,column2Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	{
		std::stringstream stream;
		stream << selectedMeshStatistics.highestUVSet;
		Print(0,column3Offset,heightOffset,	stream.str().c_str(), colour2);
	}
	heightOffset+=NEXUS_2D_TEXT_HEIGHT_SKIP;

	return true;
}

bool C_Overlay2D::Draw()
{
	if (!g_pNexusSystem->GetCorePtr()->IsSceneLoaded())
		return true;

	S32 heightOffset = 5;

	if (g_pNexusSystem->GetState().showCameraDetails)
	{
		if (!DrawCameraInfo(heightOffset))
			return false;
	}

	if (g_pNexusSystem->GetState().showPolyCount)
	{
		if (!DrawMeshInfo(heightOffset))
			return false;
	}

	if (g_pNexusSystem->GetState().showAnimationDetails)
	{
		if (!DrawAnimationInfo(heightOffset))
			return false;
	}

	return true;
}

void C_Overlay2D::Print(
	S32 fontIndex, 
	S32 x, 
	S32 y,
	const char* text, 
	const Maths::HVec4& colour)
{
	RECT r;
	SetRect(&r, x, y, 0, 0);
	LPD3DXFONT pFont = g_pViewportParent->GetDisplayManagerPtr()->GetD3DWindowPtr()->GetFontPtr(fontIndex);
	pFont->DrawText( 
		NULL, 
		text, 
		-1, 
		&r, 
		DT_NOCLIP, 
		D3DXCOLOR( 
		colour[0], 
		colour[1],
		colour[2],
		colour[3]));
}
