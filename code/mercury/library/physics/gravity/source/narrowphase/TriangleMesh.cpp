#include <stdafx.h>
#include "gravity/narrowphase/TriangleMesh.h"

//using namespace Maths;

namespace CollisionDetectionEngine
{

void TriangleMesh::SetVerticeSize(U32 size)
{
	m_sizeVertice=size;
}

void TriangleMesh::SetIndiceSize(U32 size)
{
	m_sizeIndice=size;
}

void TriangleMesh::RemoveDuplicatedVertice()
{
	U32* vertsIndex=new U32[m_sizeVertice];
	Maths::HVec3* temp=new Maths::HVec3[m_sizeVertice];
	U32 size=0;

	for(U32 i=0; i<m_sizeVertice; ++i)
	{
		vertsIndex[i] = i;
		for(U32 j=0; j<size; ++j)
		{
			if(m_pVertice[i].All_EQ(temp[j]))
			{
				vertsIndex[i] = j;
				break;
				
			}
		}

		if(vertsIndex[i] == i)
		{
			temp[size++] = m_pVertice[i];
		}
	}

	for(U32 i = 0; i < this->m_sizeIndice; ++i)
	{
		this->m_pIndice[i] = vertsIndex[m_pIndice[i]];
	}

	delete [] m_pVertice;
	delete [] vertsIndex;

	m_pVertice=temp;
	m_sizeVertice=size;
}

void TriangleMesh::RemoveDegeneratedTriangle()
{
	using namespace Maths;

	U32* newIndice=new U32[m_sizeIndice];
	U32 size=0;
	
	for(U32 i=0; i<m_sizeIndice; i+=3)
	{
		HVec3& a=m_pVertice[m_pIndice[i]];
		HVec3& b=m_pVertice[m_pIndice[i+1]];
		HVec3& c=m_pVertice[m_pIndice[i+2]];

		const HVec3 ab(b-a);
		const HVec3 ac(c-a);
		const HF32 res(ab|ac);
		const HF32 epi(Constants<HF32>::Epsilon2());

		//The triangle isn't degenerated, remove it from the mesh
		if(Abs(res) > epi)
		{
			newIndice[size++]=m_pIndice[i];
			newIndice[size++]=m_pIndice[i+1];
			newIndice[size++]=m_pIndice[i+2];
		}
	}

	if(m_pIndice)
	{
		delete [] m_pIndice;
	}

	m_pIndice=newIndice;
	m_sizeIndice=size;

}

Maths::HVec3* TriangleMesh::GetVertice()
{
	return m_pVertice;
}

U32* TriangleMesh::GetIndice()
{
	return m_pIndice;
}

};
