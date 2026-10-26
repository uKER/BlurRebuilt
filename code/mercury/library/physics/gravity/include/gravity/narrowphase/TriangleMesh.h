#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

namespace CollisionDetectionEngine
{
class TriangleMesh
{
public:
	TriangleMesh(Maths::HVec3* vertice, U32* indice): m_pVertice(vertice), m_pIndice(indice)
	{}

	~TriangleMesh()
	{
		if(m_pVertice)
		{
			delete [] m_pVertice;
		}

		if(m_pIndice)
		{
			delete[] m_pIndice;
		}

		m_pVertice=NULL;
		m_pIndice=NULL;
	}

	void SetVerticeSize(U32 size);
	void SetIndiceSize(U32 size);
	void RemoveDuplicatedVertice();
	void RemoveDegeneratedTriangle();
	Maths::HVec3* GetVertice();
	U32* GetIndice();

private:
	Maths::HVec3* m_pVertice;
	U32* m_pIndice;
	U32 m_sizeVertice;
	U32 m_sizeIndice;


};
}

#endif
