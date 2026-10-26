#ifndef SR_BOUNDS_H
#define SR_BOUNDS_H

namespace LPVS
{
	class Bounds
	{
	public:
		///Constructor
		Bounds();
		
		///Destructor.
		~Bounds();

		void AddVertex(const Position3& vertex);

		///Get the bounding box minimum extents.
		const Vector3F32& GetMin() const;
		
		///Get the bounding box maximum extents.
		const Vector3F32& GetMax() const;

		///Get the centre of this bounding box.
		const Position3& GetCentre() const;

		///Get the radius of a sphere completely enclosing this bounding box.
		F32	GetBoundingBoxRadiusSquared() const;

		///Get the radius of a sphere completely enclosing this object
		F32	GetObjectRadiusSquared() const;

		///Clear the contents of this object to their initial values
		void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		void Reset();

		Vector3F32	m_bb[2];

		Position3	m_centre;
		F32			m_objectRadiusSquared;
		F32			m_bbRadiusSquared;
	};
	#endif
}

#endif
