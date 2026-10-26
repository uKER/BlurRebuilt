#ifndef CONVERTER_SHARED_SKINNING_PROCESSOR_H
#define CONVERTER_SHARED_SKINNING_PROCESSOR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk { class C_SceneMeshNode; class C_WeightEnvelope; class C_SceneNode;}

namespace SkinningProcessor
{

struct Polygon
{
	Polygon(const Aftk::C_SceneMeshNode * _pNode, int _iTriangle) : pNode(_pNode), iTriangle(_iTriangle) {};
    const Aftk::C_SceneMeshNode *pNode;
    int iTriangle;
};

//Callback function
typedef void (*SkinningProcessorCallback)(const Polygon& polygon, S32 iMaterialIndex, void* pData);

class I_SkinningProcessor
{
public:
        virtual ~I_SkinningProcessor() {}

        // insert a polygon into the scene
        virtual void Insert(const Polygon& polygon, S32 iMaterialIndex) = 0;

        // do the skinning processing
        virtual void Process() = 0;

		//Get Num of MatrixPalette
		virtual S32 GetNumMatrixPalette() = 0;
		//Get Num of Bones in a MatrixPalette
		virtual S32 GetMatrixPaletteNumBones(S32 matrixPaletteIndex) = 0;
		//Get Bone Index
		virtual S32 GetMatrixPaletteBoneIndex(S32 matrixPaletteIndex, S32 boneIndex) = 0;
		//Get Bone Name
		virtual const char * GetBoneName(S32 matrixPaletteIndex, S32 boneIndex) = 0;
        
		//Get Num Bones
		virtual S32 GetNumBones() = 0;
		//Get Bone from Index
		virtual const Aftk::C_SceneNode * GetBone(S32 boneIndex) = 0;
		
		//Get maximum number of bone palette matrices.
		virtual U32 GetMaxMatrices() const=0;
		//Get maximum number of bone influences per vertex.
		virtual U32 GetMaxVertexBlend() const=0;

		//Get MatrixPalette from polygon
		virtual S32 GetMatrixPaletteIndex(const Polygon& polygon) = 0;

		// Get blend info
		virtual bool GetVertexInfo(const Aftk::C_SceneMeshNode *pNode, U32 iVertexIndex, U32* boneIndexArray, F32* boneWeightArray ) = 0;
		virtual bool GetVertexInfo(const Polygon& polygon, U32 vertexPolygonIndex, U32* boneIndexArray, F32* boneWeightArray ) = 0;

		//Callback function to list all the added polygons (pData for aux data pass into the callback)
		virtual void PolygonListCallback(SkinningProcessorCallback pCallbackFunc, void* pData) = 0;

		virtual void CreateJointList(const Btl::Vector<const Aftk::C_SceneNode*>& joints) = 0;
		virtual S32 GetNumHardMappedBones() const = 0;
};

I_SkinningProcessor* CreateSkinningProcessor(U32 maxMatrices, U32 maxVertexBlend);
void DestroySkinningProcessor(I_SkinningProcessor* pScene);
}


#endif
