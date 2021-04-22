#include "game.h"

// Game Setup
Game* SetupGame(int num_players) {
  // Allocate space for game.
  Game *game = (Game*) malloc(sizeof(Game));
  // Initialize variables.
  game->num_players = num_players;
  game->music = true;
  game->chairs = num_players - 1;

  // Initialize syncronization object
  sem_init(&game->semaphore, 0, 0);
	sem_init(&game->take_chair, 0, 0);
  return game;
}

void EndGame(Game *game) {
  if (game->chairs == 0) {
	  // Destroy synchronization objects.
    sem_destroy(&game->semaphore);
		sem_destroy(&game->take_chair);
	  // Free game space.
	  free(game);
	  return;
  }
  printf("### Error!\n");
}

// Code for Players.
void* TakeChair(Game *game) { // Note: This ends the thread when the player is out. 
  bool alive = true;
  while (alive) {
		// Join the game
    sem_post(&game->semaphore);
		// Take a chair if one is available
		sem_post(&game->take_chair);
		if (game->chairs > 0) {
			game->chairs--;
			printf("Player got a chair!\n");
		} else {
			alive = false;
      printf("Player is out!\n");
			return 0;
		}
		printf("Chairs: %d\n", game->chairs);
  }

  return 0;
}

// Code for DJ.
void* StartGame(Game *game) {
	printf("-- Starting Game.\n");
	bool game_on = true;
	int round = 1;
	while (game_on) {
		// Wait for players to be loaded
    for (int i = 0; i < game->num_players; i++) {
      sem_wait(&game->semaphore);
    }
		game->chairs = game->num_players - 1;
		printf("Chairs have been set up. Let round %d begin!\n", round);

		// Wait on all chairs to be taken in the round
		for (int i = 0; i < game->chairs; i++) {
			sem_wait(&game->take_chair);
			// Check to see if chair was taken
			printf("Chair %d taken.\n", i+1);
		}
		printf("All chairs have been taken.\n");
	
		// If there is a winner.
		if (game->num_players == 1) {
			game_on = false;
			printf("We have a winner!\n");
			return 0;
		}
		game->num_players--;
		printf("-- Round %d over! ------------------------------------------\n", round);
		round++;
	}
	printf("-- Game over.\n");
  return 0;
}

char Input() {
	char c = '\0';
	fflush(stdin);
	c = getc(stdin);
	return c;
}