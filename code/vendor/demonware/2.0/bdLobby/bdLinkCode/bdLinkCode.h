// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_LINKCODE_H
#define BD_LINKCODE_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLinkCode/bdLink.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyErrorCode.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;

#define BD_LINKCODE_GET_LINK_CODES					(1u)
#define BD_LINKCODE_GET_ENTITY_IDS_FROM_LINK_CODES	(2u)


/// bdLinkCode service
class bdLinkCode
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdLinkCode(bdRemoteTaskManager *const remoteTaskManager);

		/// Destructor
		~bdLinkCode();

		/// Retrieves (possibly generating) the link codes for the given entity 
		/// ID in the given categories.
		/// \param entityID[in]:		The ID of the entity to retrieve the
		///								link codes for.
		/// \param categories[in]:		An array of categories to retrieve the
		///								link codes for.
		/// \param links[out]:			An array of bdLink objects of length at
		///								least numCategories.  The result of the
		///								request will be written here.
		/// \param numCategories[in]:	The number of categories in the array.
		/// \return	A reference to the task handle that should be pumped to
		///			completion, or BD_NULL if the operation could not be 
		///			performed (message could not be sent, or maximum number of
		///			tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		/// \note	This service makes no guarantee on the order of the bdLink
		///			objects written to the provided links array.
		bdRemoteTaskRef getLinkCodes(const bdUInt64 entityID,
									 const bdUInt32 *const categories,
									 bdLink *const links,
									 const bdUInt numCategories);

		/// Gets the entity IDs associated with the provided link codes in a
		/// specific category.
		/// \param category[in]:		The category that the provided link
		///								codes are in.
		/// \param linkCodes[in]:		An array of link codes (pointers to
		///								null terminated strings) to retrieve
		///								the entity IDs of.
		/// \param links[out]:			An array of bdLink objects of length at
		///								least numLinkCodes.  The result of the
		///								request will be written here.
		/// \param numLinkCodes[in]:	The number of link codes in the array.
		/// \return	A reference to the task handle that should be pumped to
		///			completion, or BD_NULL if the operation could not be 
		///			performed (message could not be sent, or maximum number of
		///			tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		/// \note	This service makes no guarantee on the order of the bdLink
		///			objects written to the provided links array.
		/// \note	It is possible that fewer bdLink objects will be returned
		///			than the number of link codes provided.  This happens only
		///			when some of the provided link codes are invalid.
		bdRemoteTaskRef getEntityIDsFromLinkCodes(const bdUInt32 category,
												  const bdNChar8 *const *const linkCodes,
												  bdLink *const links,
												  const bdUInt numLinkCodes);

	protected:

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif //BD_LINKCODE_H

