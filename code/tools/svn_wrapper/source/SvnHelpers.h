namespace Vcs
{
	//A helper class to restore the unhandled exception filter (after calls to svn_cmdline_init())
	//rather than recompile svn without SVN_USE_WIN32_CRASHHANDLER
	//The l33t alternative would be to use an API hook and patch 
	//SetUnhandledExceptionFilter to prevent any overriding
	//see http://www.debuginfo.com/articles/debugfilters.html#enforce
	class C_ExceptionFilterHelper
	{
	public:
		C_ExceptionFilterHelper()
		{
			//get current exception callback
			m_pExceptionFilter = ::SetUnhandledExceptionFilter(0);
		}

		~C_ExceptionFilterHelper()
		{
			//restore
			if (m_pExceptionFilter)
				::SetUnhandledExceptionFilter(m_pExceptionFilter);
		}
		LPTOP_LEVEL_EXCEPTION_FILTER m_pExceptionFilter;
	};

	int Max(int a, int b)
	{
		return (a > b) ? a : b;
	}

	// SnprintErrorString

	void SnprintErrorString(char* pDst, const int maxLength, svn_error_t* pError)
	{
		char* pStart	= pDst;
		char* pEnd		= pDst + maxLength;

		for(; pError && (pDst < pEnd); pError = pError->child)
		{
			if ((pStart != pDst) && ((pDst + 1) < pEnd))
				*pDst++ = '\n';

			int remaining	= Max((int)(pEnd - pDst), 0);
			int result		= 0;

			if (pError->message)
			{
				result = _snprintf_s(pDst, remaining, _TRUNCATE, "%s", pError->message);
			}
			else
			{
				if (svn_strerror(pError->apr_err, pDst, remaining))
					result = strlen(pDst);
				else
					result = _snprintf_s(pDst, remaining, _TRUNCATE, "APR_ERR(%08X)", pError->apr_err);
			}

			if (result < 0)
				break;

			pDst += result;
		}

		if (pDst < pEnd)
			*pDst = '\0';
		else if (maxLength > 0)
			*(pEnd - 1) = '\0';
	}

	// AprPool

	class AprPool
	{
	public:
		AprPool(apr_pool_t* parentPool) :
			_pool(svn_pool_create(parentPool))
		{
		}

		~AprPool()
		{
			if (_pool)
				svn_pool_destroy(_pool);
		}

	public:
		operator apr_pool_t*()		{ return _pool; }
		apr_pool_t* operator->()	{ return _pool; }

	private:
		apr_pool_t* _pool;
	};

	// AprArray
	
	template<typename Type>
	class AprArray
	{
	public:
		AprArray() :
			_array(0)
		{
		}

		~AprArray()
		{
			Clear();
		}

	public:
		bool Make(apr_pool_t* pool, int initialCapacity)
		{
			if (_array)
				Clear();

			_array = apr_array_make(pool, initialCapacity, sizeof(Type));

			return _array != 0;
		}

		void Clear()
		{
			if (_array)
				apr_array_clear(_array);
		}

		apr_array_header_t* Detach()
		{
			apr_array_header_t* array = _array;

			_array = 0;

			return array;
		}

	public:
		bool PushBack(const Type& value)
		{
			if (Type* p = Append())
			{
				*p = value;
				return true;
			}

			return false;
		}

		Type* Append()
		{
			if (!_array)
				return 0;

			return (Type*)apr_array_push(_array);
		}

	public:
		operator apr_array_header_t*()		{ return _array; }
		apr_array_header_t* operator->()	{ return _array; }

	private:
		apr_array_header_t* _array;
	};

	// SvnError

	class SvnError
	{
	public:
		SvnError(svn_error_t* error = 0) : _error(error) { }
		~SvnError() { svn_error_clear(_error); }

	public:
		SvnError& operator=(svn_error_t* error)
		{
			if (_error != error)
			{
				svn_error_clear(_error);
				_error = error;
			}

			return *this;
		}
		
	public:
		operator svn_error_t*()		{ return _error; }
		svn_error_t* operator->()	{ return _error; }

	private:
		svn_error_t* _error;
	};

	Status AsStatus(svn_error_t* error)
	{
		if (!error)
			return Status(Status::kSuccess);

		Status status;

		switch(error->apr_err)
		{
			case SVN_ERR_WC_NOT_DIRECTORY:
			case SVN_ERR_WC_NOT_FILE:
				status.code = Status::kNotAWorkingCopy;
			break;

			case SVN_ERR_RA_NOT_AUTHORIZED:
			case SVN_ERR_RA_UNKNOWN_AUTH:
				status.code = Status::kAuthorisationError;
			break;

			default:
				status.code = Status::kFailed;
			break;
		}

		char buf[256];

		SnprintErrorString(buf, sizeof(buf), error);

		status.text = buf;

		return status;
	}

	// E_ItemStatus

	E_ItemStatus GetStatus(svn_wc_status_kind status)
	{
		switch (status)
		{
			case svn_wc_status_none:		return kStatus_None;
			case svn_wc_status_unversioned:	return kStatus_Unversioned;
			case svn_wc_status_normal:		return kStatus_Normal;
			case svn_wc_status_added:		return kStatus_Added;
			case svn_wc_status_missing:		return kStatus_Missing;
			case svn_wc_status_deleted:		return kStatus_Deleted;
			case svn_wc_status_replaced:	return kStatus_Replaced;
			case svn_wc_status_modified:	return kStatus_Modified;
			case svn_wc_status_merged:		return kStatus_Merged;
			case svn_wc_status_conflicted:	return kStatus_Conflicted;
			case svn_wc_status_ignored:		return kStatus_Ignored;
			case svn_wc_status_obstructed:	return kStatus_Obstructed;
			case svn_wc_status_external:	return kStatus_External;
			case svn_wc_status_incomplete:	return kStatus_Incomplete;
		}

		return kStatus_None;
	}

	// E_Schedule

	E_Schedule GetSchedule(svn_wc_schedule_t schedule)
	{
		switch(schedule)
		{
			case svn_wc_schedule_normal:	return kSchedule_Normal;
			case svn_wc_schedule_add:		return kSchedule_Add;
			case svn_wc_schedule_delete:	return kSchedule_Delete;
			case svn_wc_schedule_replace:	return kSchedule_Replace;
		}

		return kSchedule_None;
	}

	// E_NodeKind

	E_NodeKind GetNodeKind(svn_node_kind_t kind)
	{
		switch(kind)
		{
			case svn_node_none:		return kNode_None;
			case svn_node_file:		return kNode_File;
			case svn_node_dir:		return kNode_Dir;
			case svn_node_unknown:	return kNode_Unknown;
		}

		return kNode_Unknown;
	}

	// E_Depth

	svn_depth_t GetDepth(E_Depth depth)
	{
		switch(depth)
		{
			case kDepth_Unknown:	return svn_depth_unknown;
			case kDepth_Exclude:	return svn_depth_exclude;
			case kDepth_Empty:		return svn_depth_empty;
			case kDepth_Files:		return svn_depth_files;
			case kDepth_Immediates:	return svn_depth_immediates;
			case kDepth_Infinity:	return svn_depth_infinity;
		}

		return svn_depth_unknown;
	};

	// E_NotifyAction

	E_NotifyAction GetNotifyAction(svn_wc_notify_action_t action)
	{
		switch(action)
		{
			case svn_wc_notify_add:						return kAction_Add;
			case svn_wc_notify_copy:					return kAction_Copy;
			case svn_wc_notify_delete:					return kAction_Delete;
			case svn_wc_notify_restore:					return kAction_Restore;
			case svn_wc_notify_revert:					return kAction_Revert;
			case svn_wc_notify_failed_revert:			return kAction_FailedRevert;
			case svn_wc_notify_resolved:				return kAction_Resolved;
			case svn_wc_notify_skip:					return kAction_Skip;
			case svn_wc_notify_update_delete:			return kAction_UpdateDelete;
			case svn_wc_notify_update_add:				return kAction_UpdateAdd;
			case svn_wc_notify_update_update:			return kAction_UpdateUpdate;
			case svn_wc_notify_update_completed:		return kAction_UpdateCompleted;
			case svn_wc_notify_update_external:			return kAction_UpdateExternal;
			case svn_wc_notify_status_completed:		return kAction_StatusCompleted;
			case svn_wc_notify_status_external:			return kAction_StatusExternal;
			case svn_wc_notify_commit_modified:			return kAction_CommitModified;
			case svn_wc_notify_commit_added:			return kAction_CommitAdded;
			case svn_wc_notify_commit_deleted:			return kAction_CommitDeleted;
			case svn_wc_notify_commit_replaced:			return kAction_CommitReplaced;
			case svn_wc_notify_commit_postfix_txdelta:	return kAction_CommitPostfixTxDelta;
			case svn_wc_notify_blame_revision:			return kAction_BlameRevision;
			case svn_wc_notify_locked:					return kAction_Locked;
			case svn_wc_notify_unlocked:				return kAction_Unlocked;
			case svn_wc_notify_failed_lock:				return kAction_FailedLock;
			case svn_wc_notify_failed_unlock:			return kAction_FailedUnlock;
			case svn_wc_notify_exists:					return kAction_Exists;
			case svn_wc_notify_changelist_set:			return kAction_ChangelistSet;
			case svn_wc_notify_changelist_clear:		return kAction_ChangelistClear;
			case svn_wc_notify_changelist_moved:		return kAction_ChangelistMoved;
			case svn_wc_notify_merge_begin:				return kAction_MergeBegin;
			case svn_wc_notify_foreign_merge_begin:		return kAction_ForeignMergeBegin;
			case svn_wc_notify_update_replace:			return kAction_UpdateReplace;
		}

		return kAction_None;
	}

	E_NotifyState GetNotifyState(svn_wc_notify_state_t state)
	{
		switch(state)
		{
			case svn_wc_notify_state_inapplicable:	return kState_Inapplicable;
			case svn_wc_notify_state_unknown:		return kState_Unknown;
			case svn_wc_notify_state_unchanged:		return kState_Unchanged;
			case svn_wc_notify_state_missing:		return kState_Missing;
			case svn_wc_notify_state_obstructed:	return kState_Obstructed;
			case svn_wc_notify_state_changed:		return kState_Changed;
			case svn_wc_notify_state_merged:		return kState_Merged;
			case svn_wc_notify_state_conflicted:	return kState_Conflicted;
		}

		return kState_Unknown;
	}

	const char* GetNotifyActionAsText(E_NotifyAction action)
	{
		switch(action)
		{
			case kAction_Add:					return "Add";
			case kAction_Copy:					return "Copy";
			case kAction_Delete:				return "Delete";
			case kAction_Restore:				return "Restore";
			case kAction_Revert:				return "Revert";
			case kAction_FailedRevert:			return "Failed Revert";
			case kAction_Resolved:				return "Resolved";
			case kAction_Skip:					return "Skip";
			case kAction_UpdateDelete:			return "Update Delete";
			case kAction_UpdateAdd:				return "Update Add";
			case kAction_UpdateUpdate:			return "Update Update";
			case kAction_UpdateCompleted:		return "Update Completed";
			case kAction_UpdateExternal:		return "Update External";
			case kAction_StatusCompleted:		return "Status Completed";
			case kAction_StatusExternal:		return "Status External";
			case kAction_CommitModified:		return "Commit Modified";
			case kAction_CommitAdded:			return "Commit Added";
			case kAction_CommitDeleted:			return "Commit Deleted";
			case kAction_CommitReplaced:		return "Commit Replaced";
			case kAction_CommitPostfixTxDelta:	return "Commit Postfix Tx Delta";
			case kAction_BlameRevision:			return "Blame Revision";
			case kAction_Locked:				return "Locked";
			case kAction_Unlocked:				return "Unlocked";
			case kAction_FailedLock:			return "Failed Lock";
			case kAction_FailedUnlock:			return "Failed Unlock";
			case kAction_Exists:				return "Exists";
			case kAction_ChangelistSet:			return "Changelist Set";
			case kAction_ChangelistClear:		return "Changelist Clear";
			case kAction_ChangelistMoved:		return "Changelist Moved";
			case kAction_MergeBegin:			return "Merge Begin";
			case kAction_ForeignMergeBegin:		return "Foreign Merge Begin";
			case kAction_UpdateReplace:			return "Update Replace";
		}

		return "";
	}
}