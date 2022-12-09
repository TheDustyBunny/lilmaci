/*
I apologize for excessive commenting in this tiny piece of code, I was alone and without an internet connection when I wrote the initial version...
I only had VSCode to talk to

newer versions should include more optimization, visual flare, and better fucking COLLISION DETECTION.
they should definitely also include cleaner code, just in general, I threw the initial version together pretty quickly, but I'm still proud of it.
and also an intro screen, a game over screen, and more fanfare in general, I mean a normal terminal with "You win!" isn't all that satisfying.
the other fishies should probably also move up and down at random... I think that would actually add a lot to the game overall.
*/

#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <string.h>

//used this so much it began looking really ugly in the code, so I just made it a macro because "sizeof" is apparently weird with functions; works fine like this
#define arrlen( array ) ( sizeof( array ) / sizeof( array[0] ) )

#define GAMEWIDTH 64
#define GAMEHEIGHT 16
#define FISHAMOUNT 10 //I recommend keeping this below gameheight so that there's always an opening to swim past the other fishies

enum Size { TINY, SMALL, MEDIUM, LARGE };

struct Fish {
	short x, y, length;
	enum Size size;
};

struct MenuItem {
	char* text;
	char ID;
};

//functions :)
int PlayGame();
int ShowMenu();
int ShowLicense();

int main() {

	//this allows us to use funny characters in ncurses
	setlocale(LC_ALL, "");

	//start ncurses exactly how I want it
	initscr();
	start_color();
	curs_set(0);
	cbreak();
	noecho();

	ShowMenu();

	endwin();
	return 0;
}


//----------------------------------------


//show the main menu
int ShowMenu() {
	
	WINDOW* MainMenu = newwin(LINES, COLS, LINES/2-GAMEHEIGHT/2, COLS/2-GAMEWIDTH/2);
	nodelay(MainMenu, true);

	struct MenuItem PlayOption = { "Play!", 'P' };
	struct MenuItem ScoreboardOption = { "[WIP] Scoreboard!", 'S' };
	struct MenuItem LicenseOption = { "Software License!", 'L' };
	struct MenuItem ExitOption = { "Exit!", 'X' };
	
	struct MenuItem options[] = { PlayOption, ScoreboardOption, LicenseOption, ExitOption };

    char c;
    short choice = 0;
	int retscore = 0;
	bool acted = false;

	//initialize colours for displaying stuff on the menu, as one does
	init_pair(6, COLOR_GREEN, COLOR_BLACK);
	init_pair(7, COLOR_YELLOW, COLOR_BLACK);

	while(true) {
		c = wgetch(MainMenu);
		if(c != ERR) { acted = true; }

		// vim movement in menu
		if(c == 'j' && choice < arrlen(options)-1) { choice++; werase(MainMenu); }
		if(c == 'k' && choice > 0) { choice--; werase(MainMenu); }

		if(c == 's' && choice < arrlen(options)-1) { choice++; werase(MainMenu); }
		if(c == 'w' && choice > 0) { choice--; werase(MainMenu); }
		if(c == ' ') {
			switch (options[choice].ID) {
			case 'P':
				retscore = PlayGame();
				break;
			case 'X':
				delwin(MainMenu);
				return 0;
				break;
			case 'L':
				werase(MainMenu);
				ShowLicense();
				break;
			}
		}

		wattron(MainMenu, COLOR_PAIR(6));
		mvwprintw(MainMenu, GAMEHEIGHT/2-5, GAMEWIDTH/2-9, "Lilmaci!");
		wattroff(MainMenu, COLOR_PAIR(6));

		wattron(MainMenu, COLOR_PAIR(7));
		mvwprintw(MainMenu, GAMEHEIGHT/2-5, GAMEWIDTH/2, "Main Menu!");
		wattroff(MainMenu, COLOR_PAIR(7));

		if(!acted) {
			mvwprintw(MainMenu, GAMEHEIGHT/2-4, GAMEWIDTH/2-26, "(Use [WS] and [SPACE] to make selections in the menu)");
		}

		if(retscore) {
			mvwprintw(MainMenu, GAMEHEIGHT/2-4, GAMEWIDTH/2-12, "Your latest score was ");
			wattron(MainMenu, COLOR_PAIR(7));
			mvwprintw(MainMenu, GAMEHEIGHT/2-4, GAMEWIDTH/2+10, "%i!", retscore);
			wattroff(MainMenu, COLOR_PAIR(7));
		}

		for(int i = 0; i < arrlen(options); i++) {
			if(i == choice) {
				wattron(MainMenu, COLOR_PAIR(6));
				mvwprintw(MainMenu, GAMEHEIGHT/2-(arrlen(options))/2+i, GAMEWIDTH/2-strlen(options[i].text)/2-4, "><>");
				wattroff(MainMenu, COLOR_PAIR(6));

                wattron(MainMenu, A_REVERSE);
			}

			mvwprintw(MainMenu, GAMEHEIGHT/2-(arrlen(options))/2+i, GAMEWIDTH/2-strlen(options[i].text)/2, "%s\n", options[i].text);
            wattroff(MainMenu, A_REVERSE);
		}

		wrefresh(MainMenu);
		usleep(696969696969696969); // this makes the main menu not consume as much of the cpu for some reason
    }
	
	return 0;
}


//----------------------------------------


int ShowLicense() {

	WINDOW* LicenseWindow = newwin(LINES, COLS, LINES/2-GAMEHEIGHT/2, COLS/2-GAMEWIDTH/2);

	wprintw(LicenseWindow, "This program is free software: you can redistribute it and/or modify\n"
	"it under the terms of the GNU General Public License as published by\n"
	"the Free Software Foundation, either version 3 of the License, or\n"
	"(at your option) any later version.\n"
	"\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"GNU General Public License for more details.\n"
	"\n"
	"You should have received a copy of the GNU General Public License\n"
	"along with this program.  If not, see <https://www.gnu.org/licenses/>.");

	wrefresh(LicenseWindow);
	wgetch(LicenseWindow);
	delwin(LicenseWindow);
	return 0;
}


//----------------------------------------


//the actual game function itself, including the gameloop ;))))))
int PlayGame() {

	//variable setup
	srand(time(NULL));
	int score = 0;
	int belly = 0;
	int iframes = 0;
	char c;
	char* egg = "·";
	char* sfish = "~";
	char* mfishl = "<><";
	char* lfishl = "<((><<";
	char* mfishr = "><>";
	char* lfishr = ">><))>";

	struct Fish PlayerFish = { 10, 5, 1, 1 };

	struct Fish OtherFish[FISHAMOUNT];
	for(int i = 0; i < FISHAMOUNT; i++) {
		struct Fish RandomFish = { GAMEWIDTH, rand()%(GAMEHEIGHT-1)+1, 0, rand()%4 };
		OtherFish[i] = RandomFish;
	}

	//inits
	WINDOW* GameWindow = newwin(LINES, COLS, LINES/2-GAMEHEIGHT/2, COLS/2-GAMEWIDTH/2);
	nodelay(GameWindow, 1);

	//initialize colours for enemy fishies
	init_pair(0, COLOR_WHITE, COLOR_BLACK);
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_BLACK);

	//initialize colours for player fish
	init_pair(4, COLOR_GREEN, COLOR_BLACK);

	//initialize colours for score display
	init_pair(5, COLOR_YELLOW, COLOR_BLACK);

	//gameloop
	while(true) {
		werase(GameWindow);
		c = wgetch(GameWindow);
		flushinp();

		// vim movement in the game
		if(c == 'k' && PlayerFish.y > 1) { PlayerFish.y -= 1; }
		if(c == 'h' && PlayerFish.x > 0) { PlayerFish.x -= 2; }
		if(c == 'j' && PlayerFish.y < GAMEHEIGHT-1) { PlayerFish.y += 1; }
		if(c == 'l' && PlayerFish.x < GAMEWIDTH) { PlayerFish.x += 2; }

		if(c == 'w' && PlayerFish.y > 1) { PlayerFish.y -= 1; }
		if(c == 'a' && PlayerFish.x > 0) { PlayerFish.x -= 2; }
		if(c == 's' && PlayerFish.y < GAMEHEIGHT-1) { PlayerFish.y += 1; }
		if(c == 'd' && PlayerFish.x < GAMEWIDTH) { PlayerFish.x += 2; }

		//exit game immediately if esc is pressed... should probably just make this a pause button
		if(c == 27) { delwin(GameWindow); return score; }

		wattron(GameWindow, COLOR_PAIR(4));
		if(iframes > 0) { wattron(GameWindow, A_REVERSE); }

		if(PlayerFish.size == SMALL) {
			PlayerFish.length = 1;
			mvwprintw(GameWindow, PlayerFish.y, PlayerFish.x, sfish);
		} else if(PlayerFish.size == MEDIUM) {
			PlayerFish.length = 3;
			mvwprintw(GameWindow, PlayerFish.y, PlayerFish.x, mfishr);
		} else if(PlayerFish.size == LARGE) {
			PlayerFish.length = 6;
			mvwprintw(GameWindow, PlayerFish.y, PlayerFish.x, lfishr);
		}

		wattroff(GameWindow, COLOR_PAIR(4));
		wattroff(GameWindow, A_REVERSE);

		wattron(GameWindow, COLOR_PAIR(5));
		mvwprintw(GameWindow, GAMEHEIGHT+1, 0, "Score: %i", score);
		wattroff(GameWindow, COLOR_PAIR(5));

		for(int i = 0; i < FISHAMOUNT; i++) {
			//this collision detection is shite but I didn't wanna sit here for weeks with this being the only thing holding back progress
			//I *know* the nested if-statement is yucky and this part is all shite in general actually BUT AT LEAST IT WORKS (kind of, sometimes it doesn't)
			if( iframes <= 0 && OtherFish[i].x > PlayerFish.x - 2 && OtherFish[i].x < PlayerFish.x + PlayerFish.length + 1 && OtherFish[i].y == PlayerFish.y) {
				if(OtherFish[i].size < PlayerFish.size) {
					score += (OtherFish[i].size+1)*100;
					belly += (OtherFish[i].size+1)*10;
					OtherFish[i].x = GAMEWIDTH;
					OtherFish[i].y = rand()%(GAMEHEIGHT-1)+1;
					OtherFish[i].size = rand()%4;
				} else if(OtherFish[i].size > PlayerFish.size) {
					belly = 0;
					iframes = 10;
					score -= 100;
					PlayerFish.size--;
				} else {
					belly = 0;
					iframes = 10;
					score -= 100;
					PlayerFish.size--;
					OtherFish[i].x = GAMEWIDTH;
					OtherFish[i].y = rand()%(GAMEHEIGHT-1)+1;
					OtherFish[i].size = rand()%4;
				}
			}

			//this controls the speed of the fishies, every frame they move at least 1 unit, but have a 25% chance of moving 2
			//that makes their movement speed somewhat random and makes the game look a little more complex than it is
			//I'm actually rather proud of this one
			OtherFish[i].x -= ((rand()%4) == 0) + 1;

			switch(OtherFish[i].size) { //this switch statement decides which way to "draw" the fish (includes colour) depending on their size stat
			case TINY:
				wattron(GameWindow, COLOR_PAIR(0));
				mvwprintw(GameWindow, OtherFish[i].y, OtherFish[i].x, egg);
				wattroff(GameWindow, COLOR_PAIR(0));
				break;
			case SMALL:
				wattron(GameWindow, COLOR_PAIR(1));
				mvwprintw(GameWindow, OtherFish[i].y, OtherFish[i].x, sfish);
				wattroff(GameWindow, COLOR_PAIR(1));
				break;
			case MEDIUM:
				wattron(GameWindow, COLOR_PAIR(2));
				mvwprintw(GameWindow, OtherFish[i].y, OtherFish[i].x, mfishl);
				wattroff(GameWindow, COLOR_PAIR(2));
				break;
			case LARGE:
				wattron(GameWindow, COLOR_PAIR(3));
				mvwprintw(GameWindow, OtherFish[i].y, OtherFish[i].x, lfishl);
				wattroff(GameWindow, COLOR_PAIR(3));
				break;
			}

			//sends fish back when they reach the end and randomize them giving the illusion of new fish (maybe turn into a function on its own?)
			//I mean I literally have this exact same piece of code a few lines above, isn't that justification enough for making it into a function?
			if(OtherFish[i].x < 0) {
				OtherFish[i].x = GAMEWIDTH;
				OtherFish[i].y = rand()%(GAMEHEIGHT-1)+1;
				OtherFish[i].size = rand()%4;
			}
		}

		//health management for playerfish
		if(PlayerFish.size <= 0) {
			delwin(GameWindow);
			return score;
		} else if(belly >= PlayerFish.size*100 && PlayerFish.size < LARGE) {
			belly = 0;
			PlayerFish.size++;
		}

		//make this its own window so that it isn't redrawn every frame, that eats cpu cycles like people with hangovers eat mcdonald's
		//make the score display its own window while we're at it, that also does not need to be redrawn every frame, just when the actual score is updated
		mvwhline(GameWindow, 0, 0, ACS_HLINE, GAMEWIDTH);
		mvwhline(GameWindow, GAMEHEIGHT, 0, ACS_HLINE, GAMEWIDTH);

		if(iframes > 0) iframes--;

		wrefresh(GameWindow);
		usleep(125000); //crisp and smooth 8 FPS B)
	}

	delwin(GameWindow);
	return 0;
}
