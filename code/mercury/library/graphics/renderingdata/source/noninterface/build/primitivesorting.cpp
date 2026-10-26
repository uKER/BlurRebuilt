#include "stdafx.h"
#include "primitivesorting.h" 

//#pragma optimize("", off)

static Debug::Logger &g_Log = Debug::GetLogger("graphics.renderingdata");

PrimitiveSorter::State::State()
{
	effect = -1;
	indexBuffer = -1;
	vertexDeclaration = -1;
	blendState = -1;
	baseMaterial = -1;
	instanceMaterial = -1;
	sortCode = -1;
		
	for (int i = 0; i < streams.Size(); i++)
	{
		streams[i].vertexBuffer = -1;
		streams[i].material = -1;
	}
}

void PrimitiveSorter::AddPrimitive(const State& state)
{
	m_states.PushBack(state);
}

const Btl::Vector<S32>& PrimitiveSorter::GetSortedIndexArray() const
{
	return m_sortedIndexArray;
}

struct Cost
{
	Cost() {memset(this, 0, sizeof(Cost));} 

	float effect;
	float indexBuffer;
	float vertexDeclaration;
	float blendState;
	float baseMaterial;
	float instanceMaterial;
	float sortCode;
	float vertexBuffer;
	float material;
};

float CalculateCost(const PrimitiveSorter::State& state1, const PrimitiveSorter::State& state2, const Cost& single, Cost* totals = 0)
{
	Cost cost;

	cost.effect += float(state1.effect != state2.effect) * single.effect;
	cost.indexBuffer += float(state1.indexBuffer != state2.indexBuffer) * single.indexBuffer;
	cost.vertexDeclaration += float(state1.vertexDeclaration != state2.vertexDeclaration) * single.vertexDeclaration;
	cost.blendState += float(state1.blendState != state2.blendState) * single.blendState;
	cost.baseMaterial += float(state1.baseMaterial != state2.baseMaterial) * single.baseMaterial;
	cost.instanceMaterial += float(state1.instanceMaterial != state2.instanceMaterial) * single.instanceMaterial;
	cost.sortCode += float(state1.sortCode != state2.sortCode) * single.sortCode;

	for (int i = 0; i < state1.streams.Size(); i++)
	{
		cost.vertexBuffer += float(state1.streams[i].vertexBuffer != state2.streams[i].vertexBuffer) * single.vertexBuffer;
		cost.material += float(state1.streams[i].material != state2.streams[i].material) * single.material;
	}

	if (totals)
	{
		totals->effect += cost.effect;
		totals->indexBuffer += cost.indexBuffer;
		totals->vertexDeclaration += cost.vertexDeclaration;
		totals->blendState += cost.blendState;
		totals->baseMaterial += cost.baseMaterial;
		totals->instanceMaterial += cost.instanceMaterial;
		totals->sortCode += cost.sortCode;
		totals->vertexBuffer += cost.vertexBuffer;
		totals->material += cost.material;
	}

	float total = 0.f;

	total += cost.effect;
	total += cost.indexBuffer;
	total += cost.vertexDeclaration;
	total += cost.blendState;
	total += cost.baseMaterial;
	total += cost.instanceMaterial;
	total += cost.sortCode;
	total += cost.vertexBuffer;
	total += cost.material;

	return total;
}

void PrimitiveSorter::Process()
{
	S32 nStates = m_states.Size();

	Cost single;
	single.effect = 10.f;
	single.indexBuffer = 1.f;
	single.vertexDeclaration = 1.f;
	single.blendState = 1.f;
	single.baseMaterial = 2.f;
	single.instanceMaterial = 4.f;
	single.sortCode = 100.f;
	single.vertexBuffer = 1.f;
	single.material = 1.f;

	float total = 0.f;
	Cost totals;

	for (int i = 1; i < nStates; i++)
		total += CalculateCost(m_states[i - 1], m_states[i], single, &totals);

	int current = 0;

	Btl::Vector<S32> sortedIndexArray;
	sortedIndexArray.Resize(nStates);

	for (int i = 0; i < nStates; i++)
		sortedIndexArray[i] = i;

	m_sortedIndexArray.PushBack(sortedIndexArray[0]);
	Btl::Swap(sortedIndexArray[0], sortedIndexArray.Back());
	sortedIndexArray.PopBack();

	while (!sortedIndexArray.Empty())
	{
		float cheapest = FLT_MAX;
		int index = -1;

		for (int i = 0; i < sortedIndexArray.Size(); i++)
		{
			float cost = CalculateCost(m_states[current], m_states[sortedIndexArray[i]], single);

			if (cost < cheapest)
			{
				cheapest = cost;
				index = i;
			}
		}

		m_sortedIndexArray.PushBack(sortedIndexArray[index]);
		Btl::Swap(sortedIndexArray[index], sortedIndexArray.Back());
		sortedIndexArray.PopBack();
		current = m_sortedIndexArray.Back();
	}

	Cost newTotals;
	float newTotal = 0.f;

	for (int i = 1; i < nStates; i++)
		newTotal += CalculateCost(m_states[m_sortedIndexArray[i - 1]], m_states[m_sortedIndexArray[i]], single, &newTotals);

	//LOGDEBUG(g_Log, "old %0.2f, sort %0.2f", total, newTotal);

	int breakhere = 0;
}



