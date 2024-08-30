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


#define MEM_K0_TO_K1(x) (u32*)((u32)(x) + (SYS_BASE_UNCACHED - SYS_BASE_CACHED))

static u32 *xfb;
static GXRModeObj *rmode;

int frame = 0;
int frameCounter = 0;

int clicks = 0;
int cps = 0;
int cpc = 1;
int page = 1;

bool saving = false;
bool deletedata = false;

int upgradeIndex = 0;
int optionsIndex = 0;
int deleteDataIndex = 1;

int cpcPrice = 25;
int cpsPrice = 100;

void printUpgradesSelection(int index) {
    printf("\x1b[2J");
    printf("\x1b[H");

    switch (index) {
        case 0:
            printf("Upgrades\n------------------------\n> Upgrade CPC (%d Clicks)\nUpgrade CPS (%d Clicks)", cpcPrice, cpsPrice);
            break;
        case 1:
            printf("Upgrades\n------------------------\nUpgrade CPC (%d Clicks)\n> Upgrade CPS (%d Clicks)", cpcPrice, cpsPrice);
            break;
        default:
            printf("Upgrades\n------------------------\nUpgrade CPC (%d Clicks)\nUpgrade CPS (%d Clicks)", cpcPrice, cpsPrice);
            break;
    }
    printf("\n------------------------\nPage %d", page);
}

void printOptionsSelection(int index) {
    printf("\x1b[2J");
    printf("\x1b[H");

    switch (index) {
        case 0:
            printf("Options\n------------------------\n> Save Data\nErase Save");
            break;
        case 1:
            printf("Options\n------------------------\nSave Data\n> Erase Save");
            break;
        case 6:
            printf("ooga booga :3 this is coopers little easter egg for rn, to go back, press up on the dpad. :3");
            break;

        // KEEP THESE AT BOTTOM
        case 2:
            printf("Options > Erase Data\n------------------------\n> No thanks.\nYes I am sure.");
            break;
        case 3:
            printf("Options > Erase Data\n------------------------\nNo thanks.\n> Yes I am sure.");
            break;
        case 4:
            printf("Options > Erase Data\n------------------------\nYes I am REALLY sure.\n> No thanks.");
            break;
        case 5:
            printf("Options > Erase Data\n------------------------\n> Yes I am REALLY sure.\nNo thanks.");
            break;
        default:
            printf("Options\n------------------------\nSave Data\nErase Save");
            break;
    }
    printf("\n------------------------\nPage %d", page);
}

void printPage(int p) {
    printf("\x1b[2J");
    printf("\x1b[H");

    switch (p) {
        case 1:
            printf("Button Game\nVersion 1.0.0\n------------------------\nControls:\nA - Select/Click\nD-Pad - Change Page\n------------------------\nClicks: %d\nCPS: %d\nCPC: %d", clicks, cps, cpc);
			printf("\n------------------------\nPage %d", p);
            break;
        case 2:
            printUpgradesSelection(upgradeIndex);
            break;
		case 3:
            printOptionsSelection(optionsIndex);
            break;
		case 4:
			printf("Credits\n------------------------\nUproxide - Creator/Lead Developer\nCooper - Developer/Tester\nTheHighTide - Insporation\nMr. Bones - bones");
			printf("\n------------------------\nPage %d", p);
			break;
        default:
            printf("placeholder page... howd you get here?");
			printf("\n------------------------\nPage %d", p);
            break;
    }
}

void loadSave() {
	FILE* save = fopen("sd:/apps/ButtonGame/ithink.mayoisasillybeast", "r+");

	if (save == NULL) {
		// save doesnt exist! will be made on close :3
	} else {
		char buffer[1024];
		int line = 1;

		while (fgets(buffer, sizeof(buffer), save) != NULL) {
			switch (line)
			{
				case 1:
					clicks = std::stoi(buffer);
					break;
				case 2:
					cpc = std::stoi(buffer);
					break;
				case 3:
					cps = std::stoi(buffer);
					break;
				case 4:
					cpcPrice = std::stoi(buffer);
					break;
				case 5:
					cpsPrice = std::stoi(buffer);
					break;
				default:
					break;
			}
			line++;
			if (line > 5) {
				break;
			}
		}
	}
}

void saveData() {
	FILE* save = fopen("sd:/apps/ButtonGame/ithink.mayoisasillybeast", "w+");
	fprintf(save, "%d\n%d\n%d\n%d\n%d", clicks, cpc, cps, cpcPrice, cpsPrice);
	saving = true;
	fclose(save);
}

void deleteData() {
	if (remove("sd:/apps/ButtonGame/ithink.mayoisasillybeast") == 0) {
        clicks = 0;
        cpc = 1;
        cps = 0;
        cpcPrice = 25;
        cpsPrice = 100;
        printf("Data deleted successfully");
    } else
        printf("Unable to delete the file");
}

int main(int argc, char **argv) {
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

	loadSave();

    printPage(page);

    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_LEFT) {
            if (optionsIndex < 2) {
                if (page == 1) {
                    printPage(page);
                } else {
                    page -= 1;
                    printPage(page);
                }
            } else {
                //printf("\nPage %d\n", page);
                //printf("Options Index %d\n", optionsIndex);
            }
        }

        if (pressed & WPAD_BUTTON_RIGHT) {
            if (optionsIndex < 2) {
                #ifdef DEV
                page += 1;
                printPage(page);
                #else
                if (page == 4) {
                    printPage(page);
                } else {
                    page += 1;
                    printPage(page);
                }
                #endif
            }

        }

        if (pressed & WPAD_BUTTON_DOWN) {
            if (page == 2) {
                if (upgradeIndex == 1) {
                    printUpgradesSelection(upgradeIndex);
                } else {
                    if (!(upgradeIndex == 1)) {
                        upgradeIndex += 1;
                    }
                    printUpgradesSelection(upgradeIndex);
                }
            } else if (page == 3) { 
                if (optionsIndex == 2 || optionsIndex == 3) {
                    optionsIndex = 3;
                    printOptionsSelection(optionsIndex);
                } else if (optionsIndex == 4 || optionsIndex == 5) {
                    optionsIndex = 4;
                    printOptionsSelection(optionsIndex);
                } else if (optionsIndex == 1) {
                    optionsIndex = 6;
                    printOptionsSelection(optionsIndex);
                } else {
                    if (!(optionsIndex == 6)) {
                        optionsIndex += 1;
                    }
                    printOptionsSelection(optionsIndex);
                }
            } else {

			}
        } else if (pressed & WPAD_BUTTON_UP) {
            if (page == 2) {
                if (upgradeIndex == 0) {
                    printUpgradesSelection(upgradeIndex);
                } else {
                    upgradeIndex -= 1;
                    printUpgradesSelection(upgradeIndex);
                }
            } else if (page == 3) {
                if (optionsIndex == 2 || optionsIndex == 3) {
                    optionsIndex = 2;
                    printOptionsSelection(optionsIndex);
                } else if (optionsIndex == 4 || optionsIndex == 5) {
                    optionsIndex = 5;
                    printOptionsSelection(optionsIndex);
                } else if (optionsIndex == 0) {
                    printOptionsSelection(optionsIndex);
                } else if (optionsIndex == 6) {
                    optionsIndex = 1;
                    printOptionsSelection(optionsIndex);
                } else {
                    optionsIndex -= 1;
                    printOptionsSelection(optionsIndex);
                }
            } else {

			}
        }

        if (pressed & WPAD_BUTTON_HOME && optionsIndex < 2) {
			printf("\x1b[2J");
    		printf("\x1b[H");

			saveData();

			printf("Saved Data! have fun! bye bye! im uproxi");

			usleep(100000);

            return 0;
        }

        if (pressed & WPAD_BUTTON_A) {
            switch (page) {
                case 1:
                    clicks += cpc;
                    printPage(page);
                    break;
                case 2: 
                    if (upgradeIndex == 0 && clicks >= cpcPrice) {
                        cpc += 1;
                        clicks -= cpcPrice;
                        cpcPrice *= 1.2;
                        printUpgradesSelection(upgradeIndex);
                    } else if (upgradeIndex == 1 && clicks >= cpsPrice) {
                        cps += 1;
                        clicks -= cpsPrice;
                        cpsPrice *= 1.2;
                        printUpgradesSelection(upgradeIndex);
                    }
                    break;
				case 3:
					if (optionsIndex == 0) {
						saveData();
						printf("\nData Saved!");
					} else if (optionsIndex == 1) {
						optionsIndex = 2;
						printOptionsSelection(optionsIndex);
					} else if (optionsIndex == 3) {
						optionsIndex = 4;
						printOptionsSelection(optionsIndex);
					} else if (optionsIndex == 4 || optionsIndex == 2) {
						optionsIndex = 1;
						printOptionsSelection(optionsIndex);
					} else if (optionsIndex == 5) {
                        printf("\nDeleting Data...\n");
                        deleteData();
                        sleep(3);
                        page = 1;
                        optionsIndex = 0;
						printPage(page);
                    }
					break;
				/* case 4: 
					if (deleteDataIndex == 0) {
						deleteData();

						printf("\x1b[2J");
    					printf("\x1b[H");
						printf("Save Deleted.");
					} else {
						page == 3;
						printOptionsSelection(page);
					}
					break; */
                default:
                    break;
            }
        }

		if (pressed & WPAD_BUTTON_B) {
			clicks += cpc;
            printPage(page);
		};

        frameCounter++;
        if (frameCounter >= 60) {
            clicks += cps;
            frameCounter = 0;
            if (page == 1) {
                printPage(page);
            }
        }

        VIDEO_WaitVSync();
    }

    return 0;
}
