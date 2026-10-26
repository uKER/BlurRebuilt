#ifndef VISUALCONFLICTRESOLVER_H
#define VISUALCONFLICTRESOLVER_H

#ifdef _MSC_VER
#pragma once
#endif

bool VisualConflictResolver(
		HWND parentWindow,
		LarvaSchema* schema,
		Xt::Change* chgMerged, Xt::Change* chgLhs, Xt::Change* chgRhs,
		Xt::DomElement* ancestor, Xt::DomElement* lhs, Xt::DomElement* rhs
	);

#endif // VISUALCONFLICTRESOLVER_H