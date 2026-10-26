#ifndef RD_PRIMITIVE_SORTING_H
#define RD_PRIMITIVE_SORTING_H
#ifdef _MSC_VER
#pragma once
#endif

class PrimitiveSorter
{
public:

	struct State
	{
		State();

		S32 effect;
		S32 indexBuffer;
		S32 vertexDeclaration;
		S32 blendState;
		S32 baseMaterial;
		S32 instanceMaterial;
		S32 sortCode;
		
		struct Stream
		{
			S32 vertexBuffer;
			S32 material;
		};

		Btl::Vector<Stream, Btl::VectorParameters_Array<4> > streams;
	};

	void AddPrimitive(const State& state);

	void Process();

	const Btl::Vector<S32>& GetSortedIndexArray() const;

private:
	Btl::Vector<State> m_states;
	Btl::Vector<S32> m_sortedIndexArray;
};


#endif
