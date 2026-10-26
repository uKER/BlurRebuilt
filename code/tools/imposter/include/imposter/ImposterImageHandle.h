#ifndef IMPOSTER_IMPOSTERIMAGEHANDLE_H
#define IMPOSTER_IMPOSTERIMAGEHANDLE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Imposter
{
	class C_ImageHandle
	{
	private:

		class C_ImageResource
		{
		public:

			C_ImageResource(const Btl::String& fileName, Attk::C_Image *pImage)
			: m_pImage(pImage), m_fileName(fileName), m_count(0), m_dirty(false)
			{ }

			~C_ImageResource()
			{
				SAFE_DELETE_PTR(m_pImage);
			}

			//mailbox flag we can use to avoid unnecessary processing
			void SetDirty(bool flag) { m_dirty = flag; }
			bool IsDirty() const { return m_dirty; }

			Btl::String& GetFileName() { return m_fileName; }
			const Btl::String& GetFileName() const { return m_fileName; }

			Attk::C_Image* GetImage() { return m_pImage; }
			const Attk::C_Image* GetImage() const{ return m_pImage; }

			//ref count modifiers
			U32 operator++() { ++m_count; return m_count; }
			U32 operator--() { --m_count; return m_count; }
			operator U32() const { return m_count; }

		private:
			Attk::C_Image	*m_pImage;
			Btl::String		m_fileName;
			U32				m_count;
			bool			m_dirty;
		};


	public:

		static C_ImageHandle Create(const Btl::String& fileName, Attk::C_Image *pImage)
		{
			C_ImageResource *pResource = new C_ImageResource(fileName, pImage);
			return C_ImageHandle(pResource);
		}

		C_ImageHandle()
		: m_pResource(0)
		{ }

		C_ImageHandle(const C_ImageHandle& copy)
		: m_pResource(copy.m_pResource)
		{
			Increment();
		}

		C_ImageHandle(C_ImageResource *pResource)
		: m_pResource(pResource)
		{
			Increment();
		}

		~C_ImageHandle()
		{
			Decrement();
		}

		C_ImageHandle& operator=(const C_ImageHandle& r)
		{
			Decrement();
			m_pResource = r.m_pResource;
			if (m_pResource)
				++(*m_pResource);
			return *this;
		}

		C_ImageHandle& operator=(C_ImageResource *pResource)
		{
			Decrement();
			m_pResource = pResource;
			if (m_pResource)
				++(*m_pResource);
			return *this;
		}

		void SetDirty(bool flag) { if (m_pResource) m_pResource->SetDirty(flag); }
		bool IsDirty() const { return m_pResource ? m_pResource->IsDirty() : false; }

		bool Write()
		{
			if (m_pResource && m_pResource->GetImage())
				return m_pResource->GetImage()->Write(m_pResource->GetFileName().Chars());
			else
				return false;
		}

		void SetFileName(const Btl::String& fileName) { if (m_pResource) m_pResource->GetFileName() = fileName; }
		Btl::String GetFileName() const { return m_pResource ? m_pResource->GetFileName() : ""; }

		bool operator==(const C_ImageHandle& r) { return m_pResource == r.m_pResource; }
		bool operator==(const C_ImageResource *pResource) { return m_pResource == pResource; }

		bool operator!=(const C_ImageHandle& r)	{ return m_pResource != r.m_pResource; }
		bool operator!=(const C_ImageResource *pResource) { return m_pResource != pResource; }

		operator Attk::C_Image*() { return m_pResource ? m_pResource->GetImage() : 0; }
		operator const Attk::C_Image*() const { return m_pResource ? m_pResource->GetImage() : 0; }

	private:
		void Increment() { if (m_pResource) ++(*m_pResource); }
		void Decrement()
		{ 
			if (m_pResource && --(*m_pResource) == 0) 
			{ 
				SAFE_DELETE_PTR(m_pResource);
			}
		}

		C_ImageResource		*m_pResource;
	};

}

#endif
