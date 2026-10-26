#ifndef __IMPACT_COMPOUND_SHAPE_H__
#define __IMPACT_COMPOUND_SHAPE_H__

namespace Impact
{

/// a shape made out of multiple sub-shapes
class CompoundShape : public BaseShape
{
	friend class CompoundCollision;
public:
	/// constructor
			CompoundShape();

	/// destructor
			~CompoundShape();

	/// add a sub-shape
	void	AddShape(BaseShape *p);

	/// add an bounding test shape
	void	SetBoundingShape(BaseShape *p);

	/// get instance of shape
	Shape	*Instance();

	Ut::Vector<BaseShape*>	m_Shapes;
	BaseShape				*m_pBoundingShape;
};


/// a world-space instance of compoundshapeinstance
class CompoundShapeInstance : public Shape
{
	friend class CompoundCollision;
public:
	/// constructor
			CompoundShapeInstance();

	/// destructor
			~CompoundShapeInstance();

	/// add a sub-shape
	void	AddShape(BaseShape *p);

	/// add an bounding test shape
	void	SetBoundingShape(BaseShape *p);

	/// get bounding box
	Maths::C_AxisBox GetAABB(Body *pBody);

	Ut::Vector<Shape*>	m_Shapes;
	Shape				*m_pBoundingShape;
	Maths::C_AxisBox	m_CachedLocalBBox;
	bool				m_bGeneratedCachedBoundingBox;
};

/// a class for performing collision against a compound shape
class CompoundCollision : public I_PersistentData
{
public:
	/// constructor
		CompoundCollision();

	/// destructor
		~CompoundCollision();

	/// calculate contact points between a body and a compound shape
	int Collide(ContactVector &contacts, Body *p[2], Shape *pS[2]);

	Ut::Vector<I_PersistentData*>	m_Shapes;
	I_PersistentData*				m_pBoundingShape;
};

};

#endif
