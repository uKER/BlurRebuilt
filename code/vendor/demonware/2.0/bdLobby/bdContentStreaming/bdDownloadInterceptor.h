// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PRIVATE
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_DOWNLOAD_INTERCEPTOR_H
#define BD_DOWNLOAD_INTERCEPTOR_H

#include <bdCore/bdCore.h>

/// Purpose: A class to inherit from when implementing a custom download handler
/// for content server HTTP requests
class bdDownloadInterceptor
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Default constructor
	bdDownloadInterceptor();

	/// Virtual destructor
	virtual ~bdDownloadInterceptor();

	/// The callback triggered when data has been downloaded and is ready to be handled
	/// \param data[in] : A pointer to the data that has been downloaded
	/// \param dataSize[in] : The size of the data pointed to by data
	/// \return The size of data handled
	///			Returning 0 indicates that the download is finished or should be aborted
	virtual bdUInt handleDownload(void* data, bdUInt32 dataSize) = 0;
};

#endif //BD_DOWNLOAD_INTERCEPTOR_H

