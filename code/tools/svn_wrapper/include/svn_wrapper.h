#ifndef SVN_WRAPPER_H
#define SVN_WRAPPER_H

#include <Xt/Impl/XtCore.h>
#include <Xt/Impl/XtUt.h>
#include <Xt/Impl/Sz.h>

struct svn_client_ctx_t;
struct apr_pool_t;
struct apr_allocator_t;

namespace Vcs
{

	enum E_Revision
	{
		kRevision_Head = -1
	};

	enum E_ItemStatus
	{
		kStatus_None,			// Does not exist
		kStatus_Unversioned,	// Is not a versioned thing in this wc
		kStatus_Normal,			// Exists, but uninteresting
		kStatus_Added,			// Is scheduled for addition
		kStatus_Missing,		// Under v.c., but is missing
		kStatus_Deleted,		// Scheduled for deletion
		kStatus_Replaced,		// Was deleted and then re-added
		kStatus_Modified,		// Text or props have been modified
		kStatus_Merged,			// Local mods received repos mods
		kStatus_Conflicted,		// Local mods received conflicting repos mods
		kStatus_Ignored,		// Is unversioned but configured to be ignored
		kStatus_Obstructed,		// An unversioned resource is in the way of the versioned resource
		kStatus_External,		// An unversioned path populated by an svn:externals property
		kStatus_Incomplete		// A directory doesn't contain a complete entries list
	};

	enum E_Schedule
	{
		kSchedule_None,
		kSchedule_Normal,
		kSchedule_Add,
		kSchedule_Delete,
		kSchedule_Replace
	};

	enum E_NodeKind
	{
		kNode_None,		// Absent
		kNode_File,		// Regular file
		kNode_Dir,		// Directory
		kNode_Unknown	// Something's here, but we don't know what
	};

	enum E_Depth
	{
		kDepth_Unknown		= -2,	// Depth undetermined or ignored
		kDepth_Exclude		= -1,	// Exclude (i.e., don't descend into) directory
		kDepth_Empty		= 0,	// Just the directory, no entries
		kDepth_Files		= 1,	// Directory and files, no subdirs
		kDepth_Immediates	= 2,	// Directory and immediate children
		kDepth_Infinity		= 3		// Directory and all descendants
	};

	enum E_NotifyAction
	{
		kAction_None,
		kAction_Add,					// Adding a path to revision control
		kAction_Copy,					// Copying a versioned path
		kAction_Delete,					// Deleting a versioned path
		kAction_Restore,				// Restoring a missing path from the pristine text-base
		kAction_Revert,					// Reverting a modified path
		kAction_FailedRevert,			// A revert operation has failed
		kAction_Resolved,				// Resolving a conflict
		kAction_Skip,					// Skipping a path
		kAction_UpdateDelete,			// Got a delete in an update
		kAction_UpdateAdd,				// Got an add in an update
		kAction_UpdateUpdate,			// Got any other action in an update
		kAction_UpdateCompleted,		// The last notification in an update (including updates of externals)
		kAction_UpdateExternal,			// Updating an external module
		kAction_StatusCompleted,		// The last notification in a status (including status on externals)
		kAction_StatusExternal,			// Running status on an external module
		kAction_CommitModified,			// Committing a modification
		kAction_CommitAdded,			// Committing an addition
		kAction_CommitDeleted,			// Committing a deletion
		kAction_CommitReplaced,			// Committing a replacement
		kAction_CommitPostfixTxDelta,	// Transmitting post-fix text-delta data for a file
		kAction_BlameRevision,			// Processed a single revision's blame
		kAction_Locked,					// Locking a path
		kAction_Unlocked,				// Unlocking a path
		kAction_FailedLock,				// Failed to lock a path
		kAction_FailedUnlock,			// Failed to unlock a path
		kAction_Exists,					// Tried adding a path that already exists
		kAction_ChangelistSet,			// Changelist name set
		kAction_ChangelistClear,		// Changelist name cleared
		kAction_ChangelistMoved,		// Warn user that a path has moved from one changelist to another
		kAction_MergeBegin,				// A merge operation (to path) has begun
		kAction_ForeignMergeBegin,		// A merge operation (to path) from a foreign repository has begun
		kAction_UpdateReplace			// Replace notification
	};

	enum E_NotifyState
	{
		kState_Inapplicable,
		kState_Unknown,			// Notifier doesn't know or isn't saying
		kState_Unchanged,		// The state did not change
		kState_Missing,			// The item wasn't present
		kState_Obstructed,		// An unversioned item obstructed work
		kState_Changed,			// Pristine state was modified
		kState_Merged,			// Modified state had mods merged in
		kState_Conflicted		// Modified state got conflicting mods
	};

	struct Status
	{
		enum Code
		{
			kSuccess,
			kFailed,
			kAuthorisationError,
			kNotInitialised,
			kOutOfMemory,
			kNotAWorkingCopy
		};

	public:
		Code	code;
		Sz8		text;

	public:
		Status() : code(kSuccess) { }
		explicit Status(Code code_) : code(code_) { }
		Status(Code code_, const Sz8& text_) : code(code_), text(text_) { }

	public:
		operator bool() const { return code == kSuccess; }
	};

	struct Notify
	{
		const char*		path;
		E_NotifyAction	action;
		E_NodeKind		kind;
		Status			error;
		E_NotifyState	contentState;
		E_NotifyState	propState;
		int				revision;

	public:
		Notify();

	public:
		Sz8 AsText() const;
	};

	struct StatusEntry
	{
		// Local

		const char* path;

		E_ItemStatus textStatus;
		E_ItemStatus propStatus;

		bool switched;

		// Entry

		const char*	name;		// Entry's name
		int			revision;	// Base revision
		const char*	url;		// Repository url
		const char*	uuid;		// Repository uuid
		E_NodeKind	kind;		// File, dir, ...
		E_Schedule	schedule;	// Scheduling (add, delete, replace, ...)
		bool		copied;
		bool		deleted;
		bool		absent;
		bool		incomplete;
		const char*	conflictOld; // Old version of conflicted file
		const char*	conflictNew; // New version of conflicted file
		const char*	conflictWrk; // Working version of conflicted file

	public:
		StatusEntry();
	};

	class VcsContext
	{
	public:
		VcsContext()
		{
		}

		virtual ~VcsContext()
		{
		}

	public:
		virtual bool IsCancelled()
		{
			return false;
		}
		
		virtual bool GetLogin(Sz8& username, Sz8& password, const Sz8& realm, bool& maySave)
		{
			return false;
		}
	};

	typedef void (*NotifyCallback)(const Notify& notify, void* baton);
	typedef void (*StatusCallback)(const StatusEntry& statusEntry, void* baton);
	typedef bool (*SvnCancelCallback)();

	class Svn
	{
	public:
		Svn(VcsContext* context = 0);
		virtual ~Svn();

	public:
		operator bool() const { return _initialised; }

	public:
		Status Checkout(const Sz8& url, const Sz8& path, E_Depth depth = kDepth_Infinity, int revision = kRevision_Head, NotifyCallback callback = 0, void* baton = 0);
		Status Update(const Sz8& path, E_Depth depth = kDepth_Infinity, int revision = kRevision_Head, NotifyCallback callback = 0, void* baton = 0);
		Status Revert(const Sz8& path, E_Depth depth = kDepth_Infinity, NotifyCallback callback = 0, void* baton = 0);
		Status Add(const Sz8& path, E_Depth depth = kDepth_Infinity, NotifyCallback callback = 0, void* baton = 0);
		Status Delete(const Sz8& path, NotifyCallback callback = 0, void* baton = 0);
		Status Copy(const Sz8& src, const Sz8& dst, NotifyCallback callback = 0, void* baton = 0);
		Status Move(const Sz8& src, const Sz8& dst, NotifyCallback callback = 0, void* baton = 0);
		Status Commit(const Sz8& path, const Sz8& log, E_Depth depth = kDepth_Infinity, NotifyCallback callback = 0, void* baton = 0);
		Status Commit(XtUt::Array<Sz8>* paths, const Sz8& log, E_Depth depth = kDepth_Empty, NotifyCallback callback = 0, void* baton = 0);
		Status Info(const Sz8& path, E_Depth depth = kDepth_Infinity, StatusCallback callback = 0, void* baton = 0);
		Status Cleanup(const Sz8& path);
		Status SetProperty(const Sz8& path, const Sz8& prop, const Sz8& val, E_Depth depth = kDepth_Infinity);
		Status GetProperty(const Sz8& path, const Sz8& prop, Sz8& outVal);
		Status GetPristineCopy(const Sz8& path, Sz8& outPath);

	private:
		void Initialise();
		void Destroy();

	private:
		VcsContext*			_context;
		apr_pool_t*			_clientPool;
		svn_client_ctx_t*	_clientContext;
		bool				_initialised;

	private:
		Svn(const Svn&);
		void operator=(const Svn&);
	};

}

#endif // SVN_WRAPPER_H