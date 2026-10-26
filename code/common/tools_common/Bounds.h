#ifndef TU_BOUNDS_H
#define TU_BOUNDS_H

namespace Tu
{
	__declspec(align(16)) class Bounds
	{
	public:
		///Constructor
		inline Bounds();
		
		///Destructor.
		inline ~Bounds();

		inline void AddVertex(const UtMaths::MVector3F& vertex);

		///Get the bounding box minimum extents.
		inline const UtMaths::MVector3F& GetMin() const;
		
		///Get the bounding box maximum extents.
		inline const UtMaths::MVector3F& GetMax() const;

		///Get the centre of this bounding box.
		inline const UtMaths::MVector3F& GetCentre() const;

		///Get the radius of a sphere completely enclosing this bounding box.
		inline F32	GetBoundingBoxRadiusSquared() const;

		///Get the radius of a sphere completely enclosing this object
		inline F32	GetObjectRadiusSquared() const;

		///Clear the contents of this object to their initial values
		inline void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		UtMaths::MVector3F	m_bbMin;
		UtMaths::MVector3F	m_bbMax;
		UtMaths::MVector3F	m_centre;

		F32			m_objectRadiusSquared;
		F32			m_bbRadiusSquared;
	};
	#endif

	Tu::Bounds::Bounds()
	{
		Clear();
	}

	Tu::Bounds::~Bounds()
	{
	}

	void Tu::Bounds::AddVertex(const UtMaths::MVector3F& vertex)
	{
		bool updateBB = false;
		if (vertex[0] < m_bbMin[0])
		{
			m_bbMin[0] = vertex[0];
			updateBB = true;
		}
		if (vertex[1] < m_bbMin[1])
		{
			m_bbMin[1] = vertex[1];
			updateBB = true;
		}
		if (vertex[2] < m_bbMin[2])
		{
			m_bbMin[2] = vertex[2];
			updateBB = true;
		}

		if (vertex[0] > m_bbMax[0])
		{
			m_bbMax[0] = vertex[0];
			updateBB = true;
		}
		if (vertex[1] > m_bbMax[1])
		{
			m_bbMax[1] = vertex[1];
			updateBB = true;
		}
		if (vertex[2] > m_bbMax[2])
		{
			m_bbMax[2] = vertex[2];
			updateBB = true;
		}

		if (updateBB)
		{
			//Calc bounding box centre.
			m_centre[0] = ((m_bbMax[0] - m_bbMin[0]) * 0.5f + m_bbMin[0]);
			m_centre[1] = ((m_bbMax[1] - m_bbMin[1]) * 0.5f + m_bbMin[1]);
			m_centre[2] = ((m_bbMax[2] - m_bbMin[2]) * 0.5f + m_bbMin[2]);
			{
				//Let's calculate the radius^2 of the sphere enclosing this bounding box.
				F32 dx = m_bbMax[0] - m_centre[0];
				F32 dy = m_bbMax[1] - m_centre[1];
				F32 dz = m_bbMax[2] - m_centre[2];
				m_bbRadiusSquared = dx*dx + dy*dy + dz*dz;
			}
		}

		{
			//Let's calculate the radius^2 of the sphere enclosing this object.
			F32 dx = vertex[0] - m_centre[0];
			F32 dy = vertex[1] - m_centre[1];
			F32 dz = vertex[2] - m_centre[2];
			F32 currentObjectRadiusSquared = dx*dx + dy*dy + dz*dz;

			if (currentObjectRadiusSquared > m_objectRadiusSquared)
			{
				m_objectRadiusSquared = currentObjectRadiusSquared;
			}
		}
	}

	const UtMaths::MVector3F& Tu::Bounds::GetMin() const
	{
		return m_bbMin;
	}
			
	const UtMaths::MVector3F& Tu::Bounds::GetMax() const
	{
		return m_bbMax;
	}

	const UtMaths::MVector3F& Tu::Bounds::GetCentre() const
	{
		return m_centre;
	}

	F32 Tu::Bounds::GetBoundingBoxRadiusSquared() const
	{
		return m_bbRadiusSquared;
	}

	F32 Tu::Bounds::GetObjectRadiusSquared() const
	{
		return m_objectRadiusSquared;
	}

	void Tu::Bounds::Clear()
	{
		m_bbMin = UtMaths::MVector3F(100000.0f,100000.0f,100000.0f);
		m_bbMax = UtMaths::MVector3F(-100000.0f,-100000.0f,-100000.0f);

		m_centre = UtMaths::MVector3F(0,0,0);
		m_objectRadiusSquared = 0;
		m_bbRadiusSquared = 0;
	}
}

#endif
