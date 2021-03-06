// Tetris.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <thread>
#include <Windows.h>

using namespace std;

int nFieldWidth = 12;					// width of board
int nFieldHeight = 18;					// height of board
unsigned char *pField = nullptr;		// board array

int nScreenWidth = 120;					// screen width
int nScreenHeight = 80;					// screen height
int nScreenWidthOffset = 150;			// width offset (move displayed field to right)
int nScreenHeightOffset = 4;			// height offset (move displayed field to bottom)
wchar_t *screen = nullptr;				// screen array
HANDLE hConsole = nullptr;				// output console handle
DWORD dwBytesWritten = -1;				// for the output console

wstring TETROMINO[7];					// to store the tetromino blocks

bool bKey[4];							// to store the key presses

void initField() {
	// initializes the board
	pField = new unsigned char[nFieldWidth * nFieldHeight];
	
	for (int x = 0; x < nFieldWidth; ++x)	// board boundary
		for (int y = 0; y < nFieldHeight; ++y)
			pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
}

void createAssets() {
	// create tetromino assets
	TETROMINO[0].append(L"..X.");
	TETROMINO[0].append(L"..X.");
	TETROMINO[0].append(L"..X.");
	TETROMINO[0].append(L"..X.");

	TETROMINO[1].append(L"..X.");
	TETROMINO[1].append(L".XX.");
	TETROMINO[1].append(L".X..");
	TETROMINO[1].append(L"....");

	TETROMINO[2].append(L".X..");
	TETROMINO[2].append(L".XX.");
	TETROMINO[2].append(L"..X.");
	TETROMINO[2].append(L"....");

	TETROMINO[3].append(L"....");
	TETROMINO[3].append(L".XX.");
	TETROMINO[3].append(L".XX.");
	TETROMINO[3].append(L"....");

	TETROMINO[4].append(L"..X.");
	TETROMINO[4].append(L".XX.");
	TETROMINO[4].append(L"..X.");
	TETROMINO[4].append(L"....");

	TETROMINO[5].append(L"....");
	TETROMINO[5].append(L".XX.");
	TETROMINO[5].append(L"..X.");
	TETROMINO[5].append(L"..X.");

	TETROMINO[6].append(L"....");
	TETROMINO[6].append(L".XX.");
	TETROMINO[6].append(L".X..");
	TETROMINO[6].append(L".X..");
}

void initScreen() {
	// initialize the screen and output handle to display the board on
	screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; ++i)
		screen[i] = L' ';

	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	dwBytesWritten = 0;

	WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
}

int rotateTetromino(int px, int py, int r) {
	// returns the rotated positions of px and py for the given rotation
	switch (r % 4) {
	case 0: return py * 4 + px;		// 0 degrees
	case 1: return 12 + py - (px * 4);		// 90 degrees
	case 2: return 15 - (py * 4) - px;		// 180 degrees
	case 3: return 3 - py + (px * 4);		// 270 degrees
	}

	return 0;
}

bool doesTetrominoFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
	// determines whether the given tetromino and rotation are within the bounds and not overlapping existing pieces
	for(int px = 0; px < 4; ++px)
		for (int py = 0; py < 4; ++py) {
			// get index into the piece, based on the rotation
			int pi = rotateTetromino(px, py, nRotation);

			// get index of the piece in the field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			// check bounds
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
					// if there is a tetromino piece already on the field
					if (TETROMINO[nTetromino][pi] == L'X' && pField[fi] != 0)
						return false;
				}
			}
		}

	return true;
}

int main()
{

	initField();
	createAssets();
	initScreen();

	// game logic
	bool bGameOver = false;					// game status
	bool bRotateHold = false;				// user is holding the rotate key
	bool bMoveDown = false;					// move a piece down

	int nCurrentPiece = 0;					// current tetromino id
	int nCurrentRotation = 0;				// current orientation of the tetromino
	int nCurrentX = nFieldWidth >> 1;		// start from the middle of the screen
	int nCurrentY = 0;						// start from the top of the screen
	int nSpeed = 20;						// game speed
	int nSpeedCounter = 0;					// game tick counter
	int nPieceCounter = 0;					// tetromino counter
	int nScore = 0;							// score count

	vector<int> vLines;						// completed horizontal lines to be removed

	while (!bGameOver) {
		// game timing
		this_thread::sleep_for(50ms);		// game tick
		++nSpeedCounter;
		bMoveDown = (nSpeed == nSpeedCounter);
		
		// user input
		for (int k = 0; k < 4; ++k)								//  R   L   D Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

		// game logic
					  // user pressed left key
		nCurrentX += (bKey[0] && doesTetrominoFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
					  // user pressed left key
		nCurrentX += (bKey[1] && doesTetrominoFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? -1 : 0;
					  // user pressed down key
		nCurrentY += (bKey[2] && doesTetrominoFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
					 // user pressed Z key (rotate)
		if (bKey[3]) {
			nCurrentRotation += (!bRotateHold && doesTetrominoFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = true;
		}
		else
			bRotateHold = false;

		if (bMoveDown) {
			if (doesTetrominoFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				// the current piece can move down
				++nCurrentY;
			}	
			else {	// the current piece can no longer move down
				// lock the current piece in the field
				for (int px = 0; px < 4; ++px)
					for (int py = 0; py < 4; ++py)
						if (TETROMINO[nCurrentPiece][rotateTetromino(px, py, nCurrentRotation)] == L'X')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

				// check for full horizontal lines to eliminate
				for(int py = 0; py < 4; ++py) 
					if (nCurrentY + py < nFieldHeight - 1) {
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; ++px)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						if (bLine) {
							// remove complete horizontal line
							for (int px = 1; px < nFieldWidth - 1; ++px)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;

							vLines.push_back(nCurrentY + py);
						}
					}

				// increase score for one piece
				nScore += 25;
				// increase score for complete lines
				if (!vLines.empty())
					nScore += (1 << vLines.size()) * 100;
				
				// pick a next piece
				nCurrentPiece = rand() % 7;
				nCurrentRotation = 0;
				nCurrentX = nFieldWidth >> 1;
				nCurrentY = 0;

				// increase piece counter and difficulty
				++nPieceCounter;
				if (nPieceCounter % 10 == 0)
					if (nSpeed >= 10)
						--nSpeed;

				// if nextPiece cannot fit => GAME OVER!!!
				bGameOver = !doesTetrominoFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
			nSpeedCounter = 0;
		}

		// draw field
		for (int x = 0; x < nFieldWidth; ++x)
			for (int y = 0; y < nFieldHeight; ++y)
				screen[(y + nScreenHeightOffset)*nScreenWidth + (x + nScreenWidthOffset)] = L" ABCDEFG-#"[pField[y*nFieldWidth + x]];

		// draw current piece
		for (int px = 0; px < 4; ++px)
			for (int py = 0; py < 4; ++py)
				if (TETROMINO[nCurrentPiece][rotateTetromino(px, py, nCurrentRotation)] == L'X')
					screen[(nCurrentY + py + nScreenHeightOffset)*nScreenWidth + (nCurrentX + px + nScreenWidthOffset)] = nCurrentPiece + 65;

		if (!vLines.empty()) {
			//display frame (for a small instance to show the dashed lines)
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);	// delay for a small time

			for (auto &v : vLines) 
				for (int px = 1; px < nFieldWidth - 1; ++px) {
					for (int py = v; py > 0; --py)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}
			
			vLines.clear();
		}

		// draw score
		swprintf(&screen[2 * nScreenWidth + nFieldHeight + 6], 16, L"Score: %8d", nScore);

		//display frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	}

	CloseHandle(hConsole);
	cout << "GAME OVER!!!\nScore: " << nScore << "\n";
	system("pause");

    return 0;
}

