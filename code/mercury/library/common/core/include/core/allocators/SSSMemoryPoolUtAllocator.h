namespace Ut
{

/// Simple allocator using new and delete
template <class T>
class SSSMemoryPoolUtAllocator : public SSSMemoryPoolT
{
public:
	/// Allocate an object.
	static T* Allocate() 
	{
		return new T;
	}

	/// Free an object.
	static void Free(T* p)
	{
		delete(sizeof(T)) p;
	}

	template <typename U> struct Rebind {typedef SSSMemoryPoolUtAllocator<U> Other;};
};