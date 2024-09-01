#ifndef INIT_H
#define INIT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define Initialize init
#define initialize init

static u32 *xfb;
static GXRModeObj *rmode;

void init();
#endif // INIT_H