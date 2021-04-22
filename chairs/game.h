#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>

// A game contains:
// - Number of players.
// - Number of chairs available.
// - Boolean to determine if music is on.
// - Synchronization objects
typedef struct GameStruct{
  int num_players;
  int chairs;
  bool music;
  sem_t semaphore;
  sem_t take_chair;
} Game;

/**
 * Creates a Game.
 *
 * - Allocate space for game.
 * - Initialize variables.
 * - Initialize syncronization objects.
 */
Game* SetupGame();

/**
 * Ends a Game.
 *
 * - Destroy syncronization objects.
 * - Free space of the game.
 */
void EndGame(Game *game);

/**
 * Grabs a chair.
 *
 * - Wait for music to turn on.
 * - Try to grab a chair.
 */
void* TakeChair(Game *game);

/**
 * Starts the game.
 *
 * - Wait for round to end. ie. No more free chairs.
 */
void* StartGame(Game *game);

/**
 * Sets up the chairs.
 *
 * - Allocates space for chairs.
 */
void* SetupChairs(Game *game);

/**
 * Helper function
 *
 * - Take user input.
 */
char Input();