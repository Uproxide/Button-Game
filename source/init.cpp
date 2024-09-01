#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fat.h>
#include <wiiuse/wpad.h>
#include "init.h"

#define MEM_K0_TO_K1(x) (u32*)((u32)(x) + (SYS_BASE_UNCACHED - SYS_BASE_CACHED))

void init() {
    VIDEO_Init();
    WPAD_Init();
    fatInitDefault();

    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
}