#include"utils_joc.h"

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4






void print(char *buffer)
{
  write(1, buffer, strlen(buffer));
}

char * my_itoa(int x) {
  char * buffer = '\0';
  itoa(x,buffer);
  return buffer;
}

void printNum(int num)
{
  char *numBuff = "\0\0\0\0\0\0";
  itoa(num, numBuff);
  print(numBuff);
}

// GAME FUNCTIONALITY //

#define MAX_ENEMIES 10

struct object {
  int x;
  int y;
};

struct game {
    int score;
    int lives;
    int end_game;
    int mutex;
    struct object player;
    struct object enemies[MAX_ENEMIES];
};

void keyboardRead(struct game *game)
{
  char c[2];

  while(!game->end_game) {
    
    read(&c, 1);
 
    // WASD
    if (c[0] == 'w') print("\n player up") {
      //CHANGE PLAYER UP
      mutex_lock(&game->mutex);
      g_erase_xy(game->player.x, game->player.y);
      game->player.y--;
      mutex_unlock(&game->mutex);
    }
    else if (c[0] == 'a') print("\n player left") {
      //CHANGE PLAYER LEFT
      mutex_lock(&game->mutex);
      g_erase_xy(game->player.x, game->player.y);
      game->player.x--;
      mutex_unlock(&game->mutex);
    }
    else if (c[0] == 's') print("\n player down") {
      //CHANGE PLAYER RIGHT
      mutex_lock(&game->mutex);
      g_erase_xy(game->player.x+, game->player.y);
      game->player.x++;
      mutex_unlock(&game->mutex);
    }
    else if (c[0] == 'd') print("\n player right") {
     //CHANGE PLAYER DOWN
      mutex_lock(&game->mutex);
      g_erase_xy(game->player.x, game->player.y);
      game->player.x++;
      mutex_unlock(&game->mutex);
    }
  }
  exit_thread();
}

void gameStep(int id)
{
  print("\ngameStep");
  //DIBUIXAR PLAYER AND ENEMIES

  exit_thread();
}

void gameStart()
{
  struct game* game = (struct game*)dyn_mem(sizeof(struct game));
  
  //print("\[20;10f\[42;33mP");
  //INIT GAME DATA

  //INIT GAME DATA
  game->score = 0;
  game->lives = 3;
  game->end_game = 0;   //0 is no, 1 is yes(end game)
  
  //INIT OBJECTS
  game->player.x = 20;
  game->player.y = 20;
  g_draw_xy('A',10,10,YELLOW, BLUE);

  for(int i = 0; i < MAX_ENEMIES; ++i) 
  {
    game->enemies[i].x = 10+i;
    game->enemies[i].y = 20;
  }

  //INIT GAME SYNC MUTEX
  game->mutex = 0;
  mutex_init(&game->mutex);

  //RUNNING GAME ALREADY
  create_thread((void*)keyboardRead, game);
  
  while(!game->end_game) {   
    
    //CALCULATE AND EXECUTE A GAME STEP
    if(gettime()%50 == 0) {
      int time = gettime();
      while(gettime() == time);

      create_thread((void*)gameStep, game);
    }
  
  //MORE GAME DATA
  }
}



int __attribute__ ((__section__(".text.main")))
  main(void)
{

  gameStart();

  g_fill_screen('E',RED,YELLOW);
  
  while(1) { }
}
