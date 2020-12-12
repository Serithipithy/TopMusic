//
// Created by alexandra on 12/10/20.
//

#include "serverHandler.h"
//#include <iostream>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <errno.h>
//#include <unistd.h>
//#include <stdio.h>
#include <cstring>
#include <fstream>
//#include <stdlib.h>
//#include <stdio.h>
#include "json.hpp"

// for convenience
using json = nlohmann::json;
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
    char login[]="login";
    char new_register[]="register";
    char deleteSong[]="delete song";
    char restrictVote[]="restrict vote for";
    char addSong[]="add song";
    char voteSong[]="vote song";
    char addComment[]="add comment";
    char seeTopGeneral[]="see top general";
    char seeTopByGenre[]="see top by";

    if(strstr( message, quit) != nullptr) return 1;
    else
        if(strstr( message, login) != nullptr) return 2;
        else
            if(strstr( message, new_register) != nullptr) return 3;
            else
                if(strstr( message, deleteSong) != nullptr) return 4;
                else
                    if(strstr( message, restrictVote) != nullptr) return 5;
                    else
                        if(strstr( message, addSong) != nullptr) return 6;
                        else
                            if(strstr( message, voteSong) != nullptr) return 7;
                            else
                                if(strstr( message, addComment) != nullptr) return 8;
                                else
                                    if(strstr( message, seeTopGeneral) != nullptr) return 9;
                                    else
                                        if(strstr( message, seeTopByGenre) != nullptr) return 10;
    return 0;
}
void loginCommand ( const char message, char* messageToSend,int& userType ){
    std::ifstream myfile("account.json");
    json j;
    myfile >> j;

    strcpy((char *) message, reinterpret_cast<const char *>(message + 6));
    auto namepass = j.find(message);

    if( namepass != j.end() )
        strcpy(messageToSend,"There is no such username or wrong password.\n");
    else {
        userType = *namepass;
        strcpy(messageToSend,"Logged in!\n");
    }

}
