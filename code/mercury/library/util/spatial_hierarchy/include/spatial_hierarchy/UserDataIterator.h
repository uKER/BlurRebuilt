#ifndef __SD_USER_DATA_ITERATOR_H__
#define __SD_USER_DATA_ITERATOR_H__

namespace SD
{

/// iterate over each primitive, so that the user can associate their own data with it
/// a collision will return the global primitive index which can be used to look up the user data
/// in the user's array
class UserDataIterator
{
public:
	enum Mode
	{
		Mode_BaseInstances,
		Mode_MainTrees,
		Mode_Instances,
		Mode_Finished
	};

						/// constructor
						UserDataIterator(SpatialHierarchy *pSH);
						/// destructor
						~UserDataIterator();

						/// returns true if we've reached the end of the list
	bool				End();
						/// increment iterator
	void				operator ++();
						/// get the global index of this primitive
	int					GetPrimitiveIndex();
						/// get the face number of the index (-1 if the current node is an instance)
	int					GetFaceIndex();
						/// get the scene node containing the current primitive
	Aftk::C_SceneNode	*GetSceneNode();
						/// returns the current type of primitive
	Mode				GetMode();

private:

						/// get values for the current primitive
	void				GetValues();
						/// get values from a given primitive structure
	void				GetValuesFromTrianglePrimitive(TrianglePrim *p);
						/// check whether the iterator has reached the end of a part and advance if needed
	void				Advance();

	// internal counters
	Mode				m_mode;
	int					m_iCurTree;
	int					m_iCurPrim;

	// values that can be queried
	int					m_iCurTriangle;
	Aftk::C_SceneNode	*m_pCurSceneNode;
	int					m_iCurGlobalPrim;
	SpatialHierarchy	*m_pSH;
};

} // namespace SD

#endif
