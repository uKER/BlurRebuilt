#include "MiscSharedPch.h"
#include <MISC_SHARED\DGUtil\DGUtil.h>
#include <core/Maths.h>
#include <core/containers/Vector.h>

/*
************************************************
												
	DEPENDANCY GRAPH SORTING UTILS				
												
For animation tree, constraint system, etc		
												
												
************************************************
*/
using namespace Maths;
namespace DGUtil
{

void
SortByVariableIndex(Ut::Vector<Operation>& list, int startIndex, int endIndex);

int
GetMaxVariableIndex(const Ut::Vector<DGEdge>& edgeList) 
{
	int	max=0;
	int	i;
	for (i=0; i<edgeList.Size(); i++) 
	{
		SetMax(max, edgeList[i].src);
		SetMax(max, edgeList[i].dst);
	};
	return	max;
}


int
GetMaxVariableIndex(const Ut::Vector<Operation>& opList) 
{
	int	max=0;
	int	i;
	for (i=0; i<opList.Size(); i++) 
	{
		SetMax(max, opList[i].src);
		SetMax(max, opList[i].dst);
	};
	return	max;
}



// If two consecutive operations don't overlap in  sources or dests,
// we assume we can swap them.
using namespace Ut;

bool
CanSwap(const Operation*	a, const Operation*	b) 
{
	return!(AnyOverlap(a->src, b->dst) ||
			AnyOverlap(a->dst, b->src));
};

bool
SwapIfIndependant(Operation*	ops[3])
{
	const Operation*	a,*b,*c;
	a=ops[0];
	b=ops[1];
	c=ops[2];

	bool	ab_swapable = CanSwap(a,b);
	bool	bc_swapable= CanSwap(b,c);
	if (!(ab_swapable || bc_swapable))
		return	false;

	// which way does 'b' want to go.
	// if C is dependant on B, we send it earlier
	// if B is dependant on A, we send it later.
	// of course we want to determine both options..
	if (ab_swapable) 
	{
		if (AnyOverlap(c->src, b->dst)) { // if c writes anything a wants..
			Swap(ops[0], ops[1]);
			return	true;
		}
	} else if (bc_swapable) 
	{
		if (AnyOverlap(b->src, a->dst)) { // if a writes anything b wants,
			Swap(ops[2], ops[1]);
			return	true;
		}
	}
	return	false;
};


bool
SwapIfPossible( Operation*	&a, Operation*	&b) 
{
	if (CanSwap(a,b)) 
	{
		Swap(a,b);
		return	true;
	}
	return	false;
};


// Given the current state of batches
//
void
BuildBatches(
		Ut::Vector<OperationBatch>&	batches,	
		Ut::Vector<Operation*>		ops)
{
	int iBatchStart=0;
	int	batchIndex=0;
	batches.Reserve(ops.Size());
	batches.Resize(0);

	do 
	{
		if (iBatchStart>=ops.Size())
			break;

		// Find iBatchEnd from current starting point
		int	ii;
		int	iBatchEnd;

		for (iBatchEnd=iBatchStart+1; iBatchEnd<ops.Size(); iBatchEnd++) 
		{
			bool	endsBatch=false;

			for (ii=iBatchStart; ii<iBatchEnd; ii++) 
			{
				// the first item that reads anything from the current batch ends the batch
				if (AnyOfFirstInSecond(ops[iBatchEnd]->src, ops[ii]->dst)) {
					endsBatch=true;
					break;
				}
			};
			if (endsBatch)
				break;
		};

		//LOGINFO(g_Log, "batch[%d]:%d-%d/%d\n",	batchIndex,	iBatchStart,	iBatchEnd, ops.Size());
		batches.PushBack( OperationBatch(iBatchStart, iBatchEnd));
		for (ii=iBatchStart; ii<iBatchEnd; ii++) 
		{
			ops[ii]->batchIndex=batchIndex;
		}
		batchIndex++;
		iBatchStart = iBatchEnd;
	}
	while (iBatchStart < ops.Size());
}

/*
 * Sort operation list as if it's makefile-esque
 * i.e it's a list of dependancies not imperative program steps.
 * sort the 'root' objects first, and all objects dependant on them after.
 */

/*
TODO: CLARITY: This should be templated to work on DGNode;
- it makes more sense on 'Varaible' now, i.e. we've set up
the variables from their dependancy point of view.
*/
void
SortAsDependancyGraph(
		Ut::Vector<Operation>&	list)
{
	// brute force n^3 implementation;
	// propper version later..

	Ut::Vector<Operation*>	ptrs;	// while sorting...
	InitPointerList(ptrs, list);

	int	i,j,k;
	for (i=0; i<list.Size(); i++) 
	{
		bool	swapped=false;
		k=0;
		do {
			swapped=false;
			for (j=i+1; j<ptrs.Size(); j++) 
			{
				Operation* a= ptrs[i];
				Operation* b= ptrs[j];
				// if the first item reads what the last item writes,
				// they're in the wrong order; bubble 
				if (AnyOverlap(a->src, b->dst)) {
					Swap(ptrs[j-1],ptrs[j]);
					swapped=true;
				}
			}
			k++;
			// cyclic case breaks it..
		} while (swapped && k<list.Size());
	};
	PermuteList(list, ptrs);
};

void
SortAsDependancyGraph3(Ut::Vector<Operation>& list)
{
	DGVector<int>	depth;
	depth.Resize(list.Size());
	int	i;
	depth=0;

	// Fill dependancy-depth array.
	// todo: this is still potentially n^2 - one can only properly do this algorithm with an EDGE graph.
	int	maxDepth=0;
	int	prevMaxDepth=0;
	int	maxCount=list.Size();
	do 
	{
		prevMaxDepth=maxDepth;
		for (i=0; i<list.Size(); i++) 
		{
			Operation*	op=&list[i];
			int	j,nextDepth=0x7fffffff;

			// get 'earliest' dependancy depth value read.
			for (j=0; j<op->src.Size(); j++) 
			{
				SetMin( nextDepth,depth[op->src[j]]  );
			};
			nextDepth++;

			// todo: Ut::Vector::opeator=[Ut::Vector<int>] === multiple assignment..
			for (j=0; j<op->dst.Size(); j++) 
			{
				SetMax(depth[op->dst[j]],nextDepth);
			}
			SetMax(maxDepth, nextDepth);
		}
	} while (maxDepth!=prevMaxDepth && --maxCount);
};
template<typename T>
void	SplitArray(Ut::Vector<T>& dst0/*collects flagged items*/, 
				Ut::Vector<T>& dst1, /* collects unflagged items*/
				const Ut::Vector<int> flags /*flags*/, 
				const Ut::Vector<T> src/* array to split */)
{
	int	i, count0=0;
	assert(flags.Size()==src.Size());
	for (i=0; i<src.Size(); i++) {
		if (flags[i]!=0)
			count0++;
	}
	int	size0=dst0.Size();
	int	size1=dst1.Size();
	dst0.Resize(dst0.Size() + count0);
	dst1.Resize(dst1.Size() + src.Size()-count0);

	T*	d0 = &dst0[size0];
	T*	d1 = &dst1[size1];

	for (i=0; i<src.Size(); i++) 
	{
		if (flags[src[i]]!=0)
			*d0++ = src[i];
		else
			*d1++ = src[i];
	};
}
void
SortAsDependancyGraphNew(Ut::Vector<Operation>& list)
{
	int	maxVars = GetMaxVariableIndex(list);
	int	i;

	DGVector<int> visited;
	visited.Resize(maxVars);
	for (i=0; i<maxVars; i++)
		visited[i]=0;

	DGVector<int>	sortedIndices;
	DGVector<int>	unsortedIndices;
	DGVector<int>	split;
	unsortedIndices.Resize(list.Size());
	for (i=0; i<list.Size(); i++)
		unsortedIndices[i]=i;

	sortedIndices.Reserve(list.Size());

	DGVector<int>	bkp;
	bkp.Reserve(list.Size());

	// todo: this would be much neater permuting in place
	do {
		for (i=0; i<unsortedIndices.Size(); i++) 
		{
			visited[ list[i].dst ] = 1;
		};

		for (i=0; i<unsortedIndices.Size(); i++) 
		{
			int opIndex=unsortedIndices[i];
			// if all indices of this op are zero,
			// it's flagged to go in the sorted list.
			split[i] = (visited[ list[opIndex].src ]==0)?1:0;
		}
		bkp.Resize(0);
		SplitArray(sortedIndices, bkp, split, unsortedIndices);
		unsortedIndices=bkp;
	}
	while (unsortedIndices.Size()>0);
}


void
GetVariableIndexRange(const Ut::Vector<Operation>&	list, int* pmin, int* pmax)
{
	int	min = 0x7fffffff;
	int max=0;
	int	i;
	for (i=0; i<list.Size(); i++) 
	{
		int	j;
		for (j=0; j<list[i].src.Size(); j++)
			Maths::SetMinMax(min, max, list[i].src[j]);

		for (j=0; j<list[i].dst.Size(); j++)
			Maths::SetMinMax(min, max, list[i].dst[j]);
	}
	*pmin = min; *pmax = max;
};

// 
//
//
void
TraceDependancies(
		const Ut::Vector<Operation>&	list,
		const Ut::Vector<int>			srcNodes,
		Ut::Vector<int>					touchedNodes); // todo, for audio case.

// analogously there could be an 'anti-sort by parallelism'.

void
SortBatchesByVariable(
		Ut::Vector<Operation>&	operationList, 
		Ut::Vector<OperationBatch>& batches)
{
	int	i;
	for (i=0; i<batches.Size(); i++)
		SortByVariableIndex(operationList, batches[i].first, batches[i].last);
};

void
SortForParallelism(
		Ut::Vector<Operation>&	operationList, 
		Ut::Vector<OperationBatch>& batches)
{
	int	firstIndex=0;
	int	lastIndex = operationList.Size();
	Ut::Vector<Operation*>	ops;	// while sorting...
	InitPointerList(ops, operationList);

	//
	//	TODO - [1]more advanced method based 
	//	on dependancy coloring
	//	I.E. [1] color operations with dependancy 
	//	to get genuinely unique  strings,
	//			and interleave those..
	//

	//TODO Second phase of sorting:-
	// [2] second phase of scanning not just next batch but all next batches -
	// - put items in the existing batch with the largest count of similar operation types,
	// - and other than that, put things in the earliest possible batch, 
	//	Thats for dependancies.. e.g. DMA-ing.
	// [3] within batches, sort by operation type
	// [4] sort the things with most dependancies as early as possible.

	int	i;
	bool	swapped;
	int iteration=0;
	do 
	{
		//LOGINFO(g_Log, "==========\nPass %d\n", iteration);
		BuildBatches(batches, ops);
		swapped=false;
		// check if batches can be extended  by swapping...
		for (i=0; i<(batches.Size()-1); i++) 
		{
			int	ii,jj;
			OperationBatch&	thisBatch = batches[i];
			OperationBatch&	nextBatch = batches[i+1];
			// search the next batch to find something that could be in the previous one.
			// start at first+1 as we know that one isn't :)
			bool	canSwap;
			do {
				canSwap=false;
				for (ii=nextBatch.first+1; ii<nextBatch.last; ii++) {
					canSwap=true;
					for (jj=thisBatch.first; jj<thisBatch.last; jj++) {
						// is anything of candidate (ops[ii]) written by previous batch ?
						if (AnyOfFirstInSecond(ops[ii]->src, ops[jj]->dst)) {
							canSwap=false;
							break;
						}
					}
					if (canSwap)
						break;
				}
				
				if (canSwap) {
					// found that ii was a swap - shunts batch.
					Swap(ops[ii], ops[nextBatch.first]);
					thisBatch.last++;
					nextBatch.first++;
					// TODO check if next is extended too..
					// for case [a0 b0][a1 b1 c0][c1.. -> [a0 b0 c0][b1 a1][c1... SWAPPING a1-c0 ALSO SHIFTED NEXT BATCH

					swapped=true;
					//canSwap=false;// until extend case handled, we break & rebuild..
					//break;
				}
			} while (canSwap);
		};
	
		DebugDump(ops);
	}
	while (swapped);
	PermuteList(operationList, ops);

	SortBatchesByVariable(operationList, batches);
};

void
SortByVariableIndex(Ut::Vector<Operation>& list, int startIndex, int endIndex)
{
	// TODO!
	// - sort by minimum index referenced.
};




void
DebugDump(const Ut::Vector<Operation>& list) {
	int	i;
	for (i=0; i<list.Size(); i++) {
		DebugDump(list[i]);
	}
};
void	DebugDump(const Ut::Vector<Operation*>& list) {
	int	i;
	for (i=0; i<list.Size(); i++) {
		DebugDump(*list[i]);
	}
};

void	DebugDump(const Operation& op) {
	int	j;
	Util::C_String	str;
	str+="Batch[";
	str << op.batchIndex;
	str+="] (";
	for (j=0; j<op.dst.Size(); j++) {
		str<<op.dst[j]; str+=",";
	}
	str+=")<-(";
	for (j=0; j<op.src.Size(); j++) {
		str<<op.src[j]; str+=",";
	}
	str+=")\n";
	//LOGINFO(Skeleton::g_Log, (const char*) str);
}

template<typename T>
void	Set(Ut::Vector<T>& dst, const T& src0)					{ dst.Resize(1); dst[0]=src0;}
template<typename T>
void	Set(Ut::Vector<T>& dst, const T& src0, const T& src1)	{ dst.Resize(2); dst[0]=src0;dst[1]=src1;}
template<typename T>
void	Set(Ut::Vector<T>& dst, const T& src0, const T& src1, const T& src2)				{ dst.Resize(3); dst[0]=src0;dst[1]=src1;dst[2]=src2;}
template<typename T>
void	Set(Ut::Vector<T>& dst, const T& src0, const T& src1, const T& src2, const T& src3)	{ dst.Resize(4); dst[0]=src0;dst[1]=src1;dst[2]=src2;dst[3]=src3;}
template<typename T>
void	Set(Ut::Vector<T>& dst, const T& src0, const T& src1, const T& src2, const T& src3, const T& src4)	{ dst.Resize(5); dst[0]=src0;dst[1]=src1;dst[2]=src2;dst[3]=src3;dst[4]=src4;}
template<typename T>
void	Set(Ut::Vector<T>& dst, const T& src0, const T& src1, const T& src2, const T& src3, const T& src4,const T& src5)	{ dst.Resize(6); dst[0]=src0;dst[1]=src1;dst[2]=src2;dst[3]=src3;dst[4]=src4;dst[5]=src5;}

void
TestOpScheduler() 
{
	Ut::Vector<Operation>	list;
	int	i;
	int j=0;// easier to view

	list.Resize(9);	// 3
	for (i=0; i<9; i+=3, j+=10) 
	{
		list[i].dst.PushBack(j+0);
		list[i].src.PushBack(j+0);
		list[i].src.PushBack(j+1);

		list[i+1].dst.PushBack(j+0);
		list[i+1].src.PushBack(j+0);
		list[i+1].src.PushBack(j+2);

		list[i+2].dst.PushBack(j+0);
		list[i+2].src.PushBack(j+0);
		list[i+2].src.PushBack(j+3);
	}

	DebugDump(list);
	Ut::Vector<OperationBatch>	batches;

	SortForParallelism(list,batches);

	DebugDump(list);

	// test DG sorter
	list.Resize(5);
	Set(list[0].src, 0,1,2,3,4,9);	
	Set(list[0].dst, 10,11);
	Set(list[1].src, 4,2);	
	Set(list[1].dst, 8,9);
	Set(list[2].src, 0,5);	
	Set(list[2].dst, 6,7);	
	Set(list[3].src, 2,3);	
	Set(list[3].dst, 4,5);	
	Set(list[4].src, 0,1);	
	Set(list[4].dst, 2,3);	


	SortAsDependancyGraph(list);

	DebugDump(list);
};

/*
 * Sort operands - 
 * i.e - if we've changed the order in which operands are visited,
 * then we want the operands to be visited in a cache-friendly ordering...
 * 
 */

void
SortVariables(Ut::Vector<Operation>&	op, Ut::Vector<int>& ouputOriginalIndexTranslation, int* pMaxVariablesUsed) 
{
	Ut::Vector<int>& out =ouputOriginalIndexTranslation;
	int	min,max;
	GetVariableIndexRange(op, &min,&max);
	// TODO - allow sparseness, using a hash table; this will die if its too sparse.
	out.Resize(max);
	int	i;
	for (i=0; i<max; i++) out[i] = -1;

	int	nextIndex=0;
	for (i=0; i<op.Size(); i++) 
	{
		int	 operand;
		int	j;
		for (j=0; j<op[i].src.Size(); j++) 
		{
			operand = op[i].src[j];
			if (out[operand]<0)
				out[operand]=nextIndex++;
		};
		for (j=0; j<op[i].dst.Size(); j++) 
		{
			operand = op[i].dst[j];
			if (out[operand]<0)
				out[operand]=nextIndex++;
		};
	};
	if (pMaxVariablesUsed)
		*pMaxVariablesUsed = nextIndex;
}


// convert from edge format.
// note these are unidirectional.
// 
void
BuildNodesFromEdges(Ut::Vector<Operation>& listOut, const Ut::Vector<DGEdge>& edges)
{
	int	maxIndex =GetMaxVariableIndex(edges);
	if (listOut.Size() < maxIndex)
		listOut.Resize(maxIndex);

	int	i;
	for(i=0; i<edges.Size(); i++) 
	{
		const DGEdge*	edge=&edges[i];
		AddUnique(listOut[edge->dst].src, edge->src);
		AddUnique(listOut[edge->src].dst, edge->dst);
	}
}




// "GetVariableOperandDependancies"
//
// - given a list of opeations,
// collect list of which variables affect which others,
// and which operations affect which variables.
// this can then be used for propogation ....

void
GetVariableDependancies(
	Ut::Vector<Variable>&			varList,
	const Ut::Vector<Operation>& 	opList	)
{
	int	maxIndex =GetMaxVariableIndex(opList);

	if (varList.Size() < maxIndex)
		varList.Resize(maxIndex);

	int	i;
	for (i=0; i<maxIndex; i++) {
		varList[i].src.Resize(0);
		varList[i].dst.Resize(0);
	}

	int	opIndex,j;
	for (opIndex=0; opIndex<opList.Size(); opIndex++) 
	{
		const Operation*	op=&opList[opIndex];
		// each OUTPUT is dependant on operations' INPUTS,
		for (j=0; j<op->dst.Size(); j++) 
		{
			Variable* var=&varList[op->dst[j]];
			AddUnique(var->src,			op->src);
			AddUnique(var->operations,	opIndex);
		}

		// each INPUT writes to all operations OUTPUTS
		for (j=0; j<op->src.Size(); j++) 
		{
			Variable* var=&varList[op->src[j]];
			AddUnique(var->dst,	op->dst	);
			AddUnique(var->operations, opIndex);
		}
	}
}

// TODO - dependancy tracer, "this thing is affected by..."



} // namespace DGUtil





























