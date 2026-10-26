#pragma once

#define XCHECK_BEGIN if (!xStatus) { std::stringstream ss; ss 
#define XCHECK_ERROR_CLEAN_UP_RETURN_END 	\
	<< " Error" << __FILE__ << " " << __LINE__; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.c_str());\
	\
	CleanUpAfterFailure();\
	Aftk::C_Status status;\
	status.SetErrorString(ss.str().c_str());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	return status;\
}

#define XCHECK_ERROR_RETURN_END \
	<< " Error" << __FILE__ << " " << __LINE__;\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.c_str());\
	\
	Aftk::C_Status status; \
	status.SetErrorString(ss.str().c_str()); \
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	return status; \
}

#define XCHECK_ERROR_RETURN0_END \
	<< " Error " << __FILE__ << " " << __LINE__;\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.c_str());\
	\
	Aftk::C_Status status; \
	status.SetErrorString(ss.str().c_str()); \
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	return 0;\
}

#define XCHECK_ERROR_RETURN \
	if (!xStatus) \
	{ \
		std::stringstream b; \
		b << " Error " << __FILE__ << " " << __LINE__; \
		Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.c_str());\
		status.SetErrorString(ss.str().c_str()); \
		status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
		return status;\
	}

#define XCHECK_ERROR_RETURN0 \
	if (!xStatus) \
	{ \
		std::stringstream b; \
		b << " Error " << __FILE__ << " " << __LINE__; \
		Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.c_str());\
		status.SetErrorString(ss.str().c_str()); \
		status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
		return 0;\
	}

#define XCHECK_ERROR_CLEAN_UP_RETURN \
	if (!xStatus)\
	{ \
		std::stringstream b; \
		b << " Error " << __FILE__ << " " << __LINE__; \
		Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.c_str());\
		status.SetErrorString(ss.str().c_str());\
		status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
		CleanUpAfterFailure();\
		return status;\
	}

#define CHECK_ERROR_CLEAN_UP_RETURN if (!status) { CleanUpAfterFailure(); return status; }
#define CHECK_ERROR_RETURN if (!status) { return status;}




#define AFTK_DO_BEGIN { std::stringstream ss; ss  
#define AFTK_DO_ERROR_END_1 \
	<< " " << __FILE__ << " " << __LINE__; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.str().c_str());\
	\
	Aftk::C_Status status;\
	status.SetErrorString(ss.str().c_str());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
	status.SetSeverity(1);\
}

#define AFTK_DO_ERROR_RETURN_END_1 \
	<< " " << __FILE__ << " " << __LINE__; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.str().c_str());\
	\
	Aftk::C_Status status;\
	status.SetErrorString(ss.str().c_str());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
	status.SetSeverity(1);\
	return status;\
}

#define AFTK_DO_ERROR_CLEAN_UP_RETURN_END_1 \
	<< " " << __FILE__ << " " << __LINE__; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.str().c_str());\
	\
	CleanUpAfterFailure(); \
	Aftk::C_Status status; \
	status.SetErrorString(ss.str().c_str());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
	status.SetSeverity(1);\
	return status;}

#define AFTK_CHECK_BEGIN if (!status) { Tu::StringBuilder b; b 
#define AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END_1 	\
	<< " " << __FILE__ << " " << __LINE__; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.str().c_str());\
	\
	CleanUpAfterFailure();\
	status.SetErrorString(ss.str().c_str());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(1);\
	return status;\
}

#define AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END_0 ;	\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.str().c_str());\
	\
	CleanUpAfterFailure();\
	status.SetErrorString(ss.str().c_str());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(0); return status; \
}

#define AFTK_CHECK_ERROR_RETURN_END_1 \
	<< " " << __FILE__ << " " << __LINE__;\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.str().c_str());\
	\
	status.SetErrorString(ss.str().c_str());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(1);\
	return status;\
}

#define AFTK_CHECK_ERROR_RETURN_END_0 ;	\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.str().c_str());\
	\
	status.SetErrorString(ss.str().c_str());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(0);\
	return status; \
}

#define AFTK_DO_ERROR_END_0					;	\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.str().c_str());\
	\
	status.SetErrorString(ss.str().c_str()); \
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
	status.SetSeverity(0);\
}

#define AFTK_DO_ERROR_RETURN_END_0			;	\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.str().c_str());\
	\
	status.SetErrorString(ss.str().c_str());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
	status.SetSeverity(0); \
	return status;\
}
#define AFTK_DO_ERROR_CLEAN_UP_RETURN_END_0	;	\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.str().c_str());\
	\
	CleanUpAfterFailure();\
	Aftk::C_Status status;\
	status.SetErrorString(ss.str().c_str()); \
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(0); \
	return status;\
}

#define AFTK_DO_INFO_END ; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Info, ss.str().c_str);\
}

#define AFTK_DO_WARNING_END ; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Warning, ss.str().c_str);\
}

#define AFTK_DO_ERROR_END_NO_STATUS	; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, ss.str().c_str);\
}

#define AFTK_DO_DEBUG_END ; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Debug, ss.str().c_str);\
}

#define AFTK_CHECK_ERROR_CLEAN_UP_RETURN if (!status) { CleanUpAfterFailure(); return status; }
#define AFTK_CHECK_ERROR_RETURN if (!status) { return status;}
#define AFTK_CHECK_ERROR_RETURN0 if (!status) { return 0;}
#define AFTK_DO_ERROR_RETURN_END AFTK_DO_ERROR_RETURN_END_0
#define AFTK_DO_ERROR_END AFTK_DO_ERROR_END_0
#define AFTK_CHECK_ERROR_RETURN_END AFTK_CHECK_ERROR_RETURN_END_0
#define AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END AFTK_CHECK_ERROR_CLEAN_UP_RETURN_END_0
