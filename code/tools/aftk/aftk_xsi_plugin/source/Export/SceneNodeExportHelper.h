#pragma once

namespace XSITrans
{
class C_SceneNodeExportHelper
{
public:
	static Aftk::C_SceneNode* Export(
		Aftk::C_Scene& aftkScene,
		const C_ExportOptions& options, 
		XSI::X3DObject xsiNode);
};
}
