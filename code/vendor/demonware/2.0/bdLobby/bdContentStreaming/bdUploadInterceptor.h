// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_UPLOAD_INTERCEPTOR_H
#define BD_UPLOAD_INTERCEPTOR_H

#include <bdCore/bdCore.h>

/// Purpose: A class to inherit from when implementing a custom upload handler
/// for content server HTTP requests
class bdUploadInterceptor
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Default constructor
	bdUploadInterceptor();

	/// Virtual destructor
	virtual ~bdUploadInterceptor();

	/// The callback triggered when data has been uploaded and is ready to be handled
	/// \param uploadBuffer[in] : A pointer to the uploadBuffer to be filled
	/// \param bufferSize[in] : The max size of the data pointed to by uploadBuffer
	/// \param bytesUploaded[in] : The number of bytes sent so far by the underlying http layer
	/// \return The size of data copied into uploadBuffer (must be less than bufferSize)
	///			Returning 0 indicates that no more data needs to be uploaded
	virtual bdUInt handleUpload(void* uploadBuffer, bdUInt32 bufferSize, bdUInt32 bytesUploaded) = 0;
};

#endif //BD_UPLOAD_INTERCEPTOR_H

