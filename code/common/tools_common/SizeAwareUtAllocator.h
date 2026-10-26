namespace Ut
{

	/// Simple allocator using new and delete
	template <class T>
	class SizeAwareUtAllocator
	{
	public:
		/// Allocate an object.
		static T* Allocate(int n) 
		{
			return new T[n];
		}

		/// Free an object.
		static void Free(T* p, int n)
		{
			delete[] p;
		}

		template <typename U> struct Rebind {typedef SizeAwareUtAllocator<U> Other;};
	};
}

