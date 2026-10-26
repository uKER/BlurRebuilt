#ifndef CLOTHWORLD_H
#define CLOTHWORLD_H

namespace Cloth {
//-------------------------------------------------------------------------------


template <class T>
class C_WorldBase;


template <class T>
class C_Handle
{
/*	A cloth handle is essentially an encapsulated pointer to a piece of cloth. You should use C_World::CreateString/
C_World::CreateSheet to create a string/sheet and obtain a handle to it, and C_Handle::Delete to destroy the
string/sheet (which removes it from the world in the process).

The cloth handle also provides iterator functionality: you can use it to iterate through all the cloth objects of type
T in the world.
*/

public:

	// Creates a null handle.
	C_Handle(): m_pWorld(NULL), m_it(NULL) {}

	// Uses automatic copy constructor and operator=. These create a second handle pointing to the same object.
	// Uses automatic destructor: destroys the handle only. To destroy the object itself, call the Delete method.

	// Returns false if the handle is null, otherwise true.
	operator bool() const { return m_pWorld != NULL; }

	void MakeNull() {m_pWorld = NULL;}

	T & operator*() { return *m_it; }
	T* operator->() { return &(*m_it); }

	// Removes the object from its world and destroys it. This handle, along with all others that pointed to the deleted
	//	object, is invalidated (but NOT made null).
	void Delete() {	m_pWorld->m_theList.Erase(m_it); }

	void SafeDelete()
	{
		if (*this)
		{
			Delete();
			MakeNull();
		}
	}

	// Moves on to the next object. If the end is reached, the handle is made null.
	C_Handle<T> & operator++()
	{
		++m_it;
		if (m_it == m_pWorld->m_theList.End())
			m_pWorld = NULL;
		return *this;
	}


private:

	template <class R>
	friend class C_WorldBase;

	typedef typename T::List::Iterator Iterator;

	C_Handle(C_WorldBase<T>* pWorld, Iterator it): m_pWorld(pWorld), m_it(it) {}

	C_WorldBase<T>* m_pWorld;	// the handle is null iff. m_pWorld is NULL
	Iterator m_it;
};

typedef C_Handle<C_String> C_StringHandle;
typedef C_Handle<C_Sheet> C_SheetHandle;


template <class T>
class C_WorldBase
{
public:

	C_Handle<T> Create(const typename T::InitParams & initParams)
	{
		m_theList.PushFront();
		typename T::List::Iterator it = m_theList.Begin();
		it->Init(initParams);
		return C_Handle<T>(this, it);
	}


	// Returns a null handle if the world is empty.
	C_Handle<T> GetFirstObject()
	{
		if(m_theList.Size() == 0)
			return C_Handle<T>();
		else
			return C_Handle<T>( this, m_theList.Begin() );
	}


	// Invalidates all handles.
	void Clear()
	{
		m_theList.Clear();
	}


	static void Update( T & object, const Maths::C_TimeS & timeStep )
	{
		if (object.IsActive())
		{
			object.UpdateWind(timeStep.Get());

			hf maxMinorTimeStep = object.GetMaxMinorTimeStep();
			S32 nMinorTimeSteps = (S32)( ceil( timeStep.Get() / maxMinorTimeStep ) );
			hf minorTimeStep = timeStep.Get() / nMinorTimeSteps;
			for ( S32 i=0; i < nMinorTimeSteps; ++i )
				object.DoMinorUpdate( minorTimeStep );

			object.UpdateOrientations();

			if ( g_DebugDebugRender )
				object.DebugRender( DbUtil::DefaultDebugRenderSys()->Deferred() );
		}
	}


	void Update( const Maths::C_TimeS & timeStep )
	{	
		for ( typename T::List::Iterator it = m_theList.Begin(); it != m_theList.End(); ++it )
			Update(*it, timeStep);
	}


	S32 GetNumberOfObjects() const {return m_theList.Size();}


private:

	template <class R>
	friend class C_Handle;

	typename T::List m_theList;
};


class C_World : private C_WorldBase<C_String>, private C_WorldBase<C_Sheet>
{
public:

	C_StringHandle CreateString(const C_StringInitParams & initParams)
	{
		return C_WorldBase<C_String>::Create(initParams);
	}


	C_SheetHandle CreateSheet(const C_SheetInitParams & initParams)
	{
		return C_WorldBase<C_Sheet>::Create(initParams);
	}


	C_StringHandle GetFirstString()
	{
		return C_WorldBase<C_String>::GetFirstObject();
	}


	C_SheetHandle GetFirstSheet()
	{
		return C_WorldBase<C_Sheet>::GetFirstObject();
	}


	void Clear()
	{
		C_WorldBase<C_String>::Clear();
		C_WorldBase<C_Sheet>::Clear();
	}


	static void Update( C_String & object, const Maths::C_TimeS & timeStep )
	{
		C_WorldBase<C_String>::Update(object, timeStep);
	}


	static void Update( C_Sheet & object, const Maths::C_TimeS & timeStep )
	{
		C_WorldBase<C_Sheet>::Update(object, timeStep);
	}


	void Update(const Maths::C_TimeS & timeStep)
	{
		C_WorldBase<C_String>::Update(timeStep);
		C_WorldBase<C_Sheet>::Update(timeStep);
	}


	S32 GetNumberOfStrings() const {return C_WorldBase<C_String>::GetNumberOfObjects();}
	S32 GetNumberOfSheets() const {return C_WorldBase<C_Sheet>::GetNumberOfObjects();}

};


//-------------------------------------------------------------------------------
};

#endif
