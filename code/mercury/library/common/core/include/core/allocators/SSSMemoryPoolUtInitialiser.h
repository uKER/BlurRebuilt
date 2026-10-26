#ifndef SSS_MEMORY_POOL_UT_INITIALISER_H
#define SSS_MEMORY_POOL_UT_INITIALISER_H

namespace Ut 
{
	// Does nothing
	template <class T, class StoredType>
	class SSSMemoryPoolReferenceInitialiser
	{
	public:
		StoredType& Initialise(T& item) 
		{
			return item;
		}

		void Release(StoredType* item) 
		{
			assert(item);
		}
	};

	// Copies the elements
	template <class T, class StoredType>
	class SSSMemoryPoolCopyInitialiser
	{
	public:
		StoredType& Initialise(T& item) 
		{
//			StoredType* p = Allocate();
//			assert(p);
//			p->m_element = item;
//			return *p;
			return item;
		}
		void Release(StoredType* item)
		{
//			Free(item);
		}
	};
};

#endif
