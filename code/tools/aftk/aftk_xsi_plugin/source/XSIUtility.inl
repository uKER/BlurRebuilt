namespace XSITrans
{

bool IsNumberValid(F32 number, F32 absNumericRange)
{
	if (_finite(number) == 0)
		return false;
	if (fabsf(number) > absNumericRange)
		return false;
	return true;
}

bool IsNumberValid(F32 number)
{
	if (_finite(number) == 0)
		return false;
	return true;
}

}
