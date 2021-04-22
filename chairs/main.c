#include "game.h"

// Global variables
int NUM_PLAYERS;
Game *game;


void* PlayerAction(void* tid) {
  TakeChair(game);
  return NULL;
}

void* DJ(void* tid) {
  StartGame(game);
  return NULL;
}

int main(int argc, char **argv) {
  printf("Musical Chairs\n");
  printf("--------------------------------------\n\n");
  printf("Enter number of players: ");
  fflush(stdin);
  scanf("%d", &NUM_PLAYERS);

  game = SetupGame(NUM_PLAYERS);
  
  // Create DJ thread
  pthread_t dj;
  pthread_create(&dj, NULL, DJ, NULL);

  // Create player threads
  pthread_t players[game->num_players];
  for (int i = 0; i < game->num_players; i++) {
    pthread_create(&(players[i]), NULL, PlayerAction, NULL);
  }

  // Wait for players to be done.
  for (int i = 0; i < game->num_players; i++) {
    pthread_join(players[i], NULL);
  }

  // Wait for the dj to declare a winner.
  pthread_join(dj, NULL);

  EndGame(game);
  return 0;
}
