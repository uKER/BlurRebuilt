#pragma once

namespace XSITrans
{

class I_SceneNodeExport
{
public:
	virtual ~I_SceneNodeExport() {}

	virtual Aftk::C_SceneNode* Export(
		Aftk::C_Scene& aftkScene,
		const C_ExportOptions& options, 
		XSI::X3DObject xsiNode) = 0;
};

}
