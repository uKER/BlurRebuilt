#pragma once

namespace XSITrans
{

class C_SceneExport
{
public:
	C_SceneExport::C_SceneExport();

	bool Export(const C_ExportOptions& options);

	const Aftk::C_Scene& GetScene() const;

private:
	bool DoWeImportThisNode(const C_ExportOptions& options, XSI::X3DObject node) const;
	bool PreProcessScene(const C_ExportOptions& options, XSI::X3DObject node);
	bool ProcessScene(
		Aftk::C_SceneNode* pLastGoodAncestor, 
		Aftk::Matrix44* pBridgeMatrix,
		const C_ExportOptions& options, 
		XSI::X3DObject node,
		XSI::ProgressBar& bar);

	U32 m_interestingNodeCount;
	U32 m_nNodesProcessed;
	Aftk::C_Scene m_scene;
};

}