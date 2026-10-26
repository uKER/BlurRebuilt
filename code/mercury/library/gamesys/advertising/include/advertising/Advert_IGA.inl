
char script_buffer[64*1024];

const char* IGAErrorToString(int rc)
{
	switch (rc)
	{
	case RAD_SUCCESS:
		return "RAD_SUCCESS";
	case RAD_ERROR_INTERNALERROR:
		return "RAD_ERROR_INTERNALERROR";
	case RAD_ERROR_COMMUNICATION:
		return "RAD_ERROR_COMMUNICATION";
	case RAD_ERROR_NOCONTENT:
		return "RAD_ERROR_NOCONTENT";
	case RAD_ERROR_CACHE:
		return "RAD_ERROR_CACHE";
	case RAD_ERROR_SYSTEM:
		return "RAD_ERROR_SYSTEM";
	case RAD_ERROR_CANCELLED:
		return "RAD_ERROR_CANCELLED";
	case RAD_ERROR_NOTINITIALIZED:
		return "RAD_ERROR_NOTINITIALIZED";
	case RAD_ERROR_OUTOFMEMORY:
		return "RAD_ERROR_OUTOFMEMORY";
	case RAD_ERROR_ABORTED:
		return "RAD_ERROR_ABORTED";
	case RAD_ERROR_FILEACCESS:
		return "RAD_ERROR_FILEACCESS";
	case RAD_ERROR_INVALID_RESTART:
		return "RAD_ERROR_INVALID_RESTART";
	case RAD_ERROR_FILECORRUPT:
		return "RAD_ERROR_FILECORRUPT";
	case RAD_ERROR_INVALID_DIR:
		return "RAD_ERROR_INVALID_DIR";
	case RAD_ERROR_INVALID_PARAMETER:
		return "RAD_ERROR_INVALID_PARAMETER";
	case RAD_ERROR_PARTIAL:
		return "RAD_ERROR_PARTIAL";
	case RAD_ERROR_DECRYPT:
		return "RAD_ERROR_DECRYPT";
	case RAD_ERROR_DISABLED:
		return "RAD_ERROR_DISABLED";
	default:
		return "?";
	}
}

void my_startfunc(RADSESSION *sess, int rc, void *userdata)
{
	AdvertManager::Instance().SetSystemEnabled(rc == RAD_SUCCESS);
	
	if (rc == RAD_SUCCESS)
	{
		LOGDEBUG(gLog, "Advertising: my_startfunc(): (%s - %d) userdata=%s", IGAErrorToString(rc), rc, (char *)userdata);
	}
	else
	{
		LOGERROR(gLog, "Advertising: my_startfunc(): (%s - %d) userdata=%s", IGAErrorToString(rc), rc, (char *)userdata);
	}
}

void my_progressfunc(RADSESSION *sess, int progress, int total, void *userdata)
{
   LOGDEBUG(gLog, "Advertising: my_processfunc(): [%0.0f%%]", (float)progress*100.0 / total);
}

void my_completefunc(RADSESSION *sess, int rc, void *userdata)
{
   LOGDEBUG(gLog, "Advertising: my_completefunc(): (%s - %d) userdata: %s", IGAErrorToString(rc), rc, (char *)userdata);
}

void my_refreshCacheCompleteFunc(RADSESSION *sess, int rc, void *userdata)
{
	AdvertManager* am = (AdvertManager*)userdata;

	LOGDEBUG(gLog, "Advertising: RefreshCacheComplete: (%s - %d)", IGAErrorToString(rc), rc);

	am->SetRefreshCacheDone();
	
}

void my_sendcompletefunc(RADSESSION *sess, int rc, void *userdata)
{
   LOGDEBUG(gLog, "Advertising: my_sendcompletefunc(): (%s - %d) userdata: %s", IGAErrorToString(rc), rc, (char *)userdata);
}

void my_debugfunc(struct RADSESSION *sess, int type, const char *message)
{
   LOGDEBUG(gLog, "Advertising: > %s", message);
}
