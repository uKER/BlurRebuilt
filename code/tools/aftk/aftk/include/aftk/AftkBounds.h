#ifndef AFTK_BOUNDS_H
#define AFTK_BOUNDS_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	class C_Bounds
	{
	public:
		///\brief Constructor
		///
		C_Bounds();

		///\brief Copy Constructor.
		///
		///@param rhs The object to copy.
		C_Bounds(const C_Bounds& rhs);
		
		///\brief Destructor.
		///
		~C_Bounds();

		///\brief Adds a vertex and recalculates the bounds.
		///
		///@param vertex The vertex to add.
		void AddVertex(const Position3& vertex);

		///\brief Get the bounding box minimum extents.
		///
		///@return The minimum extents of this bounds object.
		const Position3& GetMin() const;
		
		///\brief Get the bounding box maximum extents.
		///
		///@return The maximum extents of this bounds object.
		const Position3& GetMax() const;

		///\brief Get the centre of this bounding box.
		///
		///@return The centre of this bounds object.
		const Position3& GetCentre() const;

		///\brief Get the radius of a sphere completely enclosing this bounding box.
		///
		///@return Bounding box sphere radius.
		F32	GetBoundingBoxRadiusSquared() const;

		///\brief Get the radius of a sphere completely enclosing this object.
		///
		///@return Object sphere radius.
		F32	GetObjectRadiusSquared() const;

		///\brief Asignment operator.
		///
		///@param rhs The bounds with which to overwrite this object.
		///@return A reference to this object.
		const C_Bounds& operator=(const C_Bounds& rhs);

		///\brief Adds a vertex and recalculates the bounds.
		///
		///@param vertex The vertex to add.
		///@return C_Bounds object with the addition made.
		C_Bounds operator +(const Position3& vertex) const;

		///\brief Adds another bounds object to this one.
		///
		///@param bounds The bounds to add.
		///@return C_Bounds object with the addition made.
		C_Bounds operator +(const C_Bounds& bounds) const;

		///\brief Adds a vertex and stores the result.
		///
		///@param vertex The vertex to add.
		///@return A reference to this bounds object.
		const C_Bounds& operator +=(const Position3& vertex);

		///\brief Adds another bounds object to this one and stores the result.
		///
		///@param bounds The bounds to add.
		///@return A reference to this bounds object.
		const C_Bounds& operator +=(const C_Bounds& bounds);

		///\brief Equivilance operator.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and rhs are equivilant. Otherwise 'false' is returned.
		bool operator== (const C_Bounds &rhs) const;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualTo(const C_Bounds &rhs, U32 flags = 0) const;

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const C_Bounds &rhs, U32 flags = 0) const;

		///Clear the contents of this object to their initial values
		void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		Position3	m_bb[2];

		Position3	m_centre;
		F32			m_objectRadiusSquared;
		F32			m_bbRadiusSquared;
	#endif
	};

	typedef Ut::UtVector< Ut::DynamicVector<C_Bounds> >	BoundsArray;
}
#endif

