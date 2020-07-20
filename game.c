#include <stdlib.h>
#include <stdio.h>
#include "myLib.h"
#include "game.h"
#include "spritesheet.h"
#include "sound.h"

#include "coinSFX.h"


// Variables
int gameOver;
int fallTimer;
int gravity;
int activeEasyCoins;
int activeZappers;
int soundTimer;
int gameTimer;
int score;
int cheatMode;
int lastZapperRow;
int lastZapperCol;
int playTimer;
int maxZappers;

// Game elements
ANISPRITE player;
ANISPRITE easyCoins[10];
ANISPRITE zappers[10];

// Button Variables
unsigned short buttons;
unsigned short oldButtons;

// Offset Variables
int hOff;
int vOff;

// Sound Variables
SOUND soundA;
SOUND soundB;

// OAM
OBJ_ATTR shadowOAM[128];

// Initialize the game
void initGame() {

	initPlayer();
    initEasyCoins();
    initZappers();

    // Initialize variables
    gameOver = 0;
    fallTimer = 1;
    activeEasyCoins = 0;
    activeZappers = 0;
    gravity = 25;
    soundTimer = 0;
    score = 0;
    gameTimer = 0;
    cheatMode = 0;
    lastZapperRow = 0;
    lastZapperCol = 0;
    playTimer = 0;
    maxZappers = 0;
}

// Updates the game each frame
void updateGame() {

	if (playTimer > 260) {
		shadowOAM[23].attr0 = ATTR0_HIDE;

		updatePlayer();
		updateEasyCoins();
		updateZappers();
	} else if (playTimer < 60) {
		shadowOAM[23].attr0 = (SCREENHEIGHT/2 - 16) | ATTR0_4BPP | ATTR0_TALL;
		shadowOAM[23].attr1 = (SCREENWIDTH/2 - 8) | ATTR1_MEDIUM;
		shadowOAM[23].attr2 = ATTR2_TILEID(10,1);
	} else if (playTimer >= 60 && playTimer < 120) {
		shadowOAM[23].attr0 = (SCREENHEIGHT/2 - 16) | ATTR0_4BPP | ATTR0_TALL;
		shadowOAM[23].attr1 = (SCREENWIDTH/2 - 8) | ATTR1_MEDIUM;
		shadowOAM[23].attr2 = ATTR2_TILEID(10,4);
	} else if (playTimer >= 120 && playTimer < 180) {
		shadowOAM[23].attr0 = (SCREENHEIGHT/2 - 16) | ATTR0_4BPP | ATTR0_TALL;
		shadowOAM[23].attr1 = (SCREENWIDTH/2 - 8) | ATTR1_MEDIUM;
		shadowOAM[23].attr2 = ATTR2_TILEID(10,7);
	} else {
		shadowOAM[23].attr0 = (SCREENHEIGHT/2 - 16) | ATTR0_4BPP | ATTR0_WIDE;
		shadowOAM[23].attr1 = (SCREENWIDTH/2 - 8) | ATTR1_LARGE;
		shadowOAM[23].attr2 = ATTR2_TILEID(10,9);
	}

	// Update score
	if (gameTimer % 20 == 0 && playTimer > 260) {
		score++;
	}

	// Increase zappers as game goes on
	if (gameTimer < 1000) {
		maxZappers = 2;
	} else if (gameTimer >= 1000 && gameTimer <= 1800) {
		maxZappers = 3;
	} else if (gameTimer > 1800 && gameTimer < 2400) {
		maxZappers = 4;
	} else if (gameTimer >= 2400) {
		maxZappers = 8;
	}

	// Update timers
    fallTimer++;
    soundTimer++;
    gameTimer++;
    playTimer++;
}

// Draws the game each frame
void drawGame() {
		drawPlayer();
		drawEasyCoins();
		drawZappers();
		drawScore();
}

// Initialize the player
void initPlayer() {
	player.height = 16;
    player.width = 16;
    player.row = SHIFTUP(player.height + 18);
    player.col = player.width;
    player.rdel = 0;
    player.cdel = 0;
	player.aniCounter = 0;
    player.curFrame = 0;
    player.numFrames = 4;
    player.aniState = 0;
}

// Handle every-frame actions of the player
void updatePlayer() {

	// Check if player hits walls
	if (SHIFTDOWN(player.row) + player.height - 1 <= 10 || (SHIFTDOWN(player.row) + player.height - 1) + 5 >= SCREENHEIGHT) {
		gameOver = 1;
	}

	hOff++;

	// Add gravity to rdel, depending on direction
	if (fallTimer % 5 == 0 && (!cheatMode)) {
		if ((player.rdel > 0 && gravity < 0) || (player.rdel < 0 && gravity > 0)) {
			player.rdel += gravity*5;
		} else {
			player.rdel += gravity;
		}
	}

	// Toggle cheat mode
	if (BUTTON_PRESSED(BUTTON_B) && !cheatMode) {
		cheatMode = 1;
		player.rdel = 0;
	} else if (BUTTON_PRESSED(BUTTON_B) && cheatMode) {
		cheatMode = 0;
	}
	
	// Change direction of gravity
	if (!cheatMode && BUTTON_PRESSED(BUTTON_A)) {
		gravity *= -1;
		fallTimer = 1;
	}
	
	// Move up/down freely in cheat mode
	if (cheatMode && BUTTON_HELD(BUTTON_UP)) {
		player.rdel = 0;
		if (gravity > 0) {
			player.rdel -= gravity * 10;
		} else {
			player.rdel += gravity*10;
		}
	} else if (cheatMode && BUTTON_HELD(BUTTON_DOWN)) {
		player.rdel = 0;
		if (gravity > 0) {
			player.rdel += gravity*10;
		} else {
			player.rdel -= gravity*10;
		}
	} else if (cheatMode) {
		player.rdel = 0;
	}

	// Player animation frames when changing gravity
	if (fallTimer <= 14 && fallTimer >= 8 && gravity == -25) {
		player.curFrame = 1;
	} else if (fallTimer <= 14 && fallTimer >= 8 && gravity == 25) {
		player.curFrame = 3;
	} else if (gravity == 25) {
		player.curFrame = 0;
	} else if(gravity == -25){
		player.curFrame = 2;
	} else {
		player.curFrame = 0;
	}

		// Update player's row on screen
		player.row += player.rdel;
}

// Draw the player
void drawPlayer() {
	shadowOAM[0].attr0 = SHIFTDOWN(player.row) | ATTR0_4BPP | ATTR0_SQUARE;
	shadowOAM[0].attr1 = player.col | ATTR1_SMALL;
	shadowOAM[0].attr2 = ATTR2_TILEID(0,player.curFrame*2);
}

// Initialize the easy coins
void initEasyCoins() {

	for (int i = 0; i < 10; i++) {
    	easyCoins[i].height = 8;
    	easyCoins[i].width = 8;
    	easyCoins[i].row = 0;
    	easyCoins[i].col = 0;
    	easyCoins[i].rdel = 0;
    	easyCoins[i].cdel = 0;
		easyCoins[i].aniCounter = 0;
    	easyCoins[i].curFrame = 0;
    	easyCoins[i].numFrames = 6;
    	easyCoins[i].aniState = 0;
    	easyCoins[i].active = 0;
    	easyCoins[i].collected = 0;
    	}

}

// Handle every-frame actions of easy coins
void updateEasyCoins() {

	// Make sure there are always 4 coins active
	if (activeEasyCoins <= 3) {
		for (int i = 0; i < 10; i++) {
			if (!easyCoins[i].active) {
				easyCoins[i].row = SCREENHEIGHT/2 - easyCoins[i].height/2 + (rand() % 100) - 50;
				easyCoins[i].col = rand() % 305 + 200;
				easyCoins[i].active = 1;
				easyCoins[i].aniCounter = 0;
				activeEasyCoins++;
				if (activeEasyCoins > 3) {
					break;
				}
			}
		}
	}

	// Move coins towards player
	for (int i = 0; i < 10; i++) {
		if (easyCoins[i].active) {
			easyCoins[i].col--;
			if (easyCoins[i].col < 0) {
				easyCoins[i].active = 0;
				easyCoins[i].collected = 0;
				activeEasyCoins--;
			}

			// Coin animation frames
			if (easyCoins[i].aniCounter % 20 == 0) {
				easyCoins[i].curFrame++;
				if (easyCoins[i].curFrame >= easyCoins[i].numFrames) {
					easyCoins[i].curFrame = 0;
				}
			}
		}
		easyCoins[i].aniCounter++;
	}

	// Handle player-coin collisions
	for (int i = 0; i < 10; i++) {
		if (collision(SHIFTDOWN(player.row), player.col, player.height, player.width,
			easyCoins[i].row, easyCoins[i].col, easyCoins[i].height, easyCoins[i].width)
			&& easyCoins[i].active) {
			easyCoins[i].collected = 1;

			// Avoid adding the score and playing the sound twice for one coin
			if (soundTimer > 20) {
				playSoundB(CoinSFX,COINSFXLEN,COINSFXFREQ,0);
				soundTimer = 0;
				score += 5;
			}
		}
	}
}

// Draw easy coins
void drawEasyCoins() {

	for (int i = 1; i < 11; i++) {
		if (easyCoins[i - 1].active) {
			shadowOAM[i].attr0 = easyCoins[i-1].row | ATTR0_4BPP | ATTR0_SQUARE;
			shadowOAM[i].attr1 = easyCoins[i-1].col | ATTR1_TINY;
			if (easyCoins[i-1].collected) {
				shadowOAM[i].attr2 = ATTR2_TILEID(2 + easyCoins[i-1].curFrame,1);
			} else {
				shadowOAM[i].attr2 = ATTR2_TILEID(2 + easyCoins[i-1].curFrame,0);
			}
		} else {
			shadowOAM[i].attr0 = ATTR0_HIDE;
		}
	}
}

// Initialize the zappers
void initZappers() {
	for (int i = 0; i < 10; i++) {
    	zappers[i].height = 15;
    	zappers[i].width = 4;
    	zappers[i].row = 0;
    	zappers[i].col = 0;
    	zappers[i].rdel = 0;
    	zappers[i].cdel = 0;
		zappers[i].aniCounter = 0;
    	zappers[i].curFrame = 0;
    	zappers[i].numFrames = 3;
    	zappers[i].aniState = 0;
    	zappers[i].active = 0;
    	zappers[i].collected = 0;
    	}
}

// Update the zappers
void updateZappers() {
	
	// Make sure there are always 6 active zappers
	if (activeZappers < maxZappers) {
		for (int i = 0; i < 10; i++) {
			if (!zappers[i].active) {
				zappers[i].row = SCREENHEIGHT/2 - zappers[i].height/2 + (rand() % 80 - 40);

				// Save last row/col index and reshuffle if next zapper is too close
				if (lastZapperRow - zappers[i].row <= 20 && lastZapperRow - zappers[i].row >= -20) {
					zappers[i].row = SCREENHEIGHT/2 - zappers[i].height/2 + (rand() % 80 - 40);
				}
				lastZapperRow = zappers[i].row;
				zappers[i].col = rand() % 305 + 200;
				if (lastZapperCol - zappers[i].col <= 20 && lastZapperCol - zappers[i].col >= -20) {
					zappers[i].col = rand() % 305 + 200;
				}
				lastZapperCol = zappers[i].col;
				zappers[i].active = 1;
				activeZappers++;
				if (activeZappers > maxZappers) {
					break;
				}
			}
		}
	}
	// Move zappers towards player
	for (int i = 0; i < 10; i++) {
		if (zappers[i].active) {
			zappers[i].col--;
			if (zappers[i].col < 0) {
				zappers[i].active = 0;
				zappers[i].collected = 0;
				activeZappers--;
			}
			// Zapper animation frames
			if (zappers[i].aniCounter % 20 == 0) {
				zappers[i].curFrame++;
				if (zappers[i].curFrame >= zappers[i].numFrames) {
					zappers[i].curFrame = 0;
				}
			}
		}
		zappers[i].aniCounter++;
	}

	// Handle player-zapper collisions
	for (int i = 0; i < 10; i++) {
		if (collision(SHIFTDOWN(player.row), player.col, player.height, player.width,
			zappers[i].row, zappers[i].col, zappers[i].height, zappers[i].width)
			&& zappers[i].active) {
			gameOver = 1;
		}
	}
}

// Draw the zappers
void drawZappers() {
    for (int i = 0; i < 10; i++) {
		if (zappers[i].active) {
			shadowOAM[i + 11].attr0 = zappers[i].row | ATTR0_4BPP | ATTR0_TALL;
			shadowOAM[i+11].attr1 = zappers[i].col | ATTR1_TINY;
			shadowOAM[i+11].attr2 = ATTR2_TILEID(2 + zappers[i].curFrame*2,2);
		} else {
			shadowOAM[i+11].attr0 = ATTR0_HIDE;
		}
	}
}

// Draw the score
void drawScore() {

	// Draw the rightmost digit
	shadowOAM[20].attr0 = 140 | ATTR0_4BPP | ATTR0_SQUARE;
	shadowOAM[20].attr1 = 220 | ATTR1_SMALL;
	shadowOAM[20].attr2 = ATTR2_TILEID(8, (score % 10)*2);

	// Draw the middle digit
	shadowOAM[21].attr0 = 140 | ATTR0_4BPP | ATTR0_SQUARE;
	shadowOAM[21].attr1 = 210 | ATTR1_SMALL;
	shadowOAM[21].attr2 = ATTR2_TILEID(8, ((score/10)%10)*2);

	// Draw the leftmost digit
	shadowOAM[22].attr0 = 140 | ATTR0_4BPP | ATTR0_SQUARE;
	shadowOAM[22].attr1 = 200 | ATTR1_SMALL;
	shadowOAM[22].attr2 = ATTR2_TILEID(8, (((score/10)/10)%10)*2);
}