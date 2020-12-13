//
// Created by alexandra on 12/10/20.
//

#include "serverHandler.h"
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <errno.h>
//#include <unistd.h>
#include <cstring>
//#include <stdlib.h>
#include <cstdio>
#include <sqlite3.h>

#define MAX_CHR 2048

using namespace std;
void parssingMsg ( char *message){
    // prelucrare text
    char wk_text[MAX_CHR];
    int i=0,k=0;
    while ( i != strlen(message) ){
        if ( k==0 && message[i]!=' ' )
            wk_text[k++]=message[i];
        else
        if ( message[i]!=' ' && message[i-1]==' '){
            wk_text[k++]=' ';
            wk_text[k++]=message[i];
        }
        else
        if ( message[i]!=' ' && message[i-1]!=' ')
            wk_text[k++]=message[i];
        i++;

    }
    strcpy( message, wk_text );
}
int getCommand ( const char* message ){
    char quit[]="quit";
    char loginUser[]="login user";
    char loginAdmin[]="login admin";
    char new_register[]="register";
    char deleteSong[]="delete song";
    char restrictVote[]="restrict vote for";
    char addSong[]="add song";
    char voteSong[]="vote song";
    char addComment[]="add comment";
    char seeTopGeneral[]="see top general";
    char seeTopByGenre[]="see top by";
    char seeUsers[]="see users";

    if(strstr( message, quit) != nullptr) return 1;
    else
    if(strstr( message, loginUser) != nullptr) return 2;
    else
    if (strstr( message, loginAdmin) != nullptr) return 3;
    else
    if(strstr( message, new_register) != nullptr) return 4;
    else
    if(strstr( message, deleteSong) != nullptr) return 5;
    else
    if(strstr( message, restrictVote) != nullptr) return 6;
    else
    if(strstr( message, addSong) != nullptr) return 7;
    else
    if(strstr( message, voteSong) != nullptr) return 8;
    else
    if(strstr( message, addComment) != nullptr) return 9;
    else
    if(strstr( message, seeTopGeneral) != nullptr) return 10;
    else
    if(strstr( message, seeTopByGenre) != nullptr) return 11;
    else
    if(strstr( message, seeUsers) != nullptr) return 12;
    else return 0;
}