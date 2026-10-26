/*
 * Copyright 1993, 1995 Christopher Seiwald.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

/*
 * make.h - bring a target up to date, once rules are in place
 *
 * 11/04/02 (seiwald) - const-ing for string literals
 */

int make( int n_targets, const char **targets, int anyhow );
int make1( TARGET *t );
int makegetdirectdependents( TARGET *t, const char *pszSingleTarget, const char **ppTargets, int *pnTargets);
int make1gettargetfrommd5cache(TARGET **t, LIST *filecache, int *pAllcached);
LIST *collectmd5components(TARGET *t, LIST *pList);
void setthectrlhandler();