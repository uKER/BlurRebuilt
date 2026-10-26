/*
 * This file is not part of Jam
 */

/*
 * hcache.h - handle #includes in source files
 */

void hcache_init(void);
void hcache_done(void);
LIST *hcache(TARGET *t, LIST *hdrscan );

/* from headers.c */
LIST *headers1( const char *file, LIST *hdrscan );

