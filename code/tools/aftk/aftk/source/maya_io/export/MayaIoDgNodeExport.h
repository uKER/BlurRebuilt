#ifndef MAYA_IO_DG_NODE_EXPORT_H
#define MAYA_IO_DG_NODE_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{

	class C_Exporter;

	class C_DgNodeExport
	{
	public:

		static Aftk::C_Status Export(
			C_Exporter *pExporter,
			MFnDependencyNode& fnDependencyNode,
			const Aftk::C_Scene* pAftkScene,
			const Aftk::C_Node* pAftkNode);

	};
}

#endif