#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include "cJSON.h"

static u32 *xfb;
static GXRModeObj *rmode;


int score = 0;
int ScorePerClick = 1;
int dataLoaded = 0; // Flag to indicate if data has been loaded

// Function to load data from JSON file
int LoadDataFromJSON() {
    cJSON *root = NULL;
    FILE *file = fopen("config.json", "rb");
    
    if (file) {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *buffer = (char *)malloc(file_size + 1);
        fread(buffer, 1, file_size, file);
        fclose(file);

        buffer[file_size] = '\0';

        root = cJSON_Parse(buffer);
        free(buffer);
    }

    if (root) {
        cJSON *pointsPerClickItem = cJSON_GetObjectItem(root, "PointsPerClick");
        cJSON *pointsItem = cJSON_GetObjectItem(root, "Points");

        if (cJSON_IsNumber(pointsPerClickItem) && cJSON_IsNumber(pointsItem)) {
            ScorePerClick = pointsPerClickItem->valueint;
            score = pointsItem->valueint;
            dataLoaded = 1; // Set the flag to indicate data has been loaded
        }

        cJSON_Delete(root);
        return 1; // Data loaded successfully
    }

    return 0; // Failed to load data
}

// Function to save data to JSON file
void SaveDataToJSON() {
    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "PointsPerClick", ScorePerClick);
    cJSON_AddNumberToObject(root, "Points", score);

    char *jsonStr = cJSON_Print(root);

    FILE *file = fopen("config.json", "wb");
    if (file) {
        fputs(jsonStr, file);
        fclose(file);
    }

    free(jsonStr);
    cJSON_Delete(root);
}
void Initialise() {
  
	VIDEO_Init();
	WPAD_Init();
 
	rmode = VIDEO_GetPreferredMode(NULL);

	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
 
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
}


int main() {
 
	Initialise();
 
	printf("Button Game\nPress A to go up by 1 to start\nPress B when you are at 50 to upgrade points per click by 1.\nDeveloped By BasilLmao on Discord\nThanks to Goober for thinking its Cool :D\nGithub: Version: 0.2.1\nChangelogs:\n- Made it where you can close your game Lmfao\n");
	
	while(1) {

		WPAD_ScanPads();
		
		u16 buttonsDown = WPAD_ButtonsDown(0);
		
		if( buttonsDown & WPAD_BUTTON_A ) {
			score += ScorePerClick;
            printf("Score: %d\n", score);
		}	
		
		else if( buttonsDown & WPAD_BUTTON_B ) {
			if (score >= 50) {
				score -= 50; 
				ScorePerClick += 1;
				printf("Points Per Click No: %d\n", ScorePerClick);
			}
			else {
				printf("Not enough Points to upgrade.\n");
			}
		}	
		
		else if (buttonsDown & WPAD_BUTTON_HOME) {
			SaveDataToJSON();
			exit(0);
        }
		else if (buttonsDown & WPAD_BUTTON_DOWN) {
            SaveDataToJSON();
            printf("Data saved to JSON file.\n");
		
	}
 
	return 0;
}