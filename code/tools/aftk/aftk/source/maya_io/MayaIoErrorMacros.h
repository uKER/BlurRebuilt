#ifndef MAYA_IO_OUTPUT_MACROS_H
#define MAYA_IO_OUTPUT_MACROS_H

#ifdef _MSC_VER_
#pragma once
#endif

#define MCHECK_BEGIN if (!mStatus) { Tu::StringBuilder b; b 
#define MCHECK_ERROR_CLEAN_UP_RETURN_END_1 	\
	<< " Maya says [" << mStatus.errorString().asChar() << "] " << __FILE__ << " " << __LINE__; \
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	CleanUpAfterFailure();\
	Aftk::C_Status status;\
	status.SetErrorString(b.Text());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(1); \
	return status;\
}

#define MCHECK_ERROR_CLEAN_UP_RETURN_END_0  \
	<< " Maya says [" << mStatus.errorString().asChar() << "] ";\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	CleanUpAfterFailure();\
	Aftk::C_Status status;\
	status.SetErrorString(b.Text());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(0);\
	return status;\
 }

#define MCHECK_ERROR_RETURN_END_0  \
	<< " Maya says [" << mStatus.errorString().asChar() << "] ";\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	Aftk::C_Status status;\
	status.SetErrorString(b.Text());\
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(0);\
	return status;\
 }

#define MCHECK_ERROR_RETURN_END_1 \
	<< " Maya says [" << mStatus.errorString().asChar() << "] " << __FILE__ << " " << __LINE__;\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	Aftk::C_Status status; \
	status.SetErrorString(b.Text()); \
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(1);\
	return status; \
}

#define MCHECK_ERROR_RETURN0_END_1 \
	<< " Maya says [" << mStatus.errorString().asChar() << "] " << __FILE__ << " " << __LINE__;\
	Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
	\
	Aftk::C_Status status; \
	status.SetErrorString(b.Text()); \
	status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
	status.SetSeverity(1);\
	return 0;\
}

#define MCHECK_ERROR_RETURN_1 \
	if (!mStatus) \
	{ \
		Tu::StringBuilder b; \
		b << " Maya says [" << mStatus.errorString().asChar() << "] " << __FILE__ << " " << __LINE__; \
		Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
		status.SetErrorString(b.Text()); \
		status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
		status.SetSeverity(1);\
		return status;\
	}

#define MCHECK_ERROR_RETURN0_1 \
	if (!mStatus) \
	{ \
		Tu::StringBuilder b; \
		b << " Maya says [" << mStatus.errorString().asChar() << "] " << __FILE__ << " " << __LINE__; \
		Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
		status.SetErrorString(b.Text()); \
		status.SetErrorCode(Aftk::C_Status::E_Status_Failure); \
		status.SetSeverity(1);\
		return 0;\
	}

#define MCHECK_ERROR_CLEAN_UP_RETURN_1 \
	if (!mStatus)\
	{ \
		Tu::StringBuilder b; \
		b << " Maya says [" << mStatus.errorString().asChar() << "] " << __FILE__ << " " << __LINE__; \
		Aftk::C_Global::OutputText(Aftk::E_OutputChannel_Error, b.Text());\
		status.SetErrorString(b.Text());\
		status.SetErrorCode(Aftk::C_Status::E_Status_Failure);\
		status.SetSeverity(1); \
		CleanUpAfterFailure();\
		return status;\
	}

#define CHECK_ERROR_CLEAN_UP_RETURN if (!status) { CleanUpAfterFailure(); return status; }
#define CHECK_ERROR_RETURN if (!status) { return status;}

#endif
