#ifndef ANIMATION_CYCLEMAPPING_H
#define ANIMATION_CYCLEMAPPING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/common/StringId.h>

namespace Animation
{
	class C_Cycle;
	class C_PoseLayout;

	/**
	
	 A mapping from cycle channels to pose field indices. This allows
	 animated data from cycles to be applied to arbitrary skeletons.
	
	 @warning C_CycleMapping objects use a customised allocation method and cannot
	 be safely allocated on the stack! Use C_CycleMapping::operator new instead.

	 The memory used by C_CycleMappings is managed using reference counting (@see AddRef, DecRef methods).
	 Referenced cycle mappings always stay in memory.
	 Unreferenced (i.e. zero reference count) cycle mappings are added to a cache 
	 and deleted on an LRU basis in order to minimise the overhead of creating new cycle mappings.

	 @note Could be generalised: e.g. a mapping from a skinned model to a pose layout as well?
	 
	*/	
	class ALIGN16 C_CycleMapping
	{
	public:

#ifndef PLATFORM_SPU	
		/// Construct a mapping between a cycle and a pose layout. PPU-only.
		C_CycleMapping( const C_Cycle& cycle, const C_PoseLayout& poseLayout );
		
		~C_CycleMapping();
#endif

		/// Increment reference count.
		void AddRef() const;

		/// Decrement reference count, potentially deleting the C_CycleMapping if it reaches zero.
		void DecRef() const;

		/// Invalidate the cycle mapping after either its C_PoseLayout or C_Cycle have been destroyed.
		void Invalidate();

		bool IsValid() const		{ return m_cycle != 0 && m_poseLayout != 0; }

		/// Get a pose layout field index from a cycle's field index.
		S32 GetPoseFieldIndex( U32 cycleFieldIndex ) const;
		
		/// Get pointer to pose layout.
		const C_PoseLayout* GetPoseLayout() const;
		
		/// Returns pointer to cycle.
		const C_Cycle* GetCycle() const;
				
		//
		// Cycle/PoseLayout lists.
		//

		/// Unlink from C_Cycle and C_PoseLayout lists.
		void Unlink();
		
		/// Link into C_Cycle and C_PoseLayout lists.
		void Link();
		
		/// Get next C_CycleMapping with the same C_Cycle.
		C_CycleMapping* GetNextByCycle() const;
		
		/// Get next C_CycleMapping with the same C_PoseLayout.
		C_CycleMapping* GetNextByPoseLayout() const;
		
		/// Get total size of mapping in bytes. DMA this number of bytes to transfer
		/// the C_CycleMapping class and its associated variable-sized data.
		U32 GetSize() const;

#ifndef PLATFORM_SPU
		/// Operator new for cycle mappings. The number of cycle fields is required
		/// to allocate the correct amount of memory for the variable-sized mapping array. PPU-only.
		static void* operator new ( size_t size, U32 numCycleFields );

		/// Delete operator. PPU-only.
		static void operator delete ( void* ptr, U32 numCycleFields );
		
		/// Delete operator. PPU-only.
		static void operator delete ( void* ptr );
#endif
	
	private:

		// Intentionally unimplemented, do not use.
		static void* operator new ( size_t );
		
		/// Calculate size of C_CycleMapping given the number of cycle fields.
		static U32 CalculateSize( U32 numCycleFields );
	
		/// Get number of cycle fields.
		U32 GetNumCycleFields() const;

		const C_Cycle*					m_cycle;
		const C_PoseLayout*				m_poseLayout;
		C_CycleMapping*					m_prevByCycle;
		C_CycleMapping*					m_nextByCycle;
		C_CycleMapping*					m_prevByPoseLayout;
		C_CycleMapping*					m_nextByPoseLayout;
		mutable const C_CycleMapping*	m_prevInCache;
		mutable const C_CycleMapping*	m_nextInCache;
		mutable S32						m_refCount;
		U16								m_numCycleFields;
		S16								m_poseFieldIndices[ 1 ]; ///< Variable-sized array mapping from cycle field indices to pose indices.

#ifndef PLATFORM_SPU
		class C_Cache
		{
		public:
			C_Cache(U32 memoryLimit);
			~C_Cache();

			void Remove(const C_CycleMapping* mapping);
			void Insert(const C_CycleMapping* mapping);
			
			void MakeSpace(U32 requiredSize);

			U32 GetMemoryUsed() const	{ return m_memoryUsed; }
			U32 GetMemoryLimit() const	{ return m_memoryLimit; }
			U32 GetNumEntries() const	{ return m_numEntries; }
		private:
			bool IsEmpty() const;
			void PurgeFront();
			const C_CycleMapping* PopFront();

			U32						m_memoryUsed;
			U32						m_memoryLimit;		// Max. memory to be used by cache.
			U32						m_numEntries;
			const C_CycleMapping*	m_front;
			const C_CycleMapping*	m_back;
		};

		static C_Cache			s_cache;		// Cache of unreferenced cycle mappings.
#endif
	};
}

#include "CycleMapping.inl"

#endif
