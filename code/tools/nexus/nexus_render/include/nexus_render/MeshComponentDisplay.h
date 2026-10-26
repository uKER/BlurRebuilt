#ifndef GMESH_COMPONENT_DISPLAY_H
#define GMESH_COMPONENT_DISPLAY_H

namespace NexusRender
{
	struct VertexBuffer;

	class C_DataPerLOD
	{
	public:
		C_DataPerLOD()
		{
		}

		~C_DataPerLOD()
		{
		}

		C_DataPerLOD(const C_DataPerLOD& rhs)
		{
			*this = rhs;
		}

		const C_DataPerLOD& operator =(const C_DataPerLOD& rhs)
		{
			normalVB = rhs.normalVB;
			tangentSpaceVBs = rhs.tangentSpaceVBs;
			selectedFaceIndices = rhs.selectedFaceIndices;
			selectedPointIndices = rhs.selectedPointIndices;
			selectedFaceVB = rhs.selectedFaceVB;
			selectedPointVB = rhs.selectedPointVB;

			return *this;
		}

		VBWrapper normalVB;
		Ut::UtVector<Ut::DynamicVector<VBWrapper> > tangentSpaceVBs;
		Ut::UtVector<Ut::DynamicVector<U32> > selectedFaceIndices;
		Ut::UtVector<Ut::DynamicVector<U32> > selectedPointIndices;
		VBWrapper selectedFaceVB;
		VBWrapper selectedPointVB;
	};

	class MeshComponentDisplay
	{
	public:
		MeshComponentDisplay();

		~MeshComponentDisplay();

		void DrawNormals(U32 lod);

		void DrawTangentSpace(U32 lod, U16 set);

		void UpdateNormals(U32 lod, bool evaluate);

		void UpdateTangentSpace(U32 lod, U16 set, bool evaluate);

		void MeshComponentDisplay::UpdateFaces(bool evaluate);

		void UpdateFaces(U32 lod, bool evaluate);

		void UpdatePoints(U32 lod, bool evaluate);

		void Update(bool evaluate);

		void SelectFaces(const Ut::UtSet<Nexus::C_ComponentSelection>& selection);

		void SelectPoints(const Ut::UtSet<Nexus::C_ComponentSelection>& selection);

		void DrawSelectedFaces(U32 lod);

		void DrawSelectedPoints(U32 lod);

		Tu::Status Create(IRenderMeshNode* pMesh);

		void Destroy();

	private:
		Tu::Status CreateNormalVertexBuffer(U32 lod, U32 nPVsUsed);

		Tu::Status CreateTangentSpaceVertexBuffers(U32 lod, U32 nPVsUsed);

		Tu::Status CreateFaceVertexBuffer(U32 lod);

		Tu::Status CreatePointVertexBuffer(U32 lod);

		Aftk::I_SceneMeshNode* m_pAftkISceneMeshNode;

		IRenderMeshNode* m_pMesh;

		Ut::UtVector<Ut::DynamicVector<C_DataPerLOD> > m_dataPerLOD;
	};
};

#endif
