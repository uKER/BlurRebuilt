template<typename PARAM_TYPE>
bool AsyncTask(HWND parent, bool (*callback)(class AsyncOp*, PARAM_TYPE*), PARAM_TYPE* pParams)
{
	C_ViewDisabler viewDisabler;

	if (parent)
		//1MB stack for safety
		return AsyncInvoke(parent, callback, pParams, 1024*1024);
	else
		return callback(0, pParams);
}
