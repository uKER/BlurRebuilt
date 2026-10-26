#ifndef SORCERYEXPORT_H
#define SORCERYEXPORT_H

#ifdef _MSC_VER
#pragma once
#endif

void SorceryExport(SorceryApp& app, bool dataOnly, bool dontDeploy, Xt::DomPack* pack);

#endif // SORCERYEXPORT_H