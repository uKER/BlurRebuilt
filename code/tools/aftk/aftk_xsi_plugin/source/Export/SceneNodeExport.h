#pragma once

namespace XSITrans
{

class C_SceneNodeExport : public I_SceneNodeExport
{
public:
	virtual ~C_SceneNodeExport();

	virtual Aftk::C_SceneNode* Export(
		Aftk::C_Scene& aftkScene,
		const C_ExportOptions& options, 
		XSI::X3DObject xsiNode);

protected:
	bool _Export(
		Aftk::C_SceneNode& aftkSceneNode,
		const C_ExportOptions& options, 
		XSI::X3DObject xsiNode);
};

}