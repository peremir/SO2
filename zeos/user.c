
void print(char *buffer)
{
  write(1, buffer, strlen(buffer));
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
    if (c[0] == 'w') print("\n player up");
      //CHANGE PLAYER UP
      
    else if (c[0] == 'a') print("\n player left");
      //CHANGE PLAYER LEFT
        
    else if (c[0] == 's') print("\n player down");
      //CHANGE PLAYER RIGHT

    else if (c[0] == 'd') print("\n player right");
     //CHANGE PLAYER DOWN 
    }
  exit_thread();
}

void gameStep(int id)
{


  exit_thread();
}

void gameStart()
{
  struct game* game = (struct game*)dyn_mem(sizeof(struct game));

  //INIT GAME DATA
  game->score = 0;
  game->lives = 3;
  game->end_game = 0;   //0 is no, 1 is yes(end game)
  
  //INIT OBJECTS
  game->player.x = 20;
  game->player.y = 20;

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
    //print("\n game running");
  }
}



int __attribute__ ((__section__(".text.main")))
  main(void)
{
  /* Next line, tries to move value 0 to CR3 register. */ 
  /* This register is a privileged one, and so it will raise an exception */
  
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  gameStart();

  while(1) { }
}
