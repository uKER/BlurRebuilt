
#include "resource.h"

// Audio.cpp
int AudioProcess(unsigned char *data, int channels, int size);

// Config.cpp
int ConfigLoad();
int ConfigSave();

// Main.cpp
extern int dprintf(char *format, ...);
extern HWND MainWnd;
extern char MainPath[];
extern unsigned char *BitmapPixels;
extern int ShowImages;

int TrayChange(int type);
int TrayInfo(const char *text);
void MainShowImage();
