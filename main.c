// Cheat: Press "B" to enter free-movement and use "UP" and "DOWN" to move!
#include "myLib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "startbg.h"
#include "pausebg.h"
#include "gamebg1.h"
#include "instructionbg.h"
#include "losebg.h"
#include "spritesheet.h"
#include "game.h"
#include "sound.h"
#include "gamebg2.h"
#include "spritesheet2.h"

#include "TitleSong.h"
#include "GameSong.h"
#include "CoinSFX.h"
#include "LoseSong.h"

// Prototypes
void initialize();

// State Prototypes
void goToStart();
void start();
void goToInstructions();
void instructions();
void goToGame();
void resumeGame();
void game();
void goToPause();
void pause();
void goToLose();
void lose();

// States
enum {START, INSTRUCTIONS, GAME, PAUSE, LOSE};
int state;

// Button Variables
unsigned short buttons;
unsigned short oldButtons;

// Random Seed
int seed;

// Offset Variables
int hOff = 0;
int vOff = 0;
int hOffTemp = 0;

// OAM
OBJ_ATTR shadowOAM[128];

// Miscellaneous Variables
int arrFlag;
int loseTimer;
int playTimer;
int score;
int maxZappers;

int main() {

    initialize(); 

	while(1) {

        // Update button variables
        oldButtons = buttons;
        buttons = BUTTONS;
                
        // State Machine
        switch(state) {

            case START:
                start();
                break;
            case INSTRUCTIONS:
            	instructions();
            	break;
            case GAME:
                game();
                break;
            case PAUSE:
                pause();
                break;
            case LOSE:
                lose();
                break;
        }
	}

	return 0;
}


void initialize() {

    // Set up BG0 and BG1
    hideSprites();
    REG_DISPCTL = MODE0 | BG0_ENABLE | SPRITE_ENABLE;
    REG_DISPCTL = REG_DISPCTL | BG1_ENABLE;
    DMANow(3, shadowOAM, OAM, 128*4);

    score = 0;


    // Set up the first state
    goToStart();

    // Set up Sounds and Timers
    setupSounds();
    setupInterrupts();
}

// Sets up the start state
void goToStart() {

	hOff = 0;

	REG_BG0HOFF = hOff;

    // Initialize sprite palette
    DMANow(3, spritesheetPal, SPRITEPALETTE, 256);
    DMANow(3, spritesheetTiles, &CHARBLOCK[4], spritesheetTilesLen/2);

	// Load the start background
	DMANow(3, startbgPal, PALETTE, 256);
	DMANow(3, startbgTiles, &CHARBLOCK[0], startbgTilesLen/2);
	DMANow(3, startbgMap, &SCREENBLOCK[30], startbgMapLen/2);

	REG_BG0CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(30) | BG_8BPP | BG_SIZE_SMALL;

	state = START;

	seed = 0;

    // Play looping title song when going to game screen
    playSoundA(TitleSong,TITLESONGLEN,TITLESONGFREQ, 1);

    arrFlag = 0;

    // Draw the arrow sprite at its default location
    shadowOAM[127].attr0 = 100 | ATTR0_4BPP | ATTR0_SQUARE;
    shadowOAM[127].attr1 = 64 | ATTR1_TINY;
    shadowOAM[127].attr2 = ATTR2_TILEID(0,8);

	initGame();

}

// Runs every frame of the start state
void start() {

	seed++;

    // Draw the arrow sprite on screen depending on what button is pressed
    if (BUTTON_PRESSED(BUTTON_DOWN)) {
        shadowOAM[127].attr0 = 120 | ATTR0_4BPP | ATTR0_SQUARE;
        shadowOAM[127].attr1 = 64 | ATTR1_TINY;
        shadowOAM[127].attr2 = ATTR2_TILEID(0,8);
        arrFlag = 1;
    } else if (BUTTON_PRESSED(BUTTON_UP)) {
        shadowOAM[127].attr0 = 100 | ATTR0_4BPP | ATTR0_SQUARE;
        shadowOAM[127].attr1 = 64 | ATTR1_TINY;
        shadowOAM[127].attr2 = ATTR2_TILEID(0,8);
        arrFlag = 0;
    }

    // Determine whether to go to game or instructions
    if (BUTTON_PRESSED(BUTTON_A)) {
        if (arrFlag) {
            srand(seed);
            goToInstructions();
        } else {
            srand(seed);
            goToGame();
        }
    }

    waitForVBlank();

    DMANow(3, shadowOAM, OAM, 128*4);

}

// Sets up the instructions state
void goToInstructions() {

	state = INSTRUCTIONS;

	// Load the instructions background
	DMANow(3, instructionbgPal, PALETTE, 256);
	DMANow(3, instructionbgTiles, &CHARBLOCK[0], instructionbgTilesLen/2);
	DMANow(3, instructionbgMap, &SCREENBLOCK[30], instructionbgMapLen/2);

	REG_BG0CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(30) | BG_8BPP | BG_SIZE_SMALL;

}

// Runs every frame of the instructions state
void instructions() {

	// Handle state transitions
    if (BUTTON_PRESSED(BUTTON_START)) {
        goToGame();
        } else if (BUTTON_PRESSED(BUTTON_SELECT)) {
    	goToStart();
    }

}
    
// Sets up the game state
void goToGame() {

	state = GAME;

    playTimer = 0;

    DMANow(3, spritesheetPal, SPRITEPALETTE, 256);
    DMANow(3, spritesheetTiles, &CHARBLOCK[4], spritesheetTilesLen/2);

    // Set up multiple backgrounds
	REG_BG0CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(30) | BG_4BPP | BG_SIZE_WIDE;
    REG_BG1CNT = BG_CHARBLOCK(1) | BG_SCREENBLOCK(28) | BG_4BPP | BG_SIZE_WIDE;

    // Play the game music
    playSoundA(GameSong, GAMESONGLEN, GAMESONGFREQ, 1);

	// Load the game backgrounds
	DMANow(3, gamebg2Pal, PALETTE, 256);
	DMANow(3, gamebg1Tiles, &CHARBLOCK[0], gamebg1TilesLen/2);
	DMANow(3, gamebg1Map, &SCREENBLOCK[30], gamebg1MapLen/2);

    DMANow(3, gamebg2Tiles, &CHARBLOCK[1], gamebg2TilesLen/2);
    DMANow(3, gamebg2Map, &SCREENBLOCK[28], gamebg2MapLen/2);

	hideSprites();
	DMANow(3, shadowOAM, OAM, 128*4);
}

// Resume the game rather than start over
void resumeGame() {

    state = GAME;

    playTimer = 0;

    // Set up multiple backgrounds
    REG_BG0CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(30) | BG_4BPP | BG_SIZE_WIDE;
    REG_BG1CNT = BG_CHARBLOCK(1) | BG_SCREENBLOCK(28) | BG_4BPP | BG_SIZE_WIDE;

    // Load the game backgrounds
    DMANow(3, gamebg2Pal, PALETTE, 256);
    DMANow(3, gamebg1Tiles, &CHARBLOCK[0], gamebg1TilesLen/2);
    DMANow(3, gamebg1Map, &SCREENBLOCK[30], gamebg1MapLen/2);

    DMANow(3, gamebg2Tiles, &CHARBLOCK[1], gamebg2TilesLen/2);
    DMANow(3, gamebg2Map, &SCREENBLOCK[28], gamebg2MapLen/2);

    hideSprites();
    DMANow(3, shadowOAM, OAM, 128*4);
}

// Runs every frame of the game state
void game() {

	updateGame();
	drawGame();

	waitForVBlank();

	DMANow(3, shadowOAM, OAM, 128*4);


	// State transitions
    if (BUTTON_PRESSED(BUTTON_START)) {
        hOffTemp = hOff;
        hOff = 0;
        goToPause();
    }
    else if (gameOver)
        goToLose();

    // Update hOff and use parallax
    REG_BG0HOFF = hOff;
    REG_BG1HOFF = hOff/2;

}

// Sets up the pause state
void goToPause() {

	arrFlag = 0;

    hideSprites();

    REG_BG0CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(30) | BG_8BPP | BG_SIZE_SMALL;

	// Load the pause background
	DMANow(3, pausebgPal, PALETTE, 256);
	DMANow(3, pausebgTiles, &CHARBLOCK[0], pausebgTilesLen/2);
	DMANow(3, pausebgMap, &SCREENBLOCK[30], pausebgMapLen/2);

    // Load the arrow pointer sprite
    shadowOAM[127].attr0 = 99 | ATTR0_4BPP | ATTR0_SQUARE;
    shadowOAM[127].attr1 = 72 | ATTR1_TINY;
    shadowOAM[127].attr2 = ATTR2_TILEID(0,8);

    waitForVBlank();
	DMANow(3, shadowOAM, OAM, 128*4);
	
	state = PAUSE;
}

// Runs every frame of the pause state
void pause() {

    // Draw the arrow sprite on screen
    if (BUTTON_PRESSED(BUTTON_DOWN)) {
        shadowOAM[127].attr0 = 117 | ATTR0_4BPP | ATTR0_SQUARE;
        shadowOAM[127].attr1 = 72 | ATTR1_TINY;
        shadowOAM[127].attr2 = ATTR2_TILEID(0,8);
        arrFlag = 1;
    } else if (BUTTON_PRESSED(BUTTON_UP)) {
        shadowOAM[127].attr0 = 99 | ATTR0_4BPP | ATTR0_SQUARE;
        shadowOAM[127].attr1 = 72 | ATTR1_TINY;
        shadowOAM[127].attr2 = ATTR2_TILEID(0,8);
        arrFlag = 0;
    }

    // Determine whether to go to game or start
    if (BUTTON_PRESSED(BUTTON_A)) {
        if (arrFlag) {
            goToStart();
        } else {
            resumeGame();
        }
    }

    waitForVBlank();

    DMANow(3, shadowOAM, OAM, 128*4);
}

// Sets up the lose state
void goToLose() {

	hOff = 0;

    loseTimer = 0;

    arrFlag = 0;

    hideSprites();

    REG_BG0CNT = BG_CHARBLOCK(0) | BG_SCREENBLOCK(30) | BG_8BPP | BG_SIZE_SMALL;

    playSoundA(LoseSong, LOSESONGLEN, LOSESONGFREQ, 1);

    // Load the lose background
    DMANow(3, losebgPal, PALETTE, 256);
    DMANow(3, losebgTiles, &CHARBLOCK[0], losebgTilesLen/2);
    DMANow(3, losebgMap, &SCREENBLOCK[30], losebgMapLen/2);

    DMANow(3, spritesheet2Pal, SPRITEPALETTE, 256);
    DMANow(3, spritesheet2Tiles, &CHARBLOCK[4], spritesheetTilesLen/2);

    // Draw the player's score
    shadowOAM[24].attr0 = 55 | ATTR0_4BPP | ATTR0_SQUARE;
    shadowOAM[24].attr1 = 200 | ATTR1_SMALL;
    shadowOAM[24].attr2 = ATTR2_TILEID(2, 3 + 2*(score % 10));

    shadowOAM[25].attr0 = 55 | ATTR0_4BPP | ATTR0_SQUARE;
    shadowOAM[25].attr1 = 190 | ATTR1_SMALL;
    shadowOAM[25].attr2 = ATTR2_TILEID(2, 3 + 2*((score/10)%10));

    shadowOAM[26].attr0 = 55 | ATTR0_4BPP | ATTR0_SQUARE;
    shadowOAM[26].attr1 = 180 | ATTR1_SMALL;
    shadowOAM[26].attr2 = ATTR2_TILEID(2, 3 + 2*(((score/10)/10)%10));

    // Draw the arrow sprite
    shadowOAM[127].attr0 = 96 | ATTR0_4BPP | ATTR0_SQUARE;
    shadowOAM[127].attr1 = 107 | ATTR1_TINY;
    shadowOAM[127].attr2 = ATTR2_TILEID(0,8);

    waitForVBlank();
    DMANow(3, shadowOAM, OAM, 128*4);
    
    state = LOSE;
}

// Runs every frame of the lose state
void lose() {

	// Draw the arrow sprite on screen
    if (BUTTON_PRESSED(BUTTON_DOWN)) {
        shadowOAM[127].attr0 = 114 | ATTR0_4BPP | ATTR0_SQUARE;
        shadowOAM[127].attr1 = 107 | ATTR1_TINY;
        shadowOAM[127].attr2 = ATTR2_TILEID(0,8);
        arrFlag = 1;
    } else if (BUTTON_PRESSED(BUTTON_UP)) {
        shadowOAM[127].attr0 = 96 | ATTR0_4BPP | ATTR0_SQUARE;
        shadowOAM[127].attr1 = 107 | ATTR1_TINY;
        shadowOAM[127].attr2 = ATTR2_TILEID(0,8);
        arrFlag = 0;
    }

    // Determine whether to go to game or start
    if (BUTTON_PRESSED(BUTTON_A) && loseTimer > 60) {
        if (arrFlag) {
            gameOver = 0;
            goToStart();
        } else {
            gameOver = 0;
            initGame();
            goToGame();
        }
    }

    waitForVBlank();

    DMANow(3, shadowOAM, OAM, 128*4);

    loseTimer++;

}