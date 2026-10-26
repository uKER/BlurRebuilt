#ifndef __POLYCLUSTER_H__
#define __POLYCLUSTER_H__

#ifdef _MSC_VER
#pragma once
#endif


//array nPolygons in size, storing the cluster index
typedef Ut::UtVector< Ut::DynamicVector<S32> > PolygonClusterMap;


U32 BuildPolygonClusterMap(const Aftk::C_SceneMeshNode *pSceneMesh, PolygonClusterMap& results);

#endif