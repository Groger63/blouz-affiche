/*********************************************************************************
*     File Name           :     blouz_affich.ino
*     Created By          :     Roger
*     Creation Date       :     [2017-04-03 14:30]
*     Last Modified       :     [AUTO_UPDATE_BEFORE_SAVE]
*     Description         :     Mini projet d'embarqué consistant à intégrer deux afficheurs 
*                               et un bouton sur un blouse UTBM afin de reccueillir
*                               des statistiques alcooliques et les afficher en temps réel.
*                               Développé en écoutant du Megadeth.
**********************************************************************************/





////////////////////////////////////////////
//                                        //
//            Librairies Utiles           //
//                                        //
////////////////////////////////////////////

#include <Wire.h>
#include <Event.h>
#include <Timer.h>
#include <LiquidCrystal_I2C.h> // Using version 1.2.1 <- /!\ testé avec les nouvelles version, ça veut pas compiler 
#include "EEPROM.h"
#include "SevSeg.h"




////////////////////////////////////////////
//                                        //
//  Declaration variables & Constantes    //
//                                        //
////////////////////////////////////////////

//Etats d'ébriété
#define SOBER 0
#define TIPSY 2
#define DRUNK 4
#define PISSED 6
#define BLACK_OUT 9


//pour l'afficheur 7 segments
#define PROMO 16
#define BEER 0
#define FILLIERE 1

//pour l'affichage des stats
#define HIGHSCORE 0
#define TODAY 1
#define MOYENNE 2
#define RANDOMSHIT 3
#define EBRIETE 4

//pour l'affichage des conneries
#define NB_RANDOMSHIT 10

//afficheur 7 segments 4 chiffres
SevSeg myDisplay;

//afficheur LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Branchement : sda A4 et SCL A5



//adresses eeprom pour stocker highscore reboots et total
const int counter_addr = 0;
const int highscore_addr = 2;
const int reboots_addr = 4;

//adresse du bouton
const int button = A0;

//statistiques à afficher
int counter_total ;
int counter_reboots ;
int counter_today ;
int counter_highscore ;
int indent = 15 ;

//un timer pour gérer differents types d'events
Timer t ;

//les etats des afficheurs
int digit_state = PROMO ;
int lcd_state = TODAY ;

//état de buvage
int still_drinking = 0 ;


//un "dictionnaire" a phrases bidons pour le lcd
String random_shit_line1[NB_RANDOMSHIT] = 
                          {" C'est pas faux ",//0
                          "     Sire,      ", //1
                          "    Le GRAS     ", //2
                          "      FREE      ", //3
                          "  Pipi statue ? ", //4
                          "  Celui qui lit ", //5
                          "  C'est pas moi ", //6
                          " T vaginalement ", //7
                          " 06 73 40 76 71 ", //8
                          "  6xKWAK sur le "  //9
                          };
                         
String random_shit_line2[NB_RANDOMSHIT] = 
                          {
                          "                ", //0
                          " on en a gros ! ", //1
                          "  c'est la vie  ", //2
                          "      HUGS      ", //3
                          "                ", //5
                          "Ceci est un con ", //7
                          "c'est la biere !", //8
                          "motocultable twa", //9
                          "   Appelle moi  ", //10
                          "    8291r stp   "  //11
                          };

//le conteneur de l'afficheur 7 segments
char tempString[5] = {'I','N','F','o'}; //initialisé à "info"



////////////////////////////////////////////
//                                        //
//            Logique Arduino             //
//                                        //
////////////////////////////////////////////

void setup()
{
  //reinit();
  pinMode(button, INPUT);

  //incrémenter le nombre de reboots pour le calcul de la moyenne/jour
  int reboots = lireInt(reboots_addr);
  reboots ++ ;
  sauverInt(reboots_addr,reboots);

  counter_today=0;

  counter_highscore = lireInt(highscore_addr);
  counter_total = lireInt(counter_addr);

  init_lcd();
  init_digits();
  test_shit();
  //lancer les events 
  t.every(10, update_digits);
  t.every(5 * 1000, switch_digits);
  t.every(5 * 1000, switch_screen);
}


void loop()
{
  t.update();
  if(digitalRead(button)==HIGH){
      drink_beer();
  }
  while(digitalRead(button)==HIGH) ;
}


////////////////////////////////////////////
//                                        //
//      Fonctions affichage LCD           //
//                                        //
////////////////////////////////////////////
void test_shit()
{
  for(int i =0 ; i < 10 ; i ++) 
  {
     lcd.setCursor(0,0);
    lcd.print(random_shit_line1[i]);
    
    lcd.setCursor(0,1);
    lcd.print(random_shit_line2[i]);
    delay(1000);
  }
  for(int i =0 ; i < 16 ; i ++) 
  {
    switch(i)
    {
      case SOBER ... TIPSY-1 :
        lcd.setCursor(0,0);
        lcd.print("Roger est sobre!");
        lcd.setCursor(0,1);
        lcd.print("Une biere, vite!");
        break;
      case TIPSY ... DRUNK-1 :
        lcd.setCursor(0,0);
        lcd.print("Roger pompette!");
        lcd.setCursor(0,1);
        lcd.print("Il a tjrs soif!");
        break;
      case DRUNK ... PISSED-1 :
        lcd.setCursor(0,0);
        lcd.print("HOURRA ! Roger");
        lcd.setCursor(0,1);
        lcd.print("  est bourre  ");
        break;
      case PISSED ... BLACK_OUT-1 :
        lcd.setCursor(0,0);
        lcd.print(" Merde, Roger ");
        lcd.setCursor(0,1);
        lcd.print("  est dechire ");
        break;
      case BLACK_OUT ... 999 :
        lcd.setCursor(0,0);
        lcd.print("éùBLACK OUT µ ");
        lcd.setCursor(0,1);
        lcd.print("é:%*://èè,zs#");
        break;
      default :
        lcd.setCursor(0,0);
        lcd.print("éùBLACK OUT µ ");
        lcd.setCursor(0,1);
        lcd.print("é:%*://èè,zs#"); 
        break;
        
    }
    delay(500);
  }
}

void init_lcd()
{
   //init lcd
  lcd.begin(16,2); // sixteen characters across - 2 lines
  lcd.backlight();
  // write lcd
  lcd.setCursor(0,0);
  lcd.print("Initialisation ");
  
  lcd.setCursor(0,1);
  lcd.print("Please wait    ");
}

void lcd_affiche_highscore()
{
  lcd.setCursor(0,0);
  lcd.print("Bieres bues     ");
    
  indent  =15 ;
  if(counter_highscore > 9) indent =14 ;
  if(counter_highscore > 99) indent =13 ;
  if(counter_highscore > 999) indent =12 ;
  lcd.setCursor(0,1);
  lcd.print("Highscore:      ");
  lcd.setCursor(indent,1);
  lcd.print(counter_highscore);
}

void lcd_affiche_new_highscore()
{
  lcd.setCursor(0,0);
  lcd.print("NEW HIGHSCORE ! ");
    
  indent  =15 ;
  if(counter_highscore > 9) indent =14 ;
  if(counter_highscore > 99) indent =13 ;
  if(counter_highscore > 999) indent =12 ;
  lcd.setCursor(0,1);
  lcd.print("Highscore:      ");
  lcd.setCursor(indent,1);
  lcd.print(counter_highscore);
}

void lcd_affiche_moyenne()
{  
  float moyenne = ((float)counter_total/(float)lireInt(reboots_addr));
  
  lcd.setCursor(0,0);
  lcd.print("Bieres bues     ");
  lcd.setCursor(0,1);
  lcd.print("Moyenne/j:      ");
  lcd.setCursor(12,1);
  lcd.print(moyenne);
  
}

void lcd_affiche_session()
{
  
  lcd.setCursor(0,0);
  lcd.print("Bieres bues     ");
  
  lcd.setCursor(0,1);
  lcd.print("Aujourd'hui:    ");

  indent  =15 ;
  if(counter_today > 9) indent =14 ;
  if(counter_today > 99) indent =13 ;
  if(counter_today > 999) indent =12 ;
  lcd.setCursor(indent,1);
  lcd.print(counter_today);
}

void lcd_affiche_total()
{  
  lcd.setCursor(0,0);
  lcd.print("Bieres bues     ");
  
  lcd.setCursor(0,1);
  lcd.print("Total :         ");

  indent  =15 ;
  if(counter_total > 9) indent =14 ;
  if(counter_total > 99) indent =13 ;
  if(counter_total > 999) indent =12 ;
  lcd.setCursor(indent,0);
  lcd.print(counter_total);  
}

void lcd_affiche_still_drinking()
{  
  lcd.setCursor(0,0);
  lcd.print("  Finis d'abord ");
  
  lcd.setCursor(0,1);
  lcd.print("celle que t'as !");
  t.after(3 * 1000 ,switch_screen);
}


void lcd_affiche_ebriete()
{  
  switch(counter_today)
  {
    case SOBER ... TIPSY-1 :
      lcd.setCursor(0,0);
      lcd.print("Roger est sobre!");
      lcd.setCursor(0,1);
      lcd.print("Une biere, vite!");
      break;
    case TIPSY ... DRUNK-1 :
      lcd.setCursor(0,0);
      lcd.print("Roger pompette!");
      lcd.setCursor(0,1);
      lcd.print("Il a tjrs soif!");
      break;
    case DRUNK ... PISSED-1 :
      lcd.setCursor(0,0);
      lcd.print("HOURRA ! Roger");
      lcd.setCursor(0,1);
      lcd.print("  est bourre  ");
      break;
    case PISSED ... BLACK_OUT-1 :
      lcd.setCursor(0,0);
      lcd.print(" Merde, Roger ");
      lcd.setCursor(0,1);
      lcd.print("  est déchire ");
      break;
    case BLACK_OUT ... 999 :
      lcd.setCursor(0,0);
      lcd.print("éùBLACK OUT µ ");
      lcd.setCursor(0,1);
      lcd.print("é:%*://èè,zs#");
      break;
    default :
      lcd.setCursor(0,0);
      lcd.print("éùBLACK OUT µ ");
      lcd.setCursor(0,1);
      lcd.print("é:%*://èè,zs#"); 
      break;
      
  }
}

void lcd_affiche_randomshit()
{
  int randomshit = random(NB_RANDOMSHIT);
  lcd.setCursor(0,0);
  lcd.print(random_shit_line1[randomshit]);
  
  lcd.setCursor(0,1);
  lcd.print(random_shit_line2[randomshit]);
}


//switche les differents mode d'affichage
void switch_screen()
{
  int tmp_lcd_state =random(5);
  while(tmp_lcd_state == lcd_state) tmp_lcd_state =random(5);
  lcd_state = tmp_lcd_state ;
  
  switch(lcd_state)
  {
    case TODAY :
      lcd_affiche_session();
      break ;
    case HIGHSCORE :
      lcd_affiche_highscore();
      break ;
    case MOYENNE :
      lcd_affiche_moyenne();
      break ;
    case RANDOMSHIT : 
      lcd_affiche_randomshit();
      break ;
    case EBRIETE : 
      lcd_affiche_ebriete();
      break ;
    default :
      lcd_affiche_randomshit();
      lcd_state = TODAY ;
      break ;
  }
  
  
}


////////////////////////////////////////////
//                                        //
//      Fonctions Logique de la merde     //
//                                        //
////////////////////////////////////////////

//remet a 0 les stats
void reinit()
{
  sauverInt(highscore_addr,0);
  sauverInt(counter_addr,0);
  sauverInt(reboots_addr,0);
}

//indente le score et change l'affichage en fonction
void drink_beer()
{
  if(still_drinking == 1 ) {
    lcd_affiche_still_drinking();
    return;
  }
  //pour pas tricher, une bière toutes les 10 secondes, ça fait déjà un beau cul sec
  still_drinking =1 ;
  t.after(10 * 1000 ,[]{still_drinking = 0 ;});
  
  counter_total = lireInt(counter_addr);
  counter_total ++ ;
  sauverInt(counter_addr,counter_total);
  
  counter_today ++ ;   
  
  counter_highscore = lireInt(highscore_addr);
  if(counter_today>counter_highscore)
  {
    counter_highscore ++;
    sauverInt(highscore_addr,counter_highscore);
    lcd_affiche_new_highscore();
  }
  else
  {
    lcd_affiche_session();
  }
}





////////////////////////////////////////////
//                                        //
//   Fonctions affichage afficheur 7seg   //
//                                        //
////////////////////////////////////////////


//pour switcher entre "info" et "16"
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

//fonction qui fait briller les chiffres, visiblement faut la garder tout le temps...
void update_digits()
{
    myDisplay.DisplayString(tempString, 0); //(numberToDisplay, decimal point location)
}



//Pour initialiser l'afficheur 7 segments 4 digits
void init_digits()
{
  int displayType = COMMON_CATHODE; //Your display is either common cathode or common anode

  
  //This pinout is for a bubble dispaly
  //Declare what pins are connected to the GND pins (cathodes)
  int digit1 = 2; //Pin 1 -> ceux ci sont reliés à l'arduino avec des résistances de 330 ohm
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



////////////////////////////////////////////
//                                        //
//   Fonctions sauvegarde dans l'eeprom   //
//                                        //
////////////////////////////////////////////


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
