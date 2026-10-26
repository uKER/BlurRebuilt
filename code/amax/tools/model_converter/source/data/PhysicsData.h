#ifndef APP_PHYSICS_DATA_H
#define APP_PHYSICS_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <gravity\treebuilder\AABBTree.h>
#include <gravity\narrowphase\Shape.h>
#include <gravity\exporter\GravityExporter.h>

#include <converter_shared\config\PolyflagConfig.h>
#include <converter_shared\PhysicsDataConverter.h>

//namespace Gravity
//{

//// See note in C_Converter constructor
//class C_PolyInform
//{
//public:
//	virtual U32 GetCollisionFlags(Aftk::C_Polygon& poly) = 0;
//	virtual U32 GetMaterial(Aftk::C_Polygon& poly) = 0;
//	virtual U32 GetUserData(Aftk::C_Polygon& poly) = 0;
//};
//
//typedef Btl::Vector< const Aftk::C_SceneNode* > C_AftkSceneNodeList;
//
//// Note, this cant really be a singleton any more if we are going to need to export multiple scenes so we cant have a member Instance()
//class C_Converter
//{
//public:
//	// I would prefer the virtual class way of setting the polyflag functions to the function pointer method as it means I do not need to provide global data.
//	// I dont mind how it is set. Either through the constructor or via a function.
//	C_Converter( C_PolyInform* pPolyInform ) { /* dummy call */ }
//	void SetPolyInform( C_PolyInform* pPolyInform ) { /* dummy call */ }
//
//	// The nodes in this list may be mesh or physics shapes and may be aftk instance nodes in which case they should be instanced. 
//	// This function would generally be only be called once per C_Converter with the full list of static nodes to process.
//	// For the route functionality, we will have several C_Converters.
//	void AddStaticNodes( const C_AftkSceneNodeList& aftkSceneNodeList ) { /* dummy call */ }
//
//	// This will export a compound shape by gluing together all the aftk scene nodes provided which may be mesh or physics shapes. 
//	// This will not actually create a physics body in the game, just the shape.
//	// If this list contains instance nodes, the instances can be expanded out.
//	// The matrix provided is a world space matrix which will be orthonormal. 
//	// This will be the world space body position when the object is created in game so the shape components will need to be transformed by the inverse of this.
//	U32 AddShape( const C_AftkSceneNodeList& aftkSceneNodeList, const Maths::HMat43& originInWorldSpace ) { /* dummy call */ static U32 uniqueId=0; return uniqueId++; }
//
//	// There needs to be a version that takes a binary stream rather than a filename
//	bool Export( Sys::I_BinaryStream* pFile ) { return true; }
//};
//
//} // namespace Gravity


class C_DataPhysicsData : public Tools::ProcessGraph::C_Data
{
public:
	C_DataPhysicsData();
	~C_DataPhysicsData();

	// inherited from C_Data
	const Util::C_String GetName() const;

	Gravity::C_Converter* NewExportScene();

	Gravity::C_Converter* GetExportScene( const S32 index );

	const Gravity::C_Converter* GetExportScene( const S32 index ) const;

	S32 GetExportSceneCount() const;

	void SetPolyInform( Gravity::C_PolyInform* pPolyInform );

	bool Export( Sys::I_File* pFile ) const;

private:
	Gravity::C_PolyInform* m_pPolyInform;
	Btl::Vector< Gravity::C_Converter* > m_ExportSceneList;
};

#endif

