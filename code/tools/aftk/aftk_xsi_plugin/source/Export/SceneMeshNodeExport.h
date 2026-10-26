#pragma once

namespace XSITrans
{

class C_SceneMeshNodeExport : public C_SceneNodeExport
{
public:
	virtual ~C_SceneMeshNodeExport();

	virtual Aftk::C_SceneNode* Export(
		Aftk::C_Scene& aftkScene,
		const C_ExportOptions& options, 
		XSI::X3DObject& xsiNode);
	
protected:
	bool _Export(
		Aftk::C_SceneMeshNode& aftkSceneMeshNode,
		const C_ExportOptions& options, 
		XSI::X3DObject& xsiNode);
};

}