/////////////////////////////////////////////////////////////
// VehicleActor
/////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Game/GameActors/Vehicles/VehicleActorSPU.h>

namespace Game {
namespace Actors {

void VehicleActorDeferredUpdateSPU( Scheduler::Task* pTask )
{
    using namespace Maths;
    using namespace Spu;
	PERF_EVENT( "VehicleActorDeferredUpdateSPU" );
	DeferredVehicleUpdateTaskData* pTaskData = (DeferredVehicleUpdateTaskData*)pTask->ParamData;
	
	PPU_Ptr< Vehicle::C_PartList >       pPartList  = pTaskData->m_pPartList;
    PPU_Ptr< ::Actors::C_VisibleDetect > pVisDetect = pTaskData->m_pVisDetect;
    
    DMA< HMat43 > matrix;
    DMA< HMat43 > rawMatrix;
    //matrix.Fetch( pTaskData->m_pMatrix );
    //rawMatrix.Fetch( pTaskData->m_pRawMatrix );
    
    PPU_Ptr< HMat43 > pTestMatrix = pTaskData->m_pMatrix;
    PPU_Ptr< HMat43 > pTestMatrix2( pTaskData->m_pMatrix );
    
    SPU_Ptr< HMat43 > pMatrix( matrix.Fetch( pTaskData->m_pMatrix ) );
    SPU_Ptr< HMat43 > pRawMatrix( rawMatrix.Fetch( pTaskData->m_pRawMatrix ) );
    
    //SPU_Ptr< HMat43 > pMatrix;
    //SPU_Ptr< HMat43 > pRawMatrix;
    //pMatrix = matrix.Fetch( pTaskData->m_pMatrix );
    //pRawMatrix = rawMatrix.Fetch( pTaskData->m_pRawMatrix );
    
    int test = 0;
    PPU_Ptr< int > pTest( &test );
    //PPU_Ptr< int > pTest2 = &test;
    DMA< int > integer;
    //SPU_Ptr< int > pSpuTest = integer.Fetch( pTest );
    //SPU_Ptr< int > pSpuTest( integer.Fetch( pTest ) );
    int* pSpuTest = integer.Fetch( pTest );
    
    DMA_Wait();
    
    
   
	//pTaskData->m_pPartList->UpdateMatrices( pTaskData->m_Matrix, pTaskData->m_pVisDetect, false, pTaskData->m_pRawMatrix );
	DebugBreak();
}

} // namespace Actors
} // namespace Game

JQ_DECLARE_SPU_MODULE( DeferredVehicleUpdateTaskSPU, Game::Actors::VehicleActorDeferredUpdateSPU, deferredvehicleupdate, "VehicleActorDeferredUpdateSPU" );

#if PLATFORM_SPU

extern "C" void _exit() { snPause(); }

//void* operator new [] ( unsigned long )
void* operator new [] ( size_t size )
{
	DebugBreak();
}

void* operator new( size_t size )
{
	DebugBreak();
}

void operator delete( void* )
{
	DebugBreak();
}

void operator delete [] ( void* )
{
	DebugBreak();
}

#endif

