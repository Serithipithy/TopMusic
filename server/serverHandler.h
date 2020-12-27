//
// Created by alexandra on 12/13/20.
//

#ifndef SERVER_SERVERHANDLER_H
#define SERVER_SERVERHANDLER_H
#define MAX_CHR 2048
#include <sqlite3.h>
void parsingMsg (char *message );
int getCommand ( const char *message );
void getUserName(const char *str,char username[100]);
void loginCommand(sqlite3* db,char* sql, int &adminORuser,char* serverResponse,const char* clientMessage, int identificator, char* username);
void registerCommand(sqlite3* db, int &adminORuser,char* serverResponse,const char* clientMessage);
void addSongCommand(sqlite3* db,const char* clientMessage, char* serverResponse);
void restrictVoteCommand(sqlite3* db,char* userName,char right[4],char* serverResponse);
void deleteSongCommand(sqlite3* db,char* clientMessage,char* serverResponse);
void voteSongCommand(sqlite3* db,char* clientMessage,char* serverResponse);
int canvote(sqlite3* db,char* user);
// not in main
static int callback(void *NowUsed, int argc, char **argv, char **azColName); // cu returnare rezultat
static int callbackInsert(void *NotUsed, int argc, char **argv, char **azColName); // fara returnare rezultat
static int callbackID(void *NowUsed, int argc, char **argv, char **azColName); // pentru a luat ID-ul melodiei

int numberofappearances(char text[MAX_CHR], char character);
void addInGenre(sqlite3* db,char ID[MAX_CHR],char genuri[200],char* serverResponse);

#endif //SERVER_SERVERHANDLER_H
