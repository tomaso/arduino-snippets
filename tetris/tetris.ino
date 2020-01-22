#include "U8glib.h" 


U8GLIB_SH1106_128X64 u8g(13, 11, 10, 9, 8);  // D0=13, D1=11, CS=10, DC=9, Reset=8 

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define WALL_WIDTH 7
#define BLOCK_SIZE 5
#define PG_WIDTH 10
#define PG_HEIGHT 24
#define RIGHT_BTN_PIN 2
#define LEFT_BTN_PIN 3
#define ROTATE_BTN_PIN 4

/*
 * One block is 
 * 4+1 x 4+1
 * so the playground is 
 * 10 x 24
 */

int rightButtonState = 0;         // variable for reading the pushbutton status
int leftButtonState = 0;         // variable for reading the pushbutton status

bool *field;

char * bricks[4][5] = {
  {"....", "..\n..", ".. \n ..", " ..\n.. ", "...\n . "},
  {".\n.\n.\n.", "..\n..", " .\n..\n. ", ". \n..\n .", " .\n..\n ."},
  {"....", "..\n..", ".. \n ..", " ..\n.. ", " . \n..."},
  {".\n.\n.\n.", "..\n..", " .\n..\n. ", ". \n..\n .", ". \n..\n. "} 
};


int current_brick = 0;
int rotation = 0;
int x=4, y=0;

void drawPlayground(void) {
  u8g.drawBox(0, 0, DISPLAY_WIDTH, WALL_WIDTH-1);
  u8g.drawBox(0, DISPLAY_HEIGHT-WALL_WIDTH+1, DISPLAY_WIDTH, WALL_WIDTH-1);
  u8g.drawBox(DISPLAY_WIDTH-WALL_WIDTH, 0, WALL_WIDTH-2, DISPLAY_HEIGHT);
}

void drawOneBlock(int x, int y)
{
  if (x<0 || x>PG_WIDTH-1 || y<0 || y>PG_HEIGHT-1)
    return;

  u8g.drawBox(
    y*BLOCK_SIZE,
    DISPLAY_HEIGHT-x*BLOCK_SIZE-BLOCK_SIZE-WALL_WIDTH,
    BLOCK_SIZE-1,
    BLOCK_SIZE-1
   );   
}

void drawField(void)
{
  int i, j;
  for (j=0; j<PG_HEIGHT; j++) {
    for (i=0; i<PG_WIDTH; i++) {
      if (field[j*PG_WIDTH+i])
        drawOneBlock(i, j);
    }
  }
}

void drawCurrentBrick(void)
{
  char *p;
  int x1 = x;
  int y1 = y;
  
  for(p=bricks[rotation][current_brick]; *p!='\0'; p++) {
    if(*p=='.') {
      drawOneBlock(x1,y1);
    }
    if(*p=='\n') {
      x1 = x;
      y1 += 1;
    } else {
      x1 += 1;  
    }
  }
}

void moveOneDown(int fromLine)
{
  int i, j;


  for(j=fromLine; j>0; j--) {
    for(i=0; i<PG_WIDTH; i++) {
      field[j*PG_WIDTH+i] = field[(j-1)*PG_WIDTH+i];
    }
  }
  // Line 0
  for(i=0; i<PG_WIDTH; i++) {
    field[i] = false;
  }
}

void checkCompleteLines(void)
{
  bool line_complete;
  
  for(int j=0; j<PG_HEIGHT; j++) {
    line_complete = true;
    for(int i=0; i<PG_WIDTH; i++) {
      line_complete &= field[j*PG_WIDTH+i];
      if(!line_complete)
        break;
    }
    if(line_complete) {
      moveOneDown(j);
    }
  }
}

void placeTheBrick(void)
{
  char *p;
  int x1 = x;
  int y1 = y;
  
  for(p=bricks[rotation][current_brick]; *p!='\0'; p++) {
    if(*p=='.') {
      field[y1*PG_WIDTH+x1] = true;
    }
    if(*p=='\n') {
      x1 = x;
      y1 += 1;
    } else {
      x1 += 1;
    }
  }
  
  x = PG_WIDTH/2;
  y = 0;
  int last_brick = current_brick;
  while(last_brick == current_brick)
    current_brick = random(0, 5);
  checkCompleteLines();
}

bool isMovePossible(int newx, int newy)
{
  char *p;
  int x1 = newx;
  int y1 = newy;

  for(p=bricks[rotation][current_brick]; *p!='\0'; p++) {
    if(y1>=PG_HEIGHT)
      return false;
      
    if(*p=='.') {
      if(field[y1*PG_WIDTH+x1] || x1>=PG_WIDTH || x1<0) {
        return false;
      }
    }
    if(*p=='\n') {
      x1 = newx;
      y1 += 1;
    } else {
      x1 += 1;
    }
  }
  return true;
}

void moveCurrentBrick(int dx, int dy)
{
  if(isMovePossible(x+dx, y)) {
    x += dx;
  }
  if(isMovePossible(x, y+dy)) {
    y += dy;
  } else {
    placeTheBrick();
  }
}
  
void setup(void) {
  
  // Initialize HW
  pinMode(RIGHT_BTN_PIN, INPUT);
  pinMode(LEFT_BTN_PIN, INPUT);
  pinMode(ROTATE_BTN_PIN, INPUT);

  // Initialize data structures
  field = (bool *) calloc(PG_WIDTH*PG_HEIGHT, sizeof(bool));
  randomSeed(analogRead(0));
  current_brick = random(0, 5);
//  current_brick = 0;
} 

void loop(void) {
  int dx = 0;
  static int cycle = 0;
  
  u8g.firstPage();
  if(digitalRead(RIGHT_BTN_PIN) == HIGH)
    dx = 1;
  if(digitalRead(LEFT_BTN_PIN) == HIGH)
    dx = -1;
  if(digitalRead(ROTATE_BTN_PIN) == HIGH)
     rotation += 1;
     if(rotation==4)
      rotation = 0;

  moveCurrentBrick(dx, 0);
  cycle++;
  if (cycle==1) {
    moveCurrentBrick(0, 1);
    cycle=0;
  }
  
  do {
    drawPlayground();
    drawField();
    drawCurrentBrick();
  } while( u8g.nextPage() );
  delay(100);
}


