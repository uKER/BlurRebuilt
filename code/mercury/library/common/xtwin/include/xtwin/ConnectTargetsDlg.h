#ifndef CONNECTTO_H
#define CONNECTTO_H

#ifdef _MSC_VER
#pragma once
#endif

enum CtMethod
{
	kCtDefaultXbox,
	kCtDefaultPc,
	kCtSearch,
	kCtHostPort
};

struct CtTarget
{
	CtMethod method;
	XtString host;
	XtString port;

public:
	CtTarget() :
		method(kCtDefaultXbox)
	{
	}
};

/*struct CtProfile
{
	Sz8						name;
	XtUt::List<CtTarget>	targets;
};

struct CtApp
{
	XtString app;
	XtString profile;
};*/

struct ConnectTargets
{
	CtTarget target;

	/*XtUt::List<CtProfile>		profiles;
	XtUt::List<CtAppProfile>	apps;*/
};

ConnectTargets* ImportConnectTargetsFromRegistry();
void ExportConnectTargetsToRegistry(ConnectTargets* ct);

#endif // CONNECTTO_H