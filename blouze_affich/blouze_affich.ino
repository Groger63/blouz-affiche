#include <Event.h>
#include <Timer.h>
#include <Wire.h>  
#include <LiquidCrystal_I2C.h> // Using version 1.2.1
#include "EEPROM.h"
#include "SevSeg.h"

#define PROMO 16
#define FILLIERE 1
#define HIGHSCORE 2
#define TODAY 3
#define MOYENNE 4

//Create an instance of the object.
SevSeg myDisplay;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 


const int counter_addr = 0;
const int highscore_addr = 2;
const int reboots_addr = 4;
const int button = A0;

int counter_total ;
int counter_reboots ;
int counter_today ;
int counter_highscore ;
int indent = 15 ;

Timer t ;

int digit_state = PROMO ;
int lcd_state = TODAY ;

char tempString[5] = {'I','N','F','o'}; //Used for sprintf

void setup()
{
  pinMode(button, INPUT);

  int reboots = lireInt(reboots_addr);
  reboots ++ ;
  sauverInt(reboots_addr,reboots);

  counter_today=0;

  counter_total = lireInt(counter_addr);
  sauverInt(counter_addr,counter_total);


  //init lcd
  lcd.begin(16,2); // sixteen characters across - 2 lines
  lcd.backlight();

  // write lcd
  indent  =15 ;
  if(counter_total > 9) indent =14 ;
  if(counter_total > 99) indent =13 ;
  if(counter_total > 999) indent =12 ;
  lcd.setCursor(0,0);
  lcd.print("Total bieres: ");
  lcd.setCursor(indent,0);
  lcd.print(counter_total);
  lcd.setCursor(0,1);

  indent  =15 ;
  if(counter_today > 9) indent =14 ;
  if(counter_today > 99) indent =13 ;
  if(counter_today > 999) indent =12 ;
  lcd.print("Aujourd'hui:");
  lcd.setCursor(indent,1);
  lcd.print(counter_today);
  
  t.every(10, update_digits);
  t.every(5000, switch_digits);
  t.every(5000, switch_screen);
  
  init_digits();


}

void loop()
{
  t.update();
  if(digitalRead(button)==HIGH){
      
    counter_total = lireInt(counter_addr);
    counter_highscore = lireInt(highscore_addr);

    counter_total ++ ;
    sauverInt(counter_addr,counter_total);
    counter_today ++ ;

    lcd.setCursor(0,0);
    lcd.print("Total bieres:   ");

    indent  =15 ;
    if(counter_total > 9) indent =14 ;
    if(counter_total > 99) indent =13 ;
    if(counter_total > 999) indent =12 ;
    lcd.setCursor(indent,0);
    lcd.print(counter_total);

    if(counter_today>counter_highscore)
    {
      counter_highscore ++;
      sauverInt(highscore_addr,counter_highscore);

      indent  =15 ;
      if(counter_highscore > 9) indent =14 ;
      if(counter_highscore > 99) indent =13 ;
      if(counter_highscore > 999) indent =12 ;
      lcd.setCursor(0,1);
      lcd.print("Highscore:      ");
      lcd.setCursor(indent,1);
      lcd.print(counter_highscore);
    }
    else{

      indent  =15 ;
      if(counter_today > 9) indent =14 ;
      if(counter_today > 99) indent =13 ;
      if(counter_today > 999) indent =12 ;
      lcd.setCursor(0,1);
      lcd.print("Aujourd'hui:    ");
      lcd.setCursor(indent,1);
      lcd.print(counter_today);
    }
    while(digitalRead(button)==HIGH) ;
  }
  

}

void sauverInt(int adresse, int val) 
{   
    //découpage de la variable val qui contient la valeur à sauvegarder en mémoire
    unsigned char faible = val & 0x00FF; //récupère les 8 bits de droite (poids faible) -> 0010 1100 
    //calcul : 1101 0111 0010 1100 & 0000 0000 1111 1111 = 0010 1100

    unsigned char fort = (val >> 8) & 0x00FF;  //décale puis récupère les 8 bits de gauche (poids fort) -> 1101 0111
    //calcul : 1101 0111 0010 1100 >> 8 = 0000 0000 1101 0111 puis le même & qu’avant

    //puis on enregistre les deux variables obtenues en mémoire
    EEPROM.write(adresse, fort) ; //on écrit les bits de poids fort en premier
    EEPROM.write(adresse+1, faible) ; //puis on écrit les bits de poids faible à la case suivante
}

int lireInt(int adresse)
{
    int val = 0 ; //variable de type int, vide, qui va contenir le résultat de la lecture

    unsigned char fort = EEPROM.read(adresse);     //récupère les 8 bits de gauche (poids fort) -> 1101 0111
    unsigned char faible = EEPROM.read(adresse+1); //récupère les 8 bits de droite (poids faible) -> 0010 1100

    //assemblage des deux variable précédentes
    val = fort ;         // val vaut alors 0000 0000 1101 0111
    val = val << 8 ;     // val vaut maintenant 1101 0111 0000 0000 (décalage)
    val = val | faible ; // utilisation du masque
    // calcul : 1101 0111 0000 0000 | 0010 1100 = 1101 0111 0010 1100

    return val ; //on n’oublie pas de retourner la valeur lue !
}


void update_digits()
{
    myDisplay.DisplayString(tempString, 0); //(numberToDisplay, decimal point location)
}

void switch_screen()
{
  counter_total = lireInt(counter_addr);
  counter_highscore = lireInt(highscore_addr);
  float moyenne = (counter_total/lireInt(reboots_addr));

  indent  =15 ;
  if(counter_total > 9) indent =14 ;
  if(counter_total > 99) indent =13 ;
  if(counter_total > 999) indent =12 ;
  // write lcd
  lcd.setCursor(0,0);
  lcd.print("Total bieres:   ");
  lcd.setCursor(indent,0);
  lcd.print(counter_total);
  if(lcd_state == TODAY)
  {

    lcd.setCursor(0,1);
    lcd.print("Aujourd'hui:    ");

    indent  =15 ;
    if(counter_today > 9) indent =14 ;
    if(counter_today > 99) indent =13 ;
    if(counter_today > 999) indent =12 ;
    lcd.setCursor(indent,1);
    lcd.print(counter_today);
    lcd_state = HIGHSCORE ;
  }
  if(lcd_state == HIGHSCORE)
  {
    lcd.setCursor(0,1);
    lcd.print("Highscore:      ");

    indent  =15 ;
    if(counter_highscore > 9) indent =14 ;
    if(counter_highscore > 99) indent =13 ;
    if(counter_highscore > 999) indent =12 ;
    lcd.setCursor(indent,1);
    lcd.print(counter_highscore);

    lcd_state = MOYENNE ;

  }
  if(lcd_state == MOYENNE)
  {
    lcd.setCursor(0,1);
    lcd.print("Moyenne/j:      ");

    indent  =15 ;
    if(moyenne > 9) indent =14 ;
    if(moyenne > 99) indent =13 ;
    if(moyenne > 999) indent =12 ;
    lcd.setCursor(indent,1);
    lcd.print(moyenne);

    lcd_state = TODAY ;

  }

}
void switch_digits()
{
  if(digit_state == PROMO ){
    digit_state=FILLIERE;
      tempString[0] = '.'; 
      tempString[1] = '1';
      tempString[2] = '6';
      tempString[3] = '.';
  }else{
    digit_state=PROMO;
      tempString[0] = 'I'; 
      tempString[1] = 'n';
      tempString[2] = 'f';
      tempString[3] = 'o';
  } 
}

void init_digits()
{

  int displayType = COMMON_CATHODE; //Your display is either common cathode or common anode

  
  //This pinout is for a bubble dispaly
  //Declare what pins are connected to the GND pins (cathodes)
  int digit1 = 2; //Pin 1
  int digit2 = 3; //Pin 10
  int digit3 = 4; //Pin 4
  int digit4 = 5; //Pin 6
       
  //Declare what pins are connected to the segments (anodes)
  int segA = 6; //Pin 12
  int segB = 7; //Pin 11
  int segC = 8; //Pin 3
  int segD = 9; //Pin 8
  int segE = 10; //Pin 2
  int segF = 11; //Pin 9
  int segG = 12; //Pin 7
  int segDP= 13; //Pin 5
   
  int numberOfDigits = 4; //Do you have a 1, 2 or 4 digit display?

  myDisplay.Begin(displayType, numberOfDigits, digit1, digit2, digit3, digit4, segA, segB, segC, segD, segE, segF, segG, segDP);
  
  myDisplay.SetBrightness(100); //Set the display to 100% brightness level
}
