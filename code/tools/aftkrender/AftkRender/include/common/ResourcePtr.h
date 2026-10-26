#ifndef AFTK_RENDER_RESOURCE_POINTER_H
#define AFTK_RENDER_RESOURCE_POINTER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	template <typename T>
	class C_ResourcePtr
	{
	public:
		class C_RefCount
		{
		public:
			C_RefCount()
				: m_Count(0)
			{ }

			~C_RefCount()
			{ }
		
			U32 operator++() { ++m_Count; return m_Count; }
			U32 operator--() { --m_Count; return m_Count; }
			operator U32() const { return m_Count; }

		private:
			U32 m_Count;
		};

		C_ResourcePtr()
			: m_pResource(0), m_pRefCount(0)
		{ }

		C_ResourcePtr(T* resource)
			: m_pResource(resource), m_pRefCount(new C_ResourcePtr<T>::C_RefCount())
		{
			Increment();
		}

		C_ResourcePtr(const C_ResourcePtr<T>& copy)
			: m_pResource(copy.m_pResource), m_pRefCount(copy.m_pRefCount)
		{
			Increment();
		}

		~C_ResourcePtr()
		{
			Decrement();
		}

		C_ResourcePtr& operator=(const C_ResourcePtr<T>& r)
		{
			Decrement();
			m_pResource = r.m_pResource;
			m_pRefCount = r.m_pRefCount;
			if (m_pRefCount)
				++(*m_pRefCount);
			return *this;
		}

		C_ResourcePtr& operator=(T* pResource)
		{
			Decrement();
			m_pResource = pResource;
			if (pResource)
			{
				m_pRefCount = new C_RefCount();
				++(*m_pRefCount);
			}
			else
				m_pRefCount = 0;
			return *this;
		}

		bool operator==(const C_ResourcePtr<T>& r) const
		{
			return m_pResource == r.m_pResource;
		}

		bool operator==(const T* r) const
		{
			return m_pResource == r;
		}

		bool operator!=(const C_ResourcePtr<T>& r) const
		{
			return m_pResource != r.m_pResource;
		}

		bool operator!=(const T* r) const
		{
			return m_pResource != r;
		}

		operator T*() { return m_pResource; }
		operator const T*() const { return m_pResource; }

		T* operator->() { return m_pResource; }
		const T* operator->() const { return m_pResource; }

		U32 Count() const { return (m_pRefCount) ? *m_pRefCount : 0; }

	private:
		void Increment() { if (m_pRefCount) ++(*m_pRefCount); }
		void Decrement()
		{ 
			if (m_pRefCount && --(*m_pRefCount) == 0) 
			{ 
				delete m_pResource; m_pResource = 0; 
				delete m_pRefCount; m_pRefCount = 0;
			}
		}

		T*			m_pResource;
		C_RefCount*	m_pRefCount;
	};
}
#endif