/*********************************************************************************
*     File Name           :     save_eeprom.c
*     Created By          :     roger
*     Creation Date       :     [2017-08-26 11:38]
*     Last Modified       :     [2017-08-26 11:38]
*     Description         :      
**********************************************************************************/

void sauverInt(int adresse, int val) 
{   
    //découpage de la variable val qui contient la valeur à sauvegarder en mémoire
    //    unsigned char faible = val & 0x00FF; //récupère les 8 bits de droite (poids faible) -> 0010 1100 
    //        //calcul : 1101 0111 0010 1100 & 0000 0000 1111 1111 = 0010 1100
    //
    //            unsigned char fort = (val >> 8) & 0x00FF;  //décale puis récupère les 8 bits de gauche (poids fort) -> 1101 0111
    //                //calcul : 1101 0111 0010 1100 >> 8 = 0000 0000 1101 0111 puis le même & qu’avant
    //
    //                    //puis on enregistre les deux variables obtenues en mémoire
    //                        EEPROM.write(adresse, fort) ; //on écrit les bits de poids fort en premier
    //                            EEPROM.write(adresse+1, faible) ; //puis on écrit les bits de poids faible à la case suivante
    //                            }
    //
    //                            int lireInt(int adresse)
    //                            {
    //                                int val = 0 ; //variable de type int, vide, qui va contenir le résultat de la lecture
    //
    //                                    unsigned char fort = EEPROM.read(adresse);     //récupère les 8 bits de gauche (poids fort) -> 1101 0111
    //                                        unsigned char faible = EEPROM.read(adresse+1); //récupère les 8 bits de droite (poids faible) -> 0010 1100
    //
    //                                            //assemblage des deux variable précédentes
    //                                                val = fort ;         // val vaut alors 0000 0000 1101 0111
    //                                                    val = val << 8 ;     // val vaut maintenant 1101 0111 0000 0000 (décalage)
    //                                                        val = val | faible ; // utilisation du masque
    //                                                            // calcul : 1101 0111 0000 0000 | 0010 1100 = 1101 0111 0010 1100
    //
    //                                                                return val ; //on n’oublie pas de retourner la valeur lue !
    //                                                                }}
