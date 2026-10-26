extern int InitStreamPlay( int sampleRate, int nrChannels );
extern int PlayBlock( char *data, int nrBytes );
extern void CloseStreamPlay();
extern int playBack( char *filename );
