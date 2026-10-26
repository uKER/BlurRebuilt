#include "AudioPch.h"
#ifdef SN_TARGET_PS3_SPU
#include "SystemHeadersSPU.h"
#else
#include "SystemHeaders.h"
#endif

//using namespace Audio;
/*
////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void* StereoConvolution::Init(void* pData)
{
    m_pData = (StereoConvolution::Data*)pData;

    m_pImpulseL = NULL;
    m_pImpulseR = NULL;

    U32 uInpulseLengthInFrames = ((m_pData->uImpulseLength - 1) / Core::uFrameLength) + 1;

    m_pDSP = CreateStereoDSP48kHzConvolution(uInpulseLengthInFrames);

    return (void*)(m_pData + 1);
}

////////////////////////////////////////////////////////////////////////////////
//	Release
////////////////////////////////////////////////////////////////////////////////
void MonoWaveTable::ReleaseContext(U8* pObjContext)
{
    if (pContext)
    {
        delete pContext;
        pContext = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void StereoConvolution::Update(U8* pObjContext)
{
    if (m_pImpulseL == NULL)
    {
        m_pObject->GetWaveManager()->Get((char*)m_pObject->GetString(m_pData->strBank), (char*)m_pObject->GetString(m_pData->strWaveL), m_pImpulseL);

        m_pDSP->SetImpulse(0, m_pImpulseL);
    }
    
    if (m_pImpulseR == NULL)
    {
        m_pObject->GetWaveManager()->Get((char*)m_pObject->GetString(m_pData->strBank), (char*)m_pObject->GetString(m_pData->strWaveR), m_pImpulseR);

        m_pDSP->SetImpulse(1, m_pImpulseR);
    }
}

////////////////////////////////////////////////////////////////////////////////
//	Name
////////////////////////////////////////////////////////////////////////////////
const char* StereoConvolution::Name()
{
    return m_pObject->GetString(m_pData->strName);
}
*/

