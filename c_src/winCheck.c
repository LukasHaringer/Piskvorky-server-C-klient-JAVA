#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "statistics.h"
#include "structures.h"
#include "winCheck.h"
#include "player.h"
#include "game.h"


/* ____________________________________________________________________________

        void printBoard(Game *game){

	Vypise do konzole hraci plochu dane game.
   ____________________________________________________________________________
*/
void printBoard(Game *game){
	int i,j;
	printf("HRACI PLOCHA:\n");
	for( i = 0; i<boardSize; i++){
	
		for(j =0; j<boardSize; j++){
			if(game->hraciPlocha[i][j] == '2'){
				printf("- ");
			}else{
				if(game->hraciPlocha[i][j] == '0'){
					printf("X ");
				}else{
					printf("0 ");
				}
			}
		}
		
		printf("\n");
	}
	


}


/* ____________________________________________________________________________

        int testBorders(int x,int y){

	Otestuje zda indexy prochazeni hraci plochy nepresahly jeji rozmery.
   ____________________________________________________________________________
*/
int testBorders(int x,int y){

	if(x>(boardSize-1) || x<0 || y>(boardSize-1) || y<0)
		return 1;
		
	return 0;
}

/* ____________________________________________________________________________

        int winCheck(int x, int y, Game *game){

	Zkontroluje zda tah na zadanych souradnicich nezpusobil vyhru hrace.
   ____________________________________________________________________________
*/
int winCheck(int x, int y, Game *game){
	
	// kontrola diagonalne zleva dole doprava nahoru
	int rada =0;
	int aktx = x-4;
	int akty = y+4;
	
	while(aktx <= x+4){	
		//printf("aktx: %d   akty: %d \n", aktx, akty);	
		// vrati jednicku pokud jsme mimo hraci plochu
		if(testBorders(aktx,akty)== 1){
			akty--;	
			aktx++;
			continue;
		}
		
		if( (game->hraciPlocha[akty][aktx]) == (game->hraciPlocha[y][x])) {
			//printf("HIT \n");
			//printf("znak akt: %c  znak xy: %c \n", game->hraciPlocha[aktx][akty], game->hraciPlocha[x][y]);
			
			
			rada++;
			if(rada == 5){
				//printf("Bingo1!!!!!!!!!!!!!!!!!!!!! \n");
				return 1;
			}
		}else{
			rada = 0;
		}
		akty--;	
		aktx++;
	}
	
	
	//diagonalne zleva nahore doprava dolu
	rada =0;
	aktx = x-4;
	akty = y-4;
	
	while(aktx <= x+4){		
			if(testBorders(aktx,akty)){
				aktx++;
				akty++;
				continue;
			}
			
			if( (game->hraciPlocha[akty][aktx]) == (game->hraciPlocha[y][x])){
				rada++;
				if(rada == 5){
					//printf("Bingo2!!!!!!!!!!!!!!!!!!!!! \n");
					return 1;
				}
			}else{
				rada = 0;
			}
			aktx++;
			akty++;
	}
	
	
	
	// kontrola horizontalne[
	rada =0;
	aktx = x-4;
	akty = y;
	
	while(aktx <= x+4){		
			if(testBorders(aktx,akty)){
				aktx++;
				continue;
			}
			
			if((game->hraciPlocha[akty][aktx]) == (game->hraciPlocha[y][x])){
				rada++;
				if(rada == 5){
					//printf("Bingo3!!!!!!!!!!!!!!!!!!!!! \n");
					return 1;
				}
			}else{
				rada = 0;
			}
			aktx++;
	}
	
	
	//kontrola vertikalne
	rada =0;
	aktx = x;
	akty = y-4;
	while(akty <= y+4){		
			if( testBorders(aktx,akty) ){
				akty++;
				continue;
			}
			
			if( (game->hraciPlocha[akty][aktx]) == (game->hraciPlocha[y][x] )) {
				rada++;
				if(rada == 5){
				//	printf("Bingo4!!!!!!!!!!!!!!!!!!!!! \n");
					return 1;
				}
			}else{
				rada = 0;
			}
			akty++;
	}
	
	return 0;
}



/* ____________________________________________________________________________

        int drawCheck(Game *game){

	Urci zda se hra nedostala do remizi.
   ____________________________________________________________________________
*/
int drawCheck(Game *game){
	if(game->zaplnenoPolicek>=230){
		return 1;
		
	}
	return 0;
}
	
