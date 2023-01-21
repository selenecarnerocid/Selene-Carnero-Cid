#include "HD44780.hpp"
#include "Arduino.h"
#include "libADC.hpp"
//The setup function is called once at startup of the sketch
//ref: https://circuitdigest.com/microcontroller-projects/custom-characters-on-lcd-using-pic16f877a
#define PROBABILITY_BLOCK 25
#define PROBABILITY_BLOCK_CONTINUE 75
#define INITIAL_DELAY 500
#define SPEED_UP 1

int line1; //upper set of lcd
int line2; //lower set of lcd
int player_position; // 246810
                     // 13579
int alive; //set zero if die
int score;
int speed;
int pic; //1 run1, 2 run2, 3 jump, 4 down

void CreateCustomPic(unsigned char *Pattern, const char Location);
void writeDisplay(int l1, int l2, int player);
void shift(int &l1, int &l2);
void insert_terrain(int &l1, int &l2);
void move_player(int &player, int l2);
int check_game();
void start_game();
void print_score ();

void CreateCustomPic(unsigned char *Pattern, const char Location) {
  int i = 0;
  LCD_WriteCommand(0x40+(Location*8));
  for (i=0;i<8;i++)
    LCD_WriteData(Pattern[i]);
}

void writeDisplay (int l1, int l2, int player)
{
  int compare = 1;
  int line;
  for (int i=0; i<=1;i++) {
    if (i==0) 
      line = l1;
    else
      line = l2;
     
    for (int col=15; col>=0; col--) {
        //LCD_GoTo(15, 1);
        //LCD_WriteData(0b11111111); 
      if (line & (compare<<(15-col))){
        LCD_GoTo(col, i);
        LCD_WriteData(0b11111111);
      }
      else {
        LCD_GoTo(col, i);
        LCD_WriteData(0b00010000);     
      } 
    }
  }
  //player
  if (player<=10){
    LCD_GoTo((player/2)+(player%2)-1, player%2);
    switch (pic) {
    case 1:
      LCD_WriteData(0);
      pic = 2;
      break;
    case 2:
      LCD_WriteData(1);
      pic = 1;
      break;
    case 3:
      LCD_WriteData(2);
      pic = 1;
      break; 
    case 4:
      LCD_WriteData(3);
      pic = 1;
      break; 
    }
  }

}

void shift(int &l1, int &l2){
  l1 <<= 1;
  l2 <<= 1;
}
void insert_terrain(int &l1, int &l2){
  if ((l1&0b10) | (l2&0b10)){
    if (random(100)<PROBABILITY_BLOCK_CONTINUE) //probability of continue block
      if (l1&0b10)
        l1 = l1 | 0b1;
      else
        l2 = l2 | 0b1;
    return;
  }
  
  if (random(100)<PROBABILITY_BLOCK){ //probability of show block
    if (random(0,2)){ //check for insert in ground
      if (!(l1 & 0b111) && !(l2 & 0b100))   //if no block up in last 3 spaces or block down and space before insert
        l2 = l2 | 0b1;
    }
    else{ //check for insert up
      if (!(l2 & 0b111))
        l1 = l1 | 0b1;
    }
  }

}
void move_player(int &player, int l2){
  //down if no ground
  if (!(player%2) && (!(l2 & (0b1 << (16-(player/2+player%2)))))){
    player--;
    pic=4;
  }

  //attending arrow keys
  if ((ADC_conversion() )> 120 && (ADC_conversion()< 140) && (player%2==1)){
    player++; //up
    pic = 3;
  }    
  if ((ADC_conversion() )> 300 && (ADC_conversion()< 320) && (player%2==0)){
    player--; //down
    pic = 4;
  }  
  if ((ADC_conversion() )>= 0 && (ADC_conversion()< 20)  && (player<9))
    player +=2; //right
  if ((ADC_conversion() )> 470 && (ADC_conversion()< 500)  && (player>2))
    player -=2; //left
}
int check_game(int l1, int l2, int p){
  int line;
  if (p%2) //it's in the low line
    line = l2;
  else
    line = l1;
  //Serial.println(l2);
  if (line & (0b1 << (16-(p/2+p%2))))
      return 0; 
    else 
      return 1; //stil alive
}

void start_game() {
  LCD_GoTo(0,0);
  LCD_WriteText("Click to start...");
  delay(1000);
  while (!alive){
     if (((ADC_conversion() )> 120 && (ADC_conversion()< 140)) ||
         ((ADC_conversion() )> 300 && (ADC_conversion()< 320) ) ||
         ((ADC_conversion() )>= 0 && (ADC_conversion()< 20)  ) ||
         ((ADC_conversion() )> 470 && (ADC_conversion()<500)) ||
         ((ADC_conversion() )> 700 && (ADC_conversion()<740)) )
          init_environment();
  }
}

void print_score (){
  if (score > 99)
    LCD_GoTo(13,0);
  else if (score >9)
    LCD_GoTo(14,0);
  else
    LCD_GoTo(15,0);   
  char cstr[4];
  itoa(score, cstr, 10);
  LCD_WriteText(cstr);
  score++; 
}

void init_environment(){
  line1 = 0;
  line2 = 0;
  //line1 = 0b00000011;
  //line2 = 0b11100000;
  player_position = 1;
  alive = 1;
  score = 0;
  speed = INITIAL_DELAY;
  pic = 1; 
}

void createPics(){
  byte run1[8] = {
  // Run position 1
    B01100,
    B01100,
    B00000,
    B01110,
    B11100,
    B01100,
    B11010,
    B10011};

  byte run2[8] = {
    B01100,
    B01100,
    B00000,
    B01100,
    B01100,
    B01100,
    B01100,
    B01110};

  byte jump[8] = { 
    B01100,
    B01100,
    B00000,
    B11110,
    B01101,
    B11111,
    B10000,
    B00000};
    
  byte jumpdown[8] = {
    B11110,
    B01101,
    B11111,
    B10000,
    B00000,
    B00000,
    B00000,
    B00000};
  CreateCustomPic (run1, 0);
  CreateCustomPic (run2, 1);
  CreateCustomPic (jump, 2);
  CreateCustomPic (jumpdown, 3);
  //LCD_GoTo(0,0);
  //LCD_WriteData(0b0);
  //delay(10000);
}

void setup()
{
	LCD_Initalize();
  ADC_Init();
  //Serial.begin(9600);
  createPics();
}


void loop()
{
  if (!alive)
    start_game();

  writeDisplay(line1,line2,player_position);
  shift(line1,line2);
  insert_terrain(line1,line2);
  move_player(player_position, line2);
  alive =  check_game(line1,line2,player_position);
  print_score ();
  speed -= SPEED_UP;
  delay (speed);
}
