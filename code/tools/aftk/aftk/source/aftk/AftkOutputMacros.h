#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_OUTPUT_MACROS_H
#define AFTK_OUTPUT_MACROS_H

#define AFTK_DO_BEGIN { Tu::StringBuilder b; b  
#define AFTK_DO_ERROR_END_1 \
	<< " " << __FILE__ << " " << __LINE__; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	Aftk::C_Status status;\
	status.SetErrorString(b.Text());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
	status.SetSeverity(1);\
}

#define AFTK_DO_ERROR_RETURN_END_1 \
	<< " " << __FILE__ << " " << __LINE__; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	Aftk::C_Status status;\
	status.SetErrorString(b.Text());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
	status.SetSeverity(1);\
	return status;\
}

#define AFTK_DO_ERROR_CLEAN_UP_RETURN_END_1 \
	<< " " << __FILE__ << " " << __LINE__; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	CleanUpAfterFailure(); \
	Aftk::C_Status status; \
	status.SetErrorString(b.Text());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
	status.SetSeverity(1);\
	return status;}

#define AFTK_CHECK_BEGIN if (!status) { Tu::StringBuilder b; b 
#define AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END_1 	\
	<< " " << __FILE__ << " " << __LINE__; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	CleanUpAfterFailure();\
	status.SetErrorString(b.Text());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(1);\
	return status;\
}

#define AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END_0 ;	\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	CleanUpAfterFailure();\
	status.SetErrorString(b.Text());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(0); return status; \
}

#define AFTK_CHECK_ERROR_RETURN_END_1 \
	<< " " << __FILE__ << " " << __LINE__;\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	status.SetErrorString(b.Text());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(1);\
	return status;\
}

#define AFTK_CHECK_ERROR_RETURN_END_0 ;	\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	status.SetErrorString(b.Text());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(0);\
	return status; \
}

#define AFTK_DO_ERROR_END_0					;	\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	status.SetErrorString(b.Text()); \
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
	status.SetSeverity(0);\
}

#define AFTK_DO_ERROR_RETURN_END_0			;	\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	status.SetErrorString(b.Text());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
	status.SetSeverity(0); \
	return status;\
}
#define AFTK_DO_ERROR_CLEAN_UP_RETURN_END_0	;	\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	CleanUpAfterFailure();\
	Aftk::C_Status status;\
	status.SetErrorString(b.Text()); \
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(0); \
	return status;\
}

#define AFTK_DO_INFO_END ; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Info, b.Text());\
}

#define AFTK_DO_WARNING_END ; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Warning, b.Text());\
}

#define AFTK_DO_ERROR_END_NO_STATUS	; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
}

#define AFTK_DO_DEBUG_END ; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Debug, b.Text());\
}

#define AFTK_CHECK_ERROR_CLEAN_UP_RETURN if (!status) { CleanUpAfterFailure(); return status; }
#define AFTK_CHECK_ERROR_RETURN if (!status) { return status;}
#define AFTK_CHECK_ERROR_RETURN0 if (!status) { return 0;}
#define AFTK_DO_ERROR_RETURN_END AFTK_DO_ERROR_RETURN_END_0
#define AFTK_DO_ERROR_END AFTK_DO_ERROR_END_0
#define AFTK_CHECK_ERROR_RETURN_END AFTK_CHECK_ERROR_RETURN_END_0
#define AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END_0

#endif

