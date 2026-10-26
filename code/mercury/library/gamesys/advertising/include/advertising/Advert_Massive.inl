
void* MassiveAlloc(size_t size)
{
	MEMORY_TRACKER_CONTEXT("advertising");

	return new BYTE[size];
}

void MassiveFree(void* ptr)
{
	delete [] (BYTE*)ptr;
}

void CallbackTeleportToZone(const char *szZoneName)
{
	JumpToZoneCallback* jtzCallback = AdvertManager::Instance().GetJumpToZoneCallback();

	if (jtzCallback)
	{
		jtzCallback(szZoneName);
	}
}

void CallbackTeleportWithinZone(const MAD_VECTOR3 *pvPosition, const MAD_VECTOR3 *pvFacing, const MAD_VECTOR3 *pvUp)
{
	Cam2::C_DebugFeed& debugFeed = Framework::C_ApplicationBase::GetInstance()->GetDebugFeed(0);

	debugFeed.SetDebugType(Cam2::E_FlyCamera);

	const Maths::HVec3 hpos = Maths::HVec3(pvPosition->x, pvPosition->y, pvPosition->z);
	const Maths::HVec3 hdir = Maths::HVec3(pvFacing->x, pvFacing->y, pvFacing->z);

	Cam2::C_Camera cam;
	cam.SetPosition(hpos);
	cam.LookAt(hpos + hdir);

	debugFeed.CopyFrom(cam);
}
