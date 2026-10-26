#ifndef DGUTIL_H
#define DGUTIL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\core.h>
#include <core/Maths.h>
#include <core/containers/Vector.h>


/*
************************************************
												
	DEPENDANCY GRAPH SORTING UTILS				
												
For animation tree, constraint system, etc		
						
todo: clarify analogies.
so far used for sorting 'functions calls' in 
animation constraints 

the Audio case is looking more Actor-esque,
with bidirectional messages, hidden dependancies
by the sounds of it.. 												

Audio Case most effectively clarified by
creating each component's persistent state as a seperate "variable"..

class	DGNode, Operation... 
												
************************************************
*/

extern void	dbprintf(const char*, ...);	// map this to standard logger.

// Added permute interfaces to vector class.
// - move to auxillary vector helpers.. (not for general consumption..)

// can't derive it on ::Permutation... 
template<typename T>
class VectorOperators : public T
{
public:
	bool operator==(  const typename T::StructureType& src) const {
		int	i;
		for (i=0; i<src.Size(); i++) {
			if (this->T::operator[](i)!=src)
				return	false;
		}
		return	true;
	}
	bool operator!=(const typename  T::StructureType& src) const {
		int	i;
		for (i=0; i<src.Size(); i++) {
			if (this->T::operator[](i)!=src)
				return	false;
		}
		return	true;
	}
};


template<typename T>
class	DGVector : public Ut::Vector<T>
{
public:
	typedef typename Ut::Vector<T>::StructureType StructureType;

	// "Permuation" == syntactically an array, but it's derefed...
	class	Permutation //: public VectorOperators<typename DGVector<T>::Permutation>
	{
		
		Ut::Vector<T>&	src;
		Ut::Vector<int>& indices;

	public:
		typedef typename Ut::Vector<T>::StructureType StructureType;
		int	Size() const { return indices.Size(); }
		T& operator[](int index) {
			
			return src.Ut::Vector<T>::operator[](indices.Ut::Vector<T>::operator[](index));
		}
		const T& operator[](int index) const {
			return src.Ut::Vector<T>::operator[](indices.Ut::Vector<T>::operator[](index));
		}
		Permutation(Ut::Vector<T>& _src, Ut::Vector<int>& _indices) : src(_src),indices(_indices)
		{
		};
		Permutation& operator=(const T& value) 
		{
			int	i;
			for (i=0; i<indices.Ut::Vector<T>::Size(); i++) {
				this->operator[](i)=value;
			}
			return*	this;
		};
		bool operator==(const T& src) const {
			int	i;
			for (i=0; i<this->Size(); i++) 
				if (this->operator[](i)!=src)
					return	false;
			return	true;
		}
		
	};

	T&		operator[](int index) { return Ut::Vector<T>::operator[](index);}
	const T&	operator[](int index) const { return Ut::Vector<T>::operator[](index);}

	bool operator==(const T& src) const {
		int	i;
		for (i=0; i<Ut::Vector<T>::Size(); i++) 
			if (Ut::Vector<T>::operator[](i)!=src)
				return	false;
		return	true;
	}

	Permutation	operator[]( Ut::Vector<T>& indices) {
		return	Permutation(*this, indices);
	}
	const Permutation	operator[]( const Ut::Vector<T>& indices) const {
		return	Permutation(*this, indices);
	}
	void operator=(Permutation& src) 
	{
		int	i;
		Ut::Vector<T>::Resize(src.Size());
		for (i=0; i<src.Ut::Vector<T>::Size(); i++) 
		{
			this->Ut::Vector<T>::operator[](i) = src[i];
		};
	};
	DGVector<T>&	operator=(const T& src) 
	{
		int	i;
		for (i=0; i<Ut::Vector<T>::Size(); i++) {
			this->Ut::Vector<T>::operator[](i)=src;
		}
		return	*this;
	}

	class	Deref
	{
	};
};


namespace DGUtil	
{
typedef int T_OP;
typedef int VariableId;
typedef int OperationId;

// DG Node			   ( TODO: rename DGNode;)
// can represent an "FunctionCall			"
// or a "Dependancy Graph Node" (in which node indices also map to Operations).
//todo - maybe turn this whole thing into a templated lib.

// TODO: operands may need concept of operand slot name 
//		to map all info of audio case


struct	DGNode {
	DGVector<int>	src;	// indices of Operands this Reads				
	DGVector<int>	dst;	// indices of Operands this Writes;				
};

// todo: rename this 'Instruction', and try to seperate out
// DGNode, Operation, Operand..
struct	Operation : public DGNode
{
	int				opaqueID;		// opaque index preserved by these util functions
	T_OP			typeId;			// type of operation (opaque, but read in sorter)

	void			Dump();
	int				batchIndex;		// workspace - batch index.
	Operation() { opaqueID=0; typeId=0; batchIndex=0;}
};

// 'operand' - representing a 'register' or 'node'
// src's and dsts are collected from operations.

class	Variable : public DGNode
{
public:
	Ut::Vector<int>	operations;
};

struct	OperationBatch		// Batch: a range with an Operation list.
{
	int	first, last;
	OperationBatch() {};
	OperationBatch (int i0, int i1) 
	{
		first=i0; last=i1;
	}
};


struct	DGEdge 
{
	VariableId	dst,src;
	DGEdge() {};
	DGEdge(int _dst, int _src) {dst=_dst; src=_src;}
};



int	GetMaxVariableIndex(const Ut::Vector<DGEdge>& edgeList);

void
ConvertTreeToDG(Ut::Vector<Operation>&	listOut, const Operation& tree);

// TODO - convert to/from edge format.
// Sort a list of operations for maximum parallelism
// i.e. group together max number of independant 
// operations without changing dependancy order.
// assumes the OperationList is like a 'program'

void
SortForParallelism(
		Ut::Vector<Operation>&	in_OperationList, 
		Ut::Vector<OperationBatch>& out_batches);

void
BuildBatches(
		Ut::Vector<OperationBatch>&	batches,	
		Ut::Vector<Operation*>		ops);

// Sort subset of operations (e.g. within a batch) by Variable Index.
// ensures cache-coherency..

void	SortByVariableOperandIndex(
			Ut::Vector<Variable>&	in_OperationList, 
			int	startIndex,
			int	endIndex);


template<typename T>
void	
InitPointerList(
		Ut::Vector<T*>& list,
		Ut::Vector<T>& src,
		int startIndex,
		int endIndex
		) 
{
	int	i;
	list.Resize(endIndex-startIndex);
	int	iDst = 0;
	for (i=startIndex; i<endIndex; i++) 
	{
		list[iDst++] = &src[i];
	};
}

template<typename T>
void	PermuteList(Ut::Vector<T>& list, Ut::Vector<T*>& permutation)
{
	assert(permutation.Size() == list.Size());
	// permute the original list... re-order it.
	//for the minute, we just copy and re-copy..
	Ut::Vector<T>	copy;	// while sorting...
	copy.Resize(list.Size());
	int	i;
	for (i=0; i<permutation.Size(); i++) 
	{
		copy[i] = *permutation[i];
	}

	// re-write the operations.
	for (i=0; i<list.Size(); i++)
		list[i] = copy[i];
};

template<typename T>
void	
InitPointerList(
		Ut::Vector<T*>& list,
		Ut::Vector<T>& src) 
{
	InitPointerList(list,src, 0, src.Size());
};


void
SortVariables(Ut::Vector<Operation>&	op, Ut::Vector<int>& ouputOriginalIndexTranslation, int* pMaxVariablesUsed);


void
TestOpScheduler();



void
GetVaiableOperandRange(const Ut::Vector<Operation>&	list, int* pmin, int* pmax);

void
SortForParallelism(
		Ut::Vector<Operation>&	operationList, 
		Ut::Vector<OperationBatch>& batches);


/*
 *	Sort operation nodes as if they are a 
 *	Dependancy Graph,  (e.g. makefile-esque)
 *
 */

void
SortAsDependancyGraph(
		Ut::Vector<Operation>&	list);


// TODO:
//
// - given a list of src Nodes, return ordered list
// sorted from furthest to closest, that would be evaluated
// to get the current node.

void
TraceDependancies(
		const Ut::Vector<Operation>&	list,
		int								srcNodeIndex,
		Ut::Vector<int>*				touchedNodes);

void	DebugDump(const Operation& op);
void	DebugDump(const Ut::Vector<Operation*>& list);
void	DebugDump(const Ut::Vector<Operation>& list);


void
BuildNodesFromEdges(Ut::Vector<Operation>& listOut, const Ut::Vector<DGEdge>& edges);
void
BuildEdgesFromNode(Ut::Vector<DGEdge>&	edgesOut, const Ut::Vector<Operation>& dgIn);



}; // namespace DGUtil;


#define DGUTILPRINTF	//

// edge-based interface.




#include <MISC_SHARED\DGUtil\DGUtilHelpers.h>


#endif
