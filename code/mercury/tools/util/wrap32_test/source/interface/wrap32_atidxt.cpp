#include "wrap32/com/stdafx.h"
#include <core/core.h>
#include <core/utility/singleton.h>
#include <wrap32/common/wrap32_common.h>
#include <wrap32/common/wrap32_xgraphics_args.h>
#include <wrap32/interface/wrap32_instance.h>
#include "ati_compress/ati_compress.h"

#pragma warning(push)
//Disable convertion from x to y of greater size warning as we move from 32->64 bit.
#pragma warning(disable:4311)
#pragma warning(disable:4312)

/*
**	ATI_TC_CalculateBufferSize
**	Calculates the required buffer size for the specified texture
*/
ATI_TC_DWORD ATI_TC_API ATI_TC_CalculateBufferSize(const ATI_TC_Texture* pTexture)
{
	assert(0);
	return 0;
}


/*
**	ATI_TC_ConvertTexture
**	Converts the source texture to the destination texture
*/

ATI_TC_ERROR ATI_TC_API ATI_TC_ConvertTexture(const ATI_TC_Texture* pSourceTexture,	/* [in]  - Pointer to the source texture */
											ATI_TC_Texture* pDestTexture,			/* [out] - Pointer to the destination texture */
											const ATI_TC_CompressOptions* pOptions, /* [in]  - Pointer to the compression options - can be NULL */
											ATI_TC_Feedback_Proc pFeedbackProc,		/* [in]  - Pointer to the feedback proc - can be NULL */
											DWORD_PTR pUser1,						/* [in]  - User data to pass to the feedback proc */
											DWORD_PTR pUser2)						/* [in]  - User data to pass to the feedback proc */
{
	assert(0);
	return ATI_TC_ERROR(0);
}

#pragma warning(pop)