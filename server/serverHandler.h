//
// Created by alexandra on 12/13/20.
//

#ifndef SERVER_SERVERHANDLER_H
#define SERVER_SERVERHANDLER_H
#define MAX_CHR 2048
#include <sqlite3.h>
void parssingMsg (char *message );
int getCommand ( const char *message );
char* getUserName(const char *str);
void loginCommand(sqlite3* db,char* sql, int &adminORuser,char* serverResponse,const char* clientMessage, int identificator);
void registerCommand(sqlite3* db, int &adminORuser,char* serverResponse,const char* clientMessage);
void addSongCommand(sqlite3* db,const char* clientMessage, char* serverResponse);
// not in main
static int callback(void *NowUsed, int argc, char **argv, char **azColName);
static int callbackInsert(void *NotUsed, int argc, char **argv, char **azColName);

int numberofappearances(char text[MAX_CHR], char character);

#endif //SERVER_SERVERHANDLER_H
