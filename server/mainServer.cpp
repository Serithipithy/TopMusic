#include "serverHandler.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sqlite3.h>

/* portul folosit */
#define PORT 2010
#define MAX_CHR 2048

/* codul de eroare returnat de anumite apeluri */
extern int errno;
static int callback(void *str, int argc, char **argv, char **azColName){
    int i;
    char* data= (char*) str;
    for(i = 0; i<argc; i++) {
        strcat(data,azColName[i]);
        strcat(data," : ");
        if(argv[i])
            strcat(data,argv[i]);
        else
            strcat(data,"NULL");
        strcat (data, "\n");
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    strcat(data,"\n");
    return 0;
}
static int callbackInsert(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for(i = 0; i<argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}
int main ()
{
    struct sockaddr_in server{};	// structura folosita de server
    struct sockaddr_in from{};
    char msgclient[MAX_CHR];		          //mesajul primit de la client
    char msgrasp[MAX_CHR]=" ";        //mesaj de raspuns pentru client
    int sd;			//descriptorul de socket
    // conectare la baza de date
    sqlite3 *db;
    //char *zErrMsg = 0;
    char sql[MAX_CHR];
    char result[MAX_CHR];
    int rc;

    // open baza de date
    rc = sqlite3_open("topmusic.db", &db);

    if( rc ) {
        fprintf(stderr, "[server]Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    } else {
        fprintf(stderr, "[Server]Opened database successfully\n");
    }

    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server]Eroare la socket().\n");
        return errno;
    }

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 5) == -1)
    {
        perror ("[server]Eroare la listen().\n");
        return errno;
    }

    while (true)
    {
        int client;
        socklen_t length = sizeof (from);

        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        client = accept (sd, (struct sockaddr *) &from, &length);

        /* eroare la acceptarea conexiunii de la un client */
        if (client < 0)
        {
            perror ("[server]Eroare la accept().\n");
            continue;
        }

        // creare fork()
        pid_t pid=fork();

        // tratare erori
        if( pid == -1 ) {
            perror("> Error forking \n");
            exit(-1);
        }
        else
        if( pid == 0 ) { // procesul copil
            int adminORuser=0;
            int command;

            while (true) {
                /* s-a realizat conexiunea, se astepta mesajul */
                bzero(msgclient, MAX_CHR);
                printf("[server]Asteptam mesajul...\n");
                fflush(stdout);

                /* citirea mesajului */
                if (read(client, msgclient, MAX_CHR) <= 0) {
                    perror("[server]Eroare la read() de la client.\n");
                    close(client);    /* inchidem conexiunea cu clientul */
                    continue;        /* continuam sa ascultam */
                }

                parssingMsg(msgclient); // scoatem spatiile care nu ne sunt necesare
                command = getCommand (msgclient);

                /*pregatim mesajul de raspuns */
                memset(msgrasp, 0,sizeof (msgrasp));
                if( command == 1 ){ // quit
                    printf("[server] Closing client connection...\n");
                    strcpy(msgrasp,"Quit!\n");
                }
                else
                if( command == 2 ){ // login user

                    if( adminORuser == 0 ) {
                        strcpy(msgclient,msgclient+strlen("login user "));
                        printf("%s\n",msgclient);
                        char *zErrMsg= nullptr;
                        memset(sql,0,sizeof (sql));
                        memset(sql,0,sizeof (result));
                        sprintf(sql,"SELECT * FROM users WHERE username='%s';",msgclient);
                        rc = sqlite3_exec(db, sql, callback, result, &zErrMsg);
                        if( rc != SQLITE_OK ){
                            fprintf(stderr, "SQL error: %s\n", zErrMsg);
                            sqlite3_free(zErrMsg);
                            adminORuser = 0;
                        }
                        else
                        if(strstr(result,msgclient)){
                            printf("%s\n",result);
                            strcpy(msgrasp,"Logged in!\n");
                            adminORuser = 2;
                        }
                        else {
                            strcpy(msgrasp,"Wrong username! Try again or register now.\n");
                            adminORuser = 0;
                        }
                    } else {
                        strcpy(msgrasp,"Already logged in!\n");
                    }
                }
                else
                if( command == 3 ) { // login admin
                    if (adminORuser == 0) {
                        strcpy(msgclient, msgclient + strlen("login admin "));
                        strcpy(msgclient,msgclient+strlen("login user "));
                        printf("%s\n",msgclient);
                        char *zErrMsg= nullptr;
                        memset(sql,0,sizeof (sql));
                        memset(sql,0,sizeof (result));
                        sprintf(sql,"SELECT * FROM admins WHERE username='%s';",msgclient);
                        rc = sqlite3_exec(db, sql, callback, result, &zErrMsg);
                        if( rc != SQLITE_OK ){
                            fprintf(stderr, "SQL error: %s\n", zErrMsg);
                            sqlite3_free(zErrMsg);
                            adminORuser = 0;
                        }
                        else
                        if(strstr(result,msgclient)){
                            printf("%s\n",result);
                            strcpy(msgrasp,"Logged in!\n");
                            adminORuser = 1;
                        }
                        else {
                            strcpy(msgrasp,"Wrong username! Try again or register now.\n");
                            adminORuser = 0;
                        }
                    }
                    else {
                     strcpy(msgrasp, "Already logged in!\n");
                }
                }
                else
                if( command == 4 ){ // register
                    if (adminORuser == 0){
                        strcpy(msgclient, msgclient + strlen("register "));
                        char *zErrMsg = nullptr;
                        memset(sql,0,sizeof (sql));
                        memset(sql,0,sizeof (result));;

                        sprintf(sql,"SELECT * FROM users WHERE username='%s';",msgclient);
                        rc = sqlite3_exec(db, sql, callback, result, &zErrMsg);

                        if( rc != SQLITE_OK ){
                            fprintf(stderr, "SQL error: %s\n", zErrMsg);
                            sqlite3_free(zErrMsg);
                            adminORuser = 0;
                        }
                        else
                        if(strstr(msgclient,result) != nullptr){
                            strcpy(msgrasp,"This username already exists. Login or try again!\n");
                            adminORuser = 0;
                        }
                        else {
                            sql[0]=0;
                            result[0]=0;

                            sprintf(sql,"INSERT INTO users (username, canvote) VALUES ('%s','yes');",msgclient);
                            rc = sqlite3_exec(db, sql, callbackInsert, 0, &zErrMsg);

                            if( rc != SQLITE_OK ){
                                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                adminORuser = 0;
                            }
                            strcpy(msgrasp,"Succesfull registration!\n");
                            adminORuser = 2;
                        }
                    }
                    else {
                        strcpy(msgrasp,"Logout first!\n");
                    }
                }
                else
                if( command == 5 ){
                    strcpy(msgclient, msgclient + strlen("delete song "));
                    // delete song
                }
                else
                if( command == 6 ){
                    strcpy(msgclient, msgclient + strlen("restrict vote for "));
                    // restrict vote
                }
                else
                if( command == 7 ){
                    strcpy(msgclient, msgclient + strlen("add song "));
                    // add song
                }
                else
                if( command == 8 ){
                    strcpy(msgclient, msgclient + strlen("vote song "));
                    // vote song
                }
                else
                if( command == 9 ){
                    strcpy(msgclient, msgclient + strlen("add comment "));
                    // add comm
                }
                else
                if( command == 10 ){
                    strcpy(msgclient, msgclient + strlen("see top general "));
                    // see top general
                }
                else
                if( command == 11 ){
                    strcpy(msgclient, msgclient + strlen("see top by "));
                    // see top by genre
                }
                else
                if ( command == 12 ){
                    strcpy(msgclient, msgclient + strlen("see users "));
                    // see users
                }
                else strcpy(msgrasp,"Wrong format or inexistent command. Try again!\n"); // no known command

                /* returnam mesajul clientului */
                if (write(client, msgrasp, strlen(msgrasp)) <= 0) {
                    perror("[server]Eroare la write() catre client.\n");
                    continue;        /* continuam sa ascultam */
                } else
                    printf("[server]Mesajul a fost trasmis cu succes.\n");

                if (strcmp(msgrasp,"Quit!\n") == 0){
                    close(client);
                    break;
                }

            }
            exit(0);
        }
        else continue;
    }				/* while */
    sqlite3_close(db); // inchidere conexiunea cu baza de date
}				/* main */