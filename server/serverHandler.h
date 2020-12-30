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
void loginCommand(sqlite3* db,char* sql, int &adminORuser,char* serverResponse,const char* clientMessage, int identificator);
void registerCommand(sqlite3* db, int &adminORuser,char* serverResponse,const char* clientMessage);
void addSongCommand(sqlite3* db,const char* clientMessage, char* serverResponse);
void restrictVoteCommand(sqlite3* db,char* userName,char right[4],char* serverResponse);
void deleteSongCommand(sqlite3* db,char* clientMessage,char* serverResponse);
void voteSongCommand(sqlite3* db,char* clientMessage,char* serverResponse);
void addCommentCommand(sqlite3* db,char* clientMessage,char* serverResponse,char loggedIn[100]);
void see_top_general(sqlite3* db, char* serverResponse);
void see_users_command(sqlite3* db,char* clientMessage);
void see_top_genre(sqlite3* db,char* clientMessage,char* serverResponse);
void see_comments(sqlite3* db,char* clientMessage,char* serverResponse);
void see_genres_command(sqlite3* db,char* serverResponse);
void see_details(sqlite3* db,char* clientMessage,char* serverResponse);
int verify_vote_right(sqlite3* db,char user[100]);
void erase_empty_tables(sqlite3* db);
// not in main
static int callback(void *NowUsed, int argc, char **argv, char **azColName); // cu returnare rezultat
static int callbackUsers(void *NowUsed, int argc, char **argv, char **azColName);
static int callbackInsert(void *NotUsed, int argc, char **argv, char **azColName); // fara returnare rezultat
static int callbackID(void *NowUsed, int argc, char **argv, char **azColName); // pentru a luat ID-ul melodiei
static int callbackDetails(void *NowUsed, int argc, char **argv, char **azColName);

int numberofappearances(char text[MAX_CHR], char character);
void addInGenre(sqlite3* db,char ID[MAX_CHR],char genuri[200],char* serverResponse);

#endif //SERVER_SERVERHANDLER_H
