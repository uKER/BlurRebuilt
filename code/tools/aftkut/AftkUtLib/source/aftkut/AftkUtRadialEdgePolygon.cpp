#include "stdafx.h"

namespace AftkUt
{
	namespace RadialEdgeMesh
	{
		void C_Polygon::CalculateCentroid()
		{
			U32 nVerts = 0;
			Maths::HVec3 centroid(0.f, 0.f, 0.f);

			for (ListEdge* edgeItem = GetFirstEdge(); edgeItem; edgeItem = edgeItem->GetNext())
			{
				//return the first vertex on each edge, but allowing for reversed edges
				if (edgeItem->m_flags & LIST_ITEM_EDGE_REVERSED)
				{
					centroid += Aftk::TemplateToNightmare(edgeItem->m_data.m_pVertices[1]->m_data.m_pos);
				}
				else
				{
					centroid += Aftk::TemplateToNightmare(edgeItem->m_data.m_pVertices[0]->m_data.m_pos);
				}
				nVerts++;
			}

			m_centroid = Aftk::NightmareToTemplate(centroid*(1.0f/(F32)nVerts));
		}

		//Polygon intersection members
		void C_Polygon::GetOrderedEdgeVerts(Ut::UtVector<Ut::DynamicVector<ListVert*> >& verts) const
		{
			verts.Clear();

			for (ListEdge* edgeItem = GetFirstEdge(); edgeItem; edgeItem = edgeItem->GetNext())
			{
				//return the first vertex on each edge, but allowing for reversed edges
				if (edgeItem->m_flags & LIST_ITEM_EDGE_REVERSED)
				{
					verts.PushBack(edgeItem->m_data.m_pVertices[1]);
				}
				else
				{
					verts.PushBack(edgeItem->m_data.m_pVertices[0]);
				}
			}
		}

		void C_Polygon::GetOrderedEdgeVerts(ListVert** pVerts, U32 bufferLength) const
		{
			U32 i = 0;
			for (ListEdge* edgeItem = GetFirstEdge(); edgeItem; edgeItem = edgeItem->GetNext(), i++)
			{
				//return the first vertex on each edge, but allowing for reversed edges
				if (edgeItem->m_flags & LIST_ITEM_EDGE_REVERSED)
				{
					pVerts[i] = edgeItem->m_data.m_pVertices[1];
				}
				else
				{
					pVerts[i] = edgeItem->m_data.m_pVertices[0];
				}
				if (i==bufferLength)
				{
					LOGERROR(g_Log, "C_Polygon::GetOrderedEdgeVerts: Buffer too small, i > %d", bufferLength);
				}
			}
		}

		bool C_Polygon::IntersectsPlane(const Aftk::Position3& planePos, const Aftk::Normal3& planeNorm) const
		{
			Maths::HVec3 planePosVec = Aftk::TemplateToNightmare(planePos);
			Maths::HVec3 planeNormVec = Aftk::TemplateToNightmare(planeNorm);


			//if all points are on the same side, there's no intersection

			Ut::UtVector<Ut::DynamicVector<ListVert*> > verts;
			GetOrderedEdgeVerts(verts);
			

			int ctr = 0;
			int nVerts = m_pEdges.Size();
			assert(nVerts == verts.Size());

			for (int i=0; i<nVerts; i++)
			{
				Maths::HVec3 pointVec = Aftk::TemplateToNightmare(verts[i]->m_data.m_pos);
				Maths::HVec3 planeToPointVec = pointVec - planePosVec;

				float nDotP = planeToPointVec | planeNormVec;

				//set counter to increment when on same side as normal. Therefore, if all verts are on the
				//same side, the value of counter must be 0 or nVerts. Nice.

				if (nDotP > 0.f)
				{
					ctr++;
				}
			}
			
			if ((ctr == 0) || (ctr == nVerts))
			{
				return false;
			}

			return true;
		}

		bool C_Polygon::IsCentroidInFrontOfPlane(const C_Plane& plane) const
		{
			Maths::C_Vertex vert = Aftk::TemplateToNightmare(m_centroid);
			Maths::C_Plane pl(Maths::C_Normal(plane.m_norm[0], plane.m_norm[1], plane.m_norm[2]),
									Maths::C_Vertex(plane.m_pos[0], plane.m_pos[1], plane.m_pos[2]));
			if (pl.Distance(vert) >= 0.f)
			{	
				return true;
			}	

			return false;
		}

		E_PlaneTest C_Polygon::TestPlane(const U32 index) const
		{	
			int nVerts = m_pEdges.Size();
			U32 nFront=0, nBack=0;
			for (ListEdge* edgeItem = GetFirstEdge(); edgeItem; edgeItem = edgeItem->GetNext())
			{
				//return the first vertex on each edge, but allowing for reversed edges
				C_Vertex* vt;
				if (edgeItem->m_flags & LIST_ITEM_EDGE_REVERSED)
				{
					vt = &edgeItem->m_data.m_pVertices[1]->m_data;
				}
				else
				{
					vt = &edgeItem->m_data.m_pVertices[0]->m_data;
				}

				if (vt->IntersectCachedPlane(index))
				{
					nFront++;
					nBack++;
					continue;
				}
				if (vt->TestCachedPlane(index))
				{
					nFront++;
				}
				else
				{
					nBack++;
				}			
			}

			if (nFront == nVerts)
			{
				return E_PlaneTest_InFront;
			}
			else if (nBack == nVerts)
			{
				return E_PlaneTest_Behind;
			}

			return E_PlaneTest_Intersect;
		}

		bool C_Polygon::IsInFrontOfPlane(const C_Plane& plane) const
		{
			Ut::UtVector<Ut::DynamicVector<ListVert*> > verts;
			GetOrderedEdgeVerts(verts);
			
			int ctr = 0;
			int nVerts = m_pEdges.Size();
			assert(m_pEdges.Size() == verts.Size());
			
			Maths::C_Plane pl(Maths::C_Normal(plane.m_norm[0], plane.m_norm[1], plane.m_norm[2]),
									Maths::C_Vertex(plane.m_pos[0], plane.m_pos[1], plane.m_pos[2]));

			for (int i=0; i<nVerts; i++)
			{
				Maths::C_Vertex vert = Aftk::TemplateToNightmare(verts[i]->m_data.m_pos);

				float fDistance = pl.Distance(vert);
				if (fDistance < -g_Tolerances.PosEpsilon()) //TODO: [IW] Need to sort out the behaviour of 
				{								//boolean tests. This shouldn't be an epsilon, it
					return false;				//should be behind/intersect/in front. With an enum
				}								//instead of a bool returned. 
			}
			
			return true;
		}

		void C_Polygon::GetPolyNormal(Maths::HVec3& normal, const C_Vertex* fromVertex) const
		{
			Maths::HVec3 fromVertexPos; 
			fromVertexPos = Aftk::TemplateToNightmare(fromVertex->m_pos);
			
			Ut::UtVector<Ut::DynamicVector<ListVert*> > verts;
			GetOrderedEdgeVerts(verts);
			int pv = -1;
			int nVerts = verts.Size();
			for (int i=0; i<nVerts; i++)
			{
				if (&verts[i]->m_data == fromVertex)
				{
					pv = i;
					break;
				}
			}
			if (pv == -1)
			{
				//printf("poo");
			}
			
			Maths::HVec3 positions[2];
			
			int prevIndex = (pv+nVerts-1)%nVerts;
			int postIndex = (pv+1)%nVerts;
			
			positions[0] = Aftk::TemplateToNightmare(verts[prevIndex]->m_data.m_pos);
			positions[1] = Aftk::TemplateToNightmare(verts[postIndex]->m_data.m_pos);
			
			Maths::HVec3 vec1 = positions[0] - fromVertexPos;
			Maths::HVec3 vec2 = positions[1] - fromVertexPos;
			
			normal = vec1.Normalised().Normal() ^ vec2.Normalised().Normal();
		
		}
//#pragma optimize ("", off)
		bool C_Polygon::IsVertexInPlane(const C_Vertex* vtPoly, const C_Vertex* vt) const
		{
			Maths::HVec3 pos, polyVtPos, norm;
			GetPolyNormal(norm, vtPoly);
						
			polyVtPos = Aftk::TemplateToNightmare(vtPoly->m_pos);
			pos = Aftk::TemplateToNightmare(vt->m_pos);

			Maths::C_Normal cnorm(norm);
			Maths::C_Vertex cpos(polyVtPos);
			Maths::C_Plane cplane(cnorm, cpos);

			cplane.Distance(Maths::C_Vertex(pos));
			float displacement = cplane.Distance(Maths::C_Vertex(pos));

			if (fabs(displacement) < g_Tolerances.PosEpsilon())
			{
				return true;
			}
			
			return false;
		}
//#pragma optimize ("", on)

		bool PolygonIntersectsConvexHull(const Ut::Vector<Aftk::Position3>& positions, const Ut::Vector<C_Plane>& planes)
		{
			//Clip polygon against each plane to see if there's anything left.
			//If so return true. 
			LOGERROR(g_Log, "Radial Edge Mesh: This is not implemented properly!");
#if 0
			SimplePoly::C_Polygon poly; 

			for (int i=0; i<positions.Size(); i++)
			{
				poly.NewVertex(Aftk::TemplateToNightmare(positions[i]));
			}

			//we have our poly, now clip by planes

			SimplePoly::C_Polygon prevPoly;
			SimplePoly::C_Polygon polys[2];
			prevPoly = poly;

			for (int i=1; i<planes.Size(); i++)
			{
				Maths::C_Plane pl(Maths::C_Normal(planes[i].m_norm[0], planes[i].m_norm[1], planes[i].m_norm[2]),
								Maths::C_Vertex(planes[i].m_pos[0], planes[i].m_pos[1], planes[i].m_pos[2]));
				polys[0].m_VertexList.Clear();
				prevPoly.Clip(pl, 0.0f, polys[0], polys[1]);
				if (polys[0].m_VertexList.Size() == 0)
				{
					//no poly clipped to front of plane, therefore no intersection
					return false;
				}	
				prevPoly = polys[0];
			}
#endif
			return true;
		}


	}

}
