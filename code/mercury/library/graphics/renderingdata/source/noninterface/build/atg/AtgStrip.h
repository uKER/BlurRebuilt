//--------------------------------------------------------------------------------------
// AtgStrip.h
//
// Tristrip routines (which convert a mesh into a list of optimized
// triangle strips).
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once
#ifndef ATGSTRIP_H
#define ATGSTRIP_H


// Stripify flags
#define ATGTRISTRIPPER_OPTIMIZE_FOR_CACHE      0x00
#define ATGTRISTRIPPER_OPTIMIZE_FOR_INDICES    0x01
#define ATGTRISTRIPPER_OUTPUT_TRISTRIP         0x00
#define ATGTRISTRIPPER_OUTPUT_TRILIST          0x02
#define ATGTRISTRIPPER_USE_RESET_INDEX         0x04  // reset index is 0xffff


//--------------------------------------------------------------------------------------
// Name: class CAtgTriStripper
// Desc: Tristrippring class to convert mesh into a list of optimized triangle strips.
//--------------------------------------------------------------------------------------
class TriStripper
{
public:
    // Main stripify routine. Stripifies a mesh and returns the number of 
    // strip indices contained in ppStripIndices. Note: Caller must make sure to
    // call delete[] on the ppStripIndices array when finished with it.
    static DWORD MakeStrips( DWORD  dwNumTriangles, // Number of triangles
                             const WORD*  pTriangles,     // Ptr to triangle indices
                             DWORD* pdwNumIndices,  // Number of output indices
                             WORD** ppStripIndices, // Output indices
                             DWORD  dwFlags = 0L ); // Flags controlling optimizer.


    // Calculate the number of cache hits and degenerate triangles
    /*static HRESULT CalcCacheHits( D3DPRIMITIVETYPE dwPrimType, DWORD dwVertexSize,
                                  WORD* pIndices, DWORD dwNumIndices,
                                  DWORD* pdwNumDegenerateTris,
                                  DWORD* pdwNumCacheHits,
                                  DWORD* pdwNumPagesCrossed );*/


    // Re-arrange vertices so that they occur in the order that they are first
    // used. Instead of actually moving vertex data around, this function returns
    // an array that specifies where (in the new vertex array) each old vertex
    // should go. It also re-maps the strip indices to use the new vertex locations.
    // Note: Caller must make sure to call delete[] on the pVertexPermutation array
    // when finished with it.
    /*static VOID ComputeVertexPermutation( DWORD  dwNumStripIndices,     // Number of strip indices
                                          WORD*  pStripIndices,         // Ptr to strip indices
                                          DWORD  dwNumVertices,         // Number of vertices in
                                          WORD** ppVertexPermutation ); // Map from original index to remapped index*/
};


#endif // ATGSTRIP_H
