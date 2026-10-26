#ifndef ADVERTRESOURCE_H
#define ADVERTRESOURCE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Advertising
{
	struct AdvertResource
	{
		struct AdvertQuad
		{
			MathsStorage::Float3	v[4];
			U16						placementIndex;
			U16						routeMask;

			template <typename Archive>
			void Serialise(Archive & ar);

			template <typename Archive>
			void Deserialise(Archive & ar);
		};

		template <typename Archive>
		void Serialise(Archive & ar);

		template <typename Archive>
		void Deserialise(Archive & ar);


		typedef Btl::Vector<AdvertQuad> AdvertQuadArray;

		AdvertQuadArray	quadArray;

		static const U32 VERSION = 1;
	};

	template<typename Archive>
	void AdvertResource::AdvertQuad::Serialise(Archive& out)
	{
		out << v[0].x << v[0].y << v[0].z;
		out << v[1].x << v[1].y << v[1].z;
		out << v[2].x << v[2].y << v[2].z;
		out << v[3].x << v[3].y << v[3].z;
		out << placementIndex;
		out << routeMask;
	}

	template<typename Archive>
	void AdvertResource::AdvertQuad::Deserialise(Archive& in)
	{
		in >> v[0].x >> v[0].y >> v[0].z;
		in >> v[1].x >> v[1].y >> v[1].z;
		in >> v[2].x >> v[2].y >> v[2].z;
		in >> v[3].x >> v[3].y >> v[3].z;
		in >> placementIndex;
		in >> routeMask;
	}

	
	template<typename Archive>
	void AdvertResource::Serialise(Archive& out)
	{
		out << AdvertResource::VERSION;
		out << (U32)quadArray.Size();
		for (U32 i = 0; i < (U32)quadArray.Size(); i++)
			out << quadArray[i];
	}

	template<typename Archive>
	void AdvertResource::Deserialise(Archive& in)
	{
		U32 version;
		in >> version;
		BIZ_ASSERT(version == AdvertResource::VERSION);

		U32 quadArraySize;
		in >> quadArraySize;
		
		U32 oldArraySize = (U32)quadArray.Size();
		U32 totalArraySize = oldArraySize + quadArraySize;
		
		if (totalArraySize)
		{
			quadArray.Resize(totalArraySize);
			for (U32 i = oldArraySize; i < (U32)quadArray.Size(); i++)
				in >> quadArray[i];
		}
	}
}

#endif