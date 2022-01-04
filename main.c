/*
I apologize for excessive commenting in this tiny piece of code, I was alone and without an internet connection when I wrote this... I only had VSCode to talk to

newer versions should include more optimization, visual flare, and better fucking COLLISION DETECTION.
they should definitely also include cleaner code, just in general, I threw the initial version together pretty quickly, but I'm still proud of it.
and also an intro screen, a game over screen, and more fanfare in general, I mean a normal terminal with "You win!" isn't all that satisfying.
the other fishies should probably also move up and down at random... I think that would actually add a lot to the game overall.
*/

#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define GAMEWIDTH 64
#define GAMEHEIGHT 16
#define FISHAMOUNT 10 //I recommend keeping this below gameheight so that there's always an opening to swim past the other fishies

/*
todo: add a way to detect multiple keypresses at the same time
probably involving a linked list and a for loop that will continue to accept getches until ERR is returned
*/

enum Size{TINY, SMALL, MEDIUM, LARGE};

struct Fish {
	short x, y, length;
	enum Size size;
};

int main() {

	//variable setup
	srand(time(NULL));
	int score = 0;
	char c;
	char* egg = ".";
	char* sfish = "~";
	char* mfishl = "<><";
	char* lfishl = "<((><<";
	char* mfishr = "><>";
	char* lfishr = ">><))>";

	struct Fish PlayerFish = { 10, 5, 1 }; //the size doesn't matter here, because it will be set to SMALL anyway when the game starts

	struct Fish OtherFish[FISHAMOUNT];
	for(int i = 0; i < FISHAMOUNT; i++) {
		struct Fish RandomFish = { GAMEWIDTH, rand()%(GAMEHEIGHT-1)+1, 0, rand()%4 };
		OtherFish[i] = RandomFish;
	}

	//inits
	initscr();
	start_color();
	curs_set(0);
	cbreak();
	noecho();
	nodelay(stdscr, 1);

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
		erase();
		c = getch();
		flushinp();

		if(c == 'w' && PlayerFish.y > 1) { PlayerFish.y -= 1; }
		if(c == 'a' && PlayerFish.x > 0) { PlayerFish.x -= 2; }
		if(c == 's' && PlayerFish.y < GAMEHEIGHT-1) { PlayerFish.y += 1; }
		if(c == 'd' && PlayerFish.x < GAMEWIDTH) { PlayerFish.x += 2; }

		attron(COLOR_PAIR(4));
		if(score >= 0 && score < 1000) {
			PlayerFish.length = 1;
			PlayerFish.size = SMALL;
			mvprintw(PlayerFish.y, PlayerFish.x, sfish);
		} else if(score >= 1000 && score < 3000) {
			PlayerFish.length = 3;
			PlayerFish.size = MEDIUM;
			mvprintw(PlayerFish.y, PlayerFish.x, mfishr);
		} else if(score >= 3000 && score < 6000) {
			PlayerFish.length = 6;
			PlayerFish.size = LARGE;
			mvprintw(PlayerFish.y, PlayerFish.x, lfishr);
		} else {
			endwin();
			printf("You win!\n");
			return 0;
		}
		attroff(COLOR_PAIR(4));

		attron(COLOR_PAIR(5));
		mvprintw(GAMEHEIGHT+1, 0, "Score: %i", score);
		attroff(COLOR_PAIR(5));

		for(int i = 0; i < FISHAMOUNT; i++) {
			//this collision detection is shite but I didn't wanna sit here for weeks with this being the only thing holding back progress
			//I *know* the nested if-statement is yucky and this part is all shite in general actually BUT AT LEAST IT WORKS (kind of, sometimes it doesn't)
			if(OtherFish[i].x > PlayerFish.x && OtherFish[i].x < PlayerFish.x + PlayerFish.length + 1 && OtherFish[i].y == PlayerFish.y) {
				if(OtherFish[i].size < PlayerFish.size) {
					score += (OtherFish[i].size+1)*100;
					OtherFish[i].x = GAMEWIDTH;
					OtherFish[i].y = rand()%(GAMEHEIGHT-1)+1;
					OtherFish[i].size = rand()%4;
				} else if(OtherFish[i].size > PlayerFish.size) {
					endwin();
					printf("You were eaten!\n");
					return 0;
				} //if neither of these is true (ie the fish are of equal size) nothing is done
			}

			//this controls the speed of the fishies, every frame they move at least 1 unit, but have a 25% chance of moving 2
			//that makes their movement speed somewhat random and makes the game look a little more complex than it is
			//I'm actually rather proud of this one
			OtherFish[i].x -= ((rand()%4) == 0) + 1;

			switch(OtherFish[i].size) { //this switch statement decides which way to "draw" the fish (includes colour) depending on their size stat
			case TINY:
				attron(COLOR_PAIR(0));
				mvprintw(OtherFish[i].y, OtherFish[i].x, egg);
				attroff(COLOR_PAIR(0));
				break;
			case SMALL:
				attron(COLOR_PAIR(1));
				mvprintw(OtherFish[i].y, OtherFish[i].x, sfish);
				attroff(COLOR_PAIR(1));
				break;
			case MEDIUM:
				attron(COLOR_PAIR(2));
				mvprintw(OtherFish[i].y, OtherFish[i].x, mfishl);
				attroff(COLOR_PAIR(2));
				break;
			case LARGE:
				attron(COLOR_PAIR(3));
				mvprintw(OtherFish[i].y, OtherFish[i].x, lfishl);
				attroff(COLOR_PAIR(3));
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

		//make this its own window so that it isn't redrawn every frame, that eats cpu cycles like people with hangovers eat mcdonald's
		//make the score display its own window while we're at it, that also does not need to be redrawn every frame, just when the actual score is updated
		mvhline(0, 0, ACS_HLINE, GAMEWIDTH);
		mvhline(GAMEHEIGHT, 0, ACS_HLINE, GAMEWIDTH);

		refresh();
		usleep(125000); //crisp and smooth 8 FPS B)
	}

	endwin();
	return 0;
}
