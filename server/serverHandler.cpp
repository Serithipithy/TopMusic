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
#include <iostream>
#include <sqlite3.h>
//#include <stdlib.h>

#define MAX_CHR 2048

using namespace std;
void parssingMsg ( char *message){
    // prelucrare text
    char wk_text[MAX_CHR];
    memset(wk_text,0,sizeof(wk_text));
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
    char help[]="help";

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
    else
    if (strstr(message,help) != nullptr) return 13;
    else return 0;
}
char* getUserName(const char *str){
    char *username = nullptr;
    int i=0,k=0;
    while( i != strlen(str) - 1 ){
        if(str[i] != ' ')
            username[k++]=str[i];
        else
            break;
        i++;
    }
    return username;
}
void loginCommand(sqlite3* db,char* sql, int &adminORuser,char* serverResponse, const char* clientMessage, int identificator){
    char *zErrMsg = nullptr;
    char result[MAX_CHR];
    int rc;
    memset(result,0,sizeof(result));

    rc = sqlite3_exec(db, sql, callback, result, &zErrMsg);
    printf("Dupa sql: '%s'\n",result);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    if(strstr(result, clientMessage)){
        printf("%s\n",result);
        strcpy(serverResponse, "Logged in!\n");
        adminORuser = identificator;
    }
    else {
        strcpy(serverResponse, "Wrong username! Try again or register now.\n");
    }
}
void registerCommand(sqlite3* db, int &adminORuser,char* serverResponse,const char* clientMessage){
    char *zErrMsg = nullptr;
    char result[MAX_CHR];
    char sql[MAX_CHR];
    int rc;
    memset(result,0,sizeof(result));
    memset(sql,0,sizeof(sql));

    sprintf(sql, "INSERT INTO users (username, canvote) VALUES ('%s','yes');", clientMessage);
    rc = sqlite3_exec(db, sql, callbackInsert, 0, &zErrMsg);

    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        strcpy(serverResponse, "Can't create an account with this username because this username already exists. Login or try again!\n");
        sqlite3_free(zErrMsg);
    } else {
        strcpy(serverResponse, "Succesfull registration!\n");
        adminORuser = 2;
    }
}
void addSongCommand(sqlite3* db,const char* clientMessage, char* serverResponse){
    char text[MAX_CHR],titlu[100],descriere[1000],link[200];
    int k;
    memset(text,0, sizeof(text));
    memset(titlu,0, sizeof(titlu));
    memset(descriere,0,sizeof(descriere));
    memset(link,0,sizeof(link));

    strcpy(text,clientMessage);
    if(numberofappearances(text,'<') != 3 || numberofappearances(text,'>') != 3){
        strcpy(serverResponse,"Wrong format! add song <title> <description> <link> \n");
    }
    else
        if(text[0] != '<' || text[strlen(text)-1] != '>')
            strcpy(serverResponse,"Wrong format! add song <title> <description> <link> \n");
        else{
            strcpy(text,text+1);
            k=0;
            while (text[0] != '>'){
                titlu[k++]=text[0];
                strcpy(text,text+1);
            }

            if( text[1] != ' ' || text[2] != '<' )
                strcpy(serverResponse,"Wrong format! add song <title> <description> <link> \n");
            else{
                strcpy(text,text+3); // stergem "> <"
                k=0;
                while (text[0] != '>'){
                    descriere[k++]=text[0];
                    strcpy(text,text+1);
                }
                if( text[1] != ' ' || text[2] != '<' )
                    strcpy(serverResponse,"Wrong format! add song <title> <description> <link> \n");
                else{
                    strcpy(text,text+3);
                    k=0;
                    while (text[0] != '>') {
                        link[k++] = text[0];
                        strcpy(text, text + 1);
                    }
                    if (strlen(titlu) < 1 || strlen(descriere) < 1 || strlen(link) < 1)
                        strcpy(serverResponse,"Wrong format! One of the atributes is empty. add song <title> <description> <link> \n");
                    else { // adaugare in baza de date
                        char *zErrMsg = nullptr;
                        char result[MAX_CHR];
                        char sql[MAX_CHR];
                        int rc;
                        memset(result,0,sizeof(result));
                        memset(sql,0,sizeof(sql));

                        sprintf(sql, "INSERT INTO songs (titlu, descriere, link) VALUES ('%s','%s','%s');", titlu,descriere,link);
                        rc = sqlite3_exec(db, sql, callbackInsert, 0, &zErrMsg);

                        if( rc != SQLITE_OK ){
                            fprintf(stderr, "SQL error: %s\n", zErrMsg);
                            strcpy(serverResponse, "Can't add this song.\n");
                            sqlite3_free(zErrMsg);
                        } else {
                            strcpy(serverResponse, "Song added!\n");
                        }
                    }
                }
            }
        }
}

// not in main
static int callback(void *NowUsed, int argc, char **argv, char **azColName){
    char* data= (char*) NowUsed;
    for(int i = 0; i<argc; i++) {
        strcat(data,azColName[i]);
        strcat(data," : ");
        strcat(data,argv[i] ? argv[i] : "NULL");
        strcat (data, "\n");
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    strcat(data,"\n");
    return 0;
}
static int callbackInsert(void *NotUsed, int argc, char **argv, char **azColName) {
    for(int i = 0; i<argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int numberofappearances(char text[MAX_CHR], char character){ // numarul de aparitii ale unui caracter
    int app=0;
    for( int i = 0; i < strlen(text) ; i++)
        if(text[i] == character)app++;
    return app;
}
