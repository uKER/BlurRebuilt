#ifndef FINDDEPENDENCIES_H
#define FINDDEPENDENCIES_H

#ifdef _MSC_VER
#pragma once
#endif

void FindDependencies(LarvaSchema* schema, XtId dependsOnId, Xt::DomRecord* record, Xt::QueryElement* queryElement, Xt::QueryContext* queryContext);

#endif // FINDDEPENDENCIES_H