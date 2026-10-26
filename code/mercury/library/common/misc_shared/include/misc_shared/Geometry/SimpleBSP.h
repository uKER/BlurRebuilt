#ifndef SIMPLEBSP_H
#define SIMPLEBSP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/core.h>
#include <core/Maths.h>
#include <system/debug/I_DebugRender.h>
#include <core/containers/genesis_containers/List.h>
#include <misc_shared/geometry/simplePoly.h>
#include <misc_shared/geometry/simplePoly.inl>

namespace SimpleBSP
{

// C_PolygonVertex

class C_PolygonVertex : public Util::C_Item<C_PolygonVertex>
{
public:
    C_PolygonVertex( const Maths::HVec3& pos );
    ~C_PolygonVertex();
    
public:
    Maths::HVec3                        m_Pos;
};

// C_Polygon

class C_Polygon
{
public:    
    C_Polygon();
    ~C_Polygon();
    
    void                    operator =( const C_Polygon& other );    
    
    void                    NewVertex( const Maths::HVec3& pos );
    void                    CalculatePlane();
    void                    Plane( const Maths::C_Plane& plane );
    const Maths::C_Plane    Plane() const;
    void                    Clip( const Maths::C_Plane& plane, const F32 epsilon, C_Polygon& front, C_Polygon& back );
    void                    DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr );
	void					DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::RGBA color );
	void                    DebugRenderSolid( DbUtil::I_DebugRenderPtr debugRenderPtr );

public:   
    Maths::C_Plane                      m_Plane;
    Util::C_List<C_PolygonVertex>       m_VertexList;
};

// C_Node

class C_Node : public Util::C_Item<C_Node>
{
public:
    C_Node( const Maths::C_Plane& plane );
    ~C_Node();
    void DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr );
    void DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::RGBA color );

    
public:
    Maths::C_Plane                      m_SplitPlane;

    C_Node*                             m_pParent;
    C_Node*                             m_pFront;
    C_Node*                             m_pBack;
    
	C_Polygon							m_DebugPoly;
};


class C_Tree
{
public:   
    C_Tree();
    virtual ~C_Tree();
    
    virtual C_Node*		NewNode( const Maths::C_Plane& plane );
	void                Insert(C_Polygon& polygon);
    void                Insert(C_Node& node, C_Polygon& polygon);
    C_Node*             CreateNodes(C_Polygon& polygon);
    void                DebugRender( DbUtil::I_DebugRenderPtr debugRenderPtr );
    const bool          Inside( C_Node& node, const Maths::HVec3& sphere ) const;
    const bool          Inside( const Maths::HVec3& sphere ) const;

public:        
    C_Node*                             m_pRootNode;    
    Util::C_List<C_Node>                m_NodeList;    
    
    F32                                 m_Epsilon;
};

class C_CellPlane : public Util::C_Item<C_CellPlane>
{
public:
    C_CellPlane( const Maths::C_Plane& plane );
public:
    Maths::C_Plane                  m_Plane;
};

class C_Cell : public Util::C_Item<C_Cell>
{
public:
    void                    operator =( const C_Cell& other );    
public:
    Util::C_List<C_CellPlane>       m_PlaneList;
};

class C_Containment
{
public:
    C_Containment( C_Tree& tree );    
    
    void                    BuildContainment( C_Cell& cell, C_Node& node );
    
    void                    CreateCell( C_Cell& cell );
    const bool              Inside( Maths::HVec3& pos );
    
public:
    Util::C_List<C_Cell>    m_CellList;
};

}; // namespace SimpleBSP

#endif
