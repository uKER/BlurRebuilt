#include "stdafx.h"
#include <svn_wrapper.h>
#include <Xt/XtTool.h>
#include "SvnHelpers.h"

namespace Vcs
{

	// Vcs::Notify

	Notify::Notify() :
		path(0),
		action(kAction_None),
		kind(kNode_None),
		contentState(kState_Inapplicable),
		propState(kState_Inapplicable),
		revision(0)
	{
	};

	Sz8 Notify::AsText() const
	{
		Sz8 text = GetNotifyActionAsText(action);

		if (path)
			return text + " " + path;

		return text;
	}

	// Vcs::StatusEntry

	StatusEntry::StatusEntry() :
		path(0),
		textStatus(kStatus_None),
		propStatus(kStatus_None),
		switched(false),
		url(0),
		name(0),
		revision(0),
		uuid(0),
		kind(kNode_None),
		schedule(kSchedule_None),
		copied(false),
		absent(false),
		incomplete(false),
		conflictOld(0),
		conflictNew(0),
		conflictWrk(0)
	{
	}

	// Vcs::NotifyCallbackBaton

	struct NotifyCallbackBaton
	{
		NotifyCallback	callback;
		void*			baton;

	public:
		NotifyCallbackBaton(NotifyCallback callback_, void* baton_) : callback(callback_), baton(baton_) { }

	public:
		static void Proc(void* baton, const svn_wc_notify_t* notify, apr_pool_t* pool)
		{
			NotifyCallbackBaton* ncb = (NotifyCallbackBaton*)baton;

			if (!ncb->callback)
				return;

			Notify n;

			n.path			= notify->path;
			n.action		= GetNotifyAction(notify->action);
			n.kind			= GetNodeKind(notify->kind);
			n.error			= AsStatus(notify->err);
			n.contentState	= GetNotifyState(notify->content_state);
			n.propState		= GetNotifyState(notify->prop_state);
			n.revision		= notify->revision;

			ncb->callback(n, ncb->baton);
		}
	};

	// Vcs::StatusCallbackBaton

	struct StatusCallbackBaton
	{
		StatusCallback	callback;
		void*			baton;

	public:
		StatusCallbackBaton(StatusCallback callback_, void* baton_) : callback(callback_), baton(baton_) { }

	public:
		static void Proc(void* baton, const char* path, svn_wc_status2_t* status)
		{
			StatusCallbackBaton* scb = (StatusCallbackBaton*)baton;

			if (!scb->callback)
				return;

			StatusEntry info;

			info.path		= path;
			info.textStatus	= GetStatus(status->text_status);
			info.propStatus	= GetStatus(status->prop_status);
			info.switched	= status->switched != 0;

			if (status->entry)
			{
				info.name			= status->entry->name;
				info.revision		= status->entry->revision;
				info.url			= status->entry->url;
				info.uuid			= status->entry->uuid;
				info.kind			= GetNodeKind(status->entry->kind);
				info.schedule		= GetSchedule(status->entry->schedule);
				info.copied			= status->entry->copied != 0;
				info.deleted		= status->entry->deleted != 0;
				info.absent			= status->entry->absent != 0;
				info.incomplete		= status->entry->incomplete != 0;
				info.conflictOld	= status->entry->conflict_old;
				info.conflictNew	= status->entry->conflict_new;
				info.conflictWrk	= status->entry->conflict_wrk;
			}

			scb->callback(info, scb->baton);
		}
	};

	// Vcs::CancelCallback
	
	svn_error_t* CancelCallback(void* baton)
	{
		VcsContext* context = (VcsContext*)baton;

		if (context && context->IsCancelled())
			return svn_error_create(SVN_ERR_CANCELLED, 0, "User cancelled");

		return SVN_NO_ERROR;
	}

	// Vcs::AuthorisationCallback

	svn_error_t* AuthorisationCallback(svn_auth_cred_simple_t** cred, void* baton, const char* realm, const char* username, svn_boolean_t may_save, apr_pool_t* pool)
	{
		VcsContext* context = (VcsContext*)baton;
		
		if (context)
		{
			Sz8		loginUser	= username ? username : "";
			Sz8		loginPass	= "";
			Sz8		loginRealm	= realm;
			bool	loginSave	= (may_save != 0);

			context->GetLogin(loginUser, loginPass, loginRealm, loginSave);

			(*cred) = (svn_auth_cred_simple_t*)apr_palloc(pool, sizeof(*cred));

			svn_utf_cstring_to_utf8(&(*cred)->username, loginUser.Text(), pool);		
			svn_utf_cstring_to_utf8(&(*cred)->password, loginPass.Text(), pool);

			(*cred)->may_save = loginSave;
		}

		return SVN_NO_ERROR;
	}

	// Vcs::LogCallback

	svn_error_t* LogCallback(const char** log_msg, const char** tmp_file, const apr_array_header_t* commit_items, void* baton, apr_pool_t* pool)
	{
		(*log_msg) = baton ? (const char*)baton : "";

		return SVN_NO_ERROR;
	}

	// Vcs::Svn

	Svn::Svn(VcsContext* context) :
		_context(0),
		_clientPool(0),
		_clientContext(0),
		_initialised(false)
	{
		Initialise();
	}

	Svn::~Svn()
	{
		Destroy();
	}

	// Vcs::Svn::Checkout

	Status Svn::Checkout(const Sz8& url, const Sz8& path, E_Depth depth, int revision, NotifyCallback callback, void* baton)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		const char*			inUrl				= svn_path_internal_style(url.Text(), pool);
		const char*			inPath				= svn_path_internal_style(path.Text(), pool);
		svn_opt_revision_t	peg					= { svn_opt_revision_unspecified };
		svn_opt_revision_t	rev					= { svn_opt_revision_head };
		bool				ignoreExternals		= false;
		bool				allowObstructions	= false;

		if (revision >= 0)
		{
			rev.kind			= svn_opt_revision_number;
			rev.value.number	= revision;
		}

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		NotifyCallbackBaton ncb(callback, baton);

		ctx.notify_func2	= NotifyCallbackBaton::Proc;
		ctx.notify_baton2	= &ncb;

		// Call

		SvnError error = svn_client_checkout3(0, inUrl, inPath, &peg, &rev, GetDepth(depth), ignoreExternals, allowObstructions, &ctx, pool);

		return AsStatus(error);
	}

	// Vcs::Svn::Update

	Status Svn::Update(const Sz8& path, E_Depth depth, int revision, NotifyCallback callback, void* baton)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		AprArray<const char*> inPaths;

		inPaths.Make(pool, 1);
		inPaths.PushBack(svn_path_internal_style(path, pool));

		svn_opt_revision_t	rev					= { svn_opt_revision_head };
		bool				depthIsSticky		= false;
		bool				ignoreExternals		= false;
		bool				allowObstructions	= false;

		if (revision >= 0)
		{
			rev.kind			= svn_opt_revision_number;
			rev.value.number	= revision;
		}

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		NotifyCallbackBaton ncb(callback, baton);

		ctx.notify_func2	= NotifyCallbackBaton::Proc;
		ctx.notify_baton2	= &ncb;

		// Call

		SvnError error = svn_client_update3(0, inPaths, &rev, GetDepth(depth), depthIsSticky, ignoreExternals, allowObstructions, &ctx, pool);

		return AsStatus(error);
	}
	
	// Vcs::Svn::Revert

	Status Svn::Revert(const Sz8& path, E_Depth depth, NotifyCallback callback, void* baton)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		AprArray<const char*> inPaths;

		inPaths.Make(pool, 1);
		inPaths.PushBack(svn_path_internal_style(path, pool));

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		NotifyCallbackBaton ncb(callback, baton);

		ctx.notify_func2	= NotifyCallbackBaton::Proc;
		ctx.notify_baton2	= &ncb;

		// Call

		SvnError error = svn_client_revert2(inPaths, GetDepth(depth), 0, &ctx, pool);

		return AsStatus(error);
	}

	// Vcs::Svn::Add

	Status Svn::Add(const Sz8& path, E_Depth depth, NotifyCallback callback, void* baton)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		const char*	inPath		= svn_path_internal_style(path.Text(), pool);
		bool		force		= false;
		bool		noIgnore	= false;
		bool		addParents	= false;

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		NotifyCallbackBaton ncb(callback, baton);

		ctx.notify_func2	= NotifyCallbackBaton::Proc;
		ctx.notify_baton2	= &ncb;

		// Call

		SvnError error = svn_client_add4(inPath, GetDepth(depth), force, noIgnore, addParents, &ctx, pool);

		return AsStatus(error);
	}

	// Vcs::Svn::Delete

	Status Svn::Delete(const Sz8& path, NotifyCallback callback, void* baton)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		AprArray<const char*> inPaths;

		inPaths.Make(pool, 1);
		inPaths.PushBack(svn_path_internal_style(path, pool));

		bool force		= true;
		bool keepLocal	= false;

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		NotifyCallbackBaton ncb(callback, baton);

		ctx.notify_func2	= NotifyCallbackBaton::Proc;
		ctx.notify_baton2	= &ncb;

		// Call

		// TODO: Revert if text_status == added/replaced

		SvnError error = svn_client_delete3(0, inPaths, force, keepLocal, 0, &ctx, pool);

		return AsStatus(error);
	}

	// Vcs::Svn::Copy

	Status Svn::Copy(const Sz8& src, const Sz8& dst, NotifyCallback callback, void* baton)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		AprArray<svn_client_copy_source_t> inSrcs;

		inSrcs.Make(pool, 1);

		if (svn_client_copy_source_t* cs = inSrcs.Append())
		{
			cs->path			= svn_path_internal_style(src, pool);
			cs->revision		= 0;
			cs->peg_revision	= 0;
		}
		else
			return Status(Status::kOutOfMemory);

		const char*	inDst		= svn_path_internal_style(dst, pool);
		bool		copyAsChild	= false;
		bool		makeParents	= false;

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		NotifyCallbackBaton ncb(callback, baton);

		ctx.notify_func2	= NotifyCallbackBaton::Proc;
		ctx.notify_baton2	= &ncb;

		// Call

		SvnError error = svn_client_copy4(0, inSrcs, inDst, copyAsChild, makeParents, 0, &ctx, pool);

		return AsStatus(error);
	}

	Status Svn::Move(const Sz8& src, const Sz8& dst, NotifyCallback callback, void* baton)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		AprArray<const char*> inSrcs;

		inSrcs.Make(pool, 1);
		inSrcs.PushBack(svn_path_internal_style(src, pool));

		const char*	inDst		= svn_path_internal_style(dst, pool);
		bool		force		= false;
		bool		moveAsChild	= false;
		bool		makeParents	= false;

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		NotifyCallbackBaton ncb(callback, baton);

		ctx.notify_func2	= NotifyCallbackBaton::Proc;
		ctx.notify_baton2	= &ncb;

		// Call

		SvnError error = svn_client_move5(0, inSrcs, inDst, force, moveAsChild, makeParents, 0, &ctx, pool);

		return AsStatus(error);
	}

	// Vcs::Svn::Commit

	Status Svn::Commit(const Sz8& path, const Sz8& log, E_Depth depth, NotifyCallback callback, void* baton)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		AprArray<const char*> inPaths;

		inPaths.Make(pool, 1);
		inPaths.PushBack(svn_path_internal_style(path, pool));

		bool keepLocks			= false;
		bool keepChangelists	= false;

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		NotifyCallbackBaton ncb(callback, baton);

		ctx.notify_func2	= NotifyCallbackBaton::Proc;
		ctx.notify_baton2	= &ncb;

		ctx.log_msg_func3	= LogCallback;

		if (SvnError error = svn_utf_cstring_to_utf8((const char**)&ctx.log_msg_baton3, log, pool))
			return AsStatus(error);

		// Call

		svn_commit_info_t* ci = 0;

		SvnError error = svn_client_commit4(&ci, inPaths, GetDepth(depth), keepLocks, keepChangelists, 0, 0, &ctx, pool);

		return AsStatus(error);
	}

	Status Svn::Commit(XtUt::Array<Sz8>* paths, const Sz8& log, E_Depth depth, NotifyCallback callback, void* baton)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		AprArray<const char*> inPaths;

		inPaths.Make(pool, Count(paths));

		for(XtUt::Array<Sz8>::Iterator it(paths); it; ++it)
			inPaths.PushBack(svn_path_internal_style(*it, pool));

		bool keepLocks			= false;
		bool keepChangelists	= false;

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		NotifyCallbackBaton ncb(callback, baton);

		ctx.notify_func2	= NotifyCallbackBaton::Proc;
		ctx.notify_baton2	= &ncb;

		ctx.log_msg_func3	= LogCallback;

		if (SvnError error = svn_utf_cstring_to_utf8((const char**)&ctx.log_msg_baton3, log, pool))
			return AsStatus(error);

		// Call

		svn_commit_info_t* ci = 0;

		SvnError error = svn_client_commit4(&ci, inPaths, GetDepth(depth), keepLocks, keepChangelists, 0, 0, &ctx, pool);

		return AsStatus(error);
	}

	// Vcs::Svn::Info

	Status Svn::Info(const Sz8& path, E_Depth depth, StatusCallback callback, void* baton)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		const char*	inPath			= svn_path_internal_style(path.Text(), pool);
		bool		getAll			= true;
		bool		update			= false;
		bool		noIgnore		= false;
		bool		ignoreExternals	= false;

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		StatusCallbackBaton scb(callback, baton);

		// Call

		SvnError error = svn_client_status3(0, inPath, 0, StatusCallbackBaton::Proc, &scb, GetDepth(depth), getAll, update, noIgnore, ignoreExternals, 0, &ctx, pool);

		return AsStatus(error);
	}

	// Vcs::Svn::Cleanup

	Status Svn::Cleanup(const Sz8& path)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		const char* inPath = svn_path_internal_style(path.Text(), pool);

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		// Call

		SvnError error = svn_client_cleanup(inPath, &ctx, pool);

		return AsStatus(error);
	}

	// Vcs::Svn::SetProperty

	Status Svn::SetProperty(const Sz8& path, const Sz8& prop, const Sz8& val, E_Depth depth)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		const char*	inProp		= 0;
		const char*	inVal		= 0;
		const char*	inPath		= svn_path_internal_style(path, pool);
		bool		skipChecks	= false;

		if (SvnError error = svn_utf_cstring_to_utf8(&inProp, prop, pool))
			return AsStatus(error);

		if (SvnError error = svn_utf_cstring_to_utf8(&inVal, val, pool))
			return AsStatus(error);

		// TODO: svn_prop_needs_translation + svn_subst_translate_string for strVal

		svn_string_t* strVal = svn_string_create(inVal, pool);

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		// Call

		SvnError error = svn_client_propset3(0, inProp, strVal, inPath, GetDepth(depth), skipChecks, SVN_INVALID_REVNUM, 0, 0, &ctx, pool);

		return AsStatus(error);
	}

	// Vcs::Svn::GetProperty

	Status Svn::GetProperty(const Sz8& path, const Sz8& prop, Sz8& outVal)
	{
		outVal.Clear();

		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		const char* inProp = 0;
		const char* inPath = svn_path_internal_style(path, pool);

		if (SvnError error = svn_utf_cstring_to_utf8(&inProp, prop, pool))
			return AsStatus(error);

		// Context

		svn_client_ctx_t ctx = *_clientContext;

		// Call

		apr_hash_t* props = 0;

		if (SvnError error = svn_client_propget3(&props, inProp, inPath, 0, 0, 0, svn_depth_empty, 0, &ctx, pool))
			return AsStatus(error);

		// Results

		apr_hash_index_t* index = apr_hash_first(pool, props);

		if (!index)
			return Status(Status::kFailed, "Property isn't set");

		svn_string_t* strVal = 0;

		apr_hash_this(index, 0, 0, (void**)&strVal);

		if (strVal)
			outVal = strVal->data;

		return Status(Status::kSuccess);
	}

	// Vcs::Svn::GetPristineCopy

	Status Svn::GetPristineCopy(const Sz8& path, Sz8& outPath)
	{
		if (!_initialised)
			return Status(Status::kNotInitialised);

		AprPool pool(_clientPool);

		// Params

		const char* inPath = svn_path_internal_style(path, pool);

		// Call

		const char* inPristine = 0;

		if (SvnError error = svn_wc_get_pristine_copy_path(inPath, &inPristine, pool))
			return AsStatus(error);

		if (!inPristine)
			return Status(Status::kFailed);

		outPath = inPristine;

		return Status(Status::kSuccess);
	}

	// Vcs::Svn::Initialise

	void Svn::Initialise()
	{
		// Svn init

		C_ExceptionFilterHelper helper;

		static bool onlyOnce = false;

		if (!onlyOnce)
		{
			if (svn_cmdline_init("svn", 0) != EXIT_SUCCESS)
				return;

			onlyOnce = true;
		}

		// Allocator and pool

		apr_allocator_t* allocator = 0;

		if (apr_allocator_create(&allocator))
			return;

		apr_allocator_max_free_set(allocator, SVN_ALLOCATOR_RECOMMENDED_MAX_FREE);

		_clientPool = svn_pool_create_ex(0, allocator);

		apr_allocator_owner_set(allocator, _clientPool);

		// Context

		if (SvnError error = svn_client_create_context(&_clientContext, _clientPool))
			return;

		_clientContext->cancel_func		= &CancelCallback;
		_clientContext->cancel_baton	= _context;

		// Authorisation callback

		AprArray<svn_auth_provider_object_t*> providers;

		if (!providers.Make(_clientPool, 10))
			return;

		svn_auth_provider_object_t* provider = 0;

		svn_auth_get_windows_simple_provider(&provider, _clientPool);										providers.PushBack(provider);
		svn_auth_get_simple_provider(&provider, _clientPool);												providers.PushBack(provider);
		svn_auth_get_username_provider(&provider, _clientPool);												providers.PushBack(provider);
		svn_auth_get_ssl_server_trust_file_provider(&provider, _clientPool);								providers.PushBack(provider);
		svn_auth_get_ssl_client_cert_file_provider(&provider, _clientPool);									providers.PushBack(provider);
		svn_auth_get_ssl_client_cert_pw_file_provider(&provider, _clientPool);								providers.PushBack(provider);
		svn_auth_get_simple_prompt_provider(&provider, &AuthorisationCallback, _context, 2, _clientPool);	providers.PushBack(provider);

		svn_auth_baton_t* ab = 0;
		svn_auth_open(&ab, providers.Detach(), _clientPool);

		Sz8 configPath = Sz8(std::getenv("APPDATA")) + "\\Subversion";

		const char* configPathString = 0;
		svn_utf_cstring_to_utf8(&configPathString, configPath, _clientPool);

		svn_config_ensure(configPathString, _clientPool);

		svn_auth_set_parameter(ab, SVN_AUTH_PARAM_CONFIG_DIR, configPathString);

		_clientContext->auth_baton = ab;

		_initialised = true;
	}

	// Vcs::Svn::Destroy

	void Svn::Destroy()
	{
		if (_clientPool)
			svn_pool_destroy(_clientPool);

		delete _context;

		_context			= 0;
		_clientPool			= 0;
		_clientContext		= 0;
		_initialised		= false;
	}

}