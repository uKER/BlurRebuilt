#ifndef MAYA_IO_SCENE_MESH_IMPORT_H
#define MAYA_IO_SCENE_MESH_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Importer;

	class C_SceneMeshImport : public C_SceneNodeImport
	{
	public:
		///
		///Creates an Aftk::C_SceneMeshNode for a given Maya mesh.
		///@return success
		static Aftk::C_Status Import(	
			C_Importer* pImporter,
			const MDagPath& transformDagPath,
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode,
			const C_ImportOptions* pOptions);

	private:
		struct C_PolySetMembership
		{
			S32 setIndices[MAYA_IO_MAX_SETS_PER_POLY];
			U32 nSets;
		};

		static S32 FindAftkUVSetIndex(
			const MFnMesh& fnMesh, 
			S32 mayaUVSetIndex,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
			Aftk::C_Status& status);

		#ifdef AFTK_MAYA_IO_7_0
		static Aftk::C_Status ImportPolygonTangentSpace(
			Aftk::C_Polygon& aftkPoly,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
			const C_ImportOptions* pOptions,
			const MFnMesh& fnMesh,
			MItMeshPolygon* pMayaPolyIter,
			Ut::UtDynamicVector<U32>* pPolyMapping);
		#endif

		///
		///Creates an AFTK polygon from a Maya polygon. Calls ExportNGon and ExportTriangles as appropriate.
		///
		///@param	pMayaPolyIter source polygon.
		///@param	aftkPolygon destination polygon.
		///@return	Maya status indicator MStatus.
		static Aftk::C_Status ImportPolygon(
			C_Importer* pImporter,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
			const C_ImportOptions* pOptions,
			const MDagPath& shapeDagPath,
			const MFnMesh& fnMesh,
			MItMeshPolygon* pMayaPolyIter,
			Ut::UtDynamicVector<U32>* pPolyMapping,
			MObjectArray& polySetList,
			MObjectArray& polySetComponentsArray,
			const Ut::UtVector<Ut::DynamicVector<S32> >& localToScenePolySetIndexMapping);

		///
		///Creates an AFTK polygon from a Maya triangle.
		///
		///@param	pMayaPolyIter source polygon.
		///@return	Maya status indicator MStatus.
		static Aftk::C_Status ImportTriangles(
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
			const C_ImportOptions* pOptions,
			const MDagPath& shapeDagPath,
			const MFnMesh& fnMesh,
			MItMeshPolygon* pMayaPolyIter,
			Ut::UtDynamicVector<U32>* pPolyMapping);

		///
		///Creates an AFTK polygon from an Maya n-gon.
		///
		///@param	pMayaPolyIter source polygon.
		///@return	Maya status indicator MStatus.
		static Aftk::C_Status ImportNGon(
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
			const C_ImportOptions* pOptions,
			const MDagPath& shapeDagPath,
			const MFnMesh& fnMesh,
			MItMeshPolygon* pMayaPolyIter,
			Ut::UtDynamicVector<U32>* pPolyMapping);

		///
		///Create and apply AFTK materials for this mesh as necessary
		///
		///@param	source mesh node
		///@param	destination mesh node
		///@param	pAftkScene destination scene
		///@return	Maya status indicator MStatus.
		static Aftk::C_Status ImportMaterials(	
			C_Importer* pImporter,
			const MDagPath& shapeDagPath,
			const MFnMesh& fnMesh,
			S32 instanceNumber,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
			Aftk::C_Scene* pAftkScene,
			const C_ImportOptions* pOptions,
			Ut::UtDynamicVector<U32>* pPolyMapping);

		///
		///Returns the first token in a space delimited string
		///
		///@param input String to acquire token from.
		///@return The first space delimited token in the input string.
		static std::string GetFirstToken(const std::string &input);

		///
		///This function convertes mesh-relative vertex indices into face-relative vertex indices.
		///Face relative vertex indices are required by the functions MItMeshPolygon::normalIndex() 
		///and ::getNormal().
		///
		///@param	getVertices Array of mesh-relative vertex indices for entire face.
		///@param	getTriangle Array of mesh-relative vertex indices for local triangle in face
		///@return	Array of face-relative indicies for the specified vertices.
		///			Number of elements in returned array == number in getTriangle
		///			(should be 3).
		///
		///	Note:	If getTriangle array does not include a corresponding vertex
		///			in getVertices array then a value of (-1) will be inserted
		///			in that position within the returned array.
		///
		static MIntArray GetLocalIndex(
			const MIntArray &getVertices, 
			const MIntArray &getTriangle,
			Aftk::C_Status& status);

		static Aftk::C_Status BuildPolySetInfo(
			C_Importer* pImporter,
			const MDagPath& shapeDagPath,
			const MFnMesh& fnMesh,
			MObjectArray& polySetList,
			MObjectArray& polySetComponentsArray,
			Ut::UtVector<Ut::DynamicVector<S32> >& localToScenePolySetIndexMapping);

		static Aftk::C_Status ImportSetsAsBlindData(	
			C_Importer* pImporter,
			const MDagPath& shapeDagPath,
			const MFnMesh& fnMesh,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
			Aftk::C_Scene* pAftkScene,
			const C_ImportOptions* pOptions,
			Ut::UtDynamicVector<U32>* pPolyMapping,
			const MObjectArray& polySetList,
			const MObjectArray& polySetComponentsArray,
			const Ut::UtVector<Ut::DynamicVector<S32> >& localToScenePolySetIndexMapping);

		static Aftk::C_Status ImportBlindDataPolyFlags(
			C_Importer* pImporter,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
			Aftk::C_Scene* pAftkScene,
			const C_ImportOptions* pOptions,
			const MFnMesh& fnMesh,
			Ut::UtDynamicVector<U32>* pPolyMapping,
			U32 polyIndex);
	};
}

#endif
