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
#define PORT 2024
#define MAX_CHR 2048

/* codul de eroare returnat de anumite apeluri */
extern int errno;
int main ()
{
    struct sockaddr_in server{};	// structura folosita de server
    struct sockaddr_in from{};
    char msgclient[MAX_CHR];		          //mesajul primit de la client
    char msgrasp[MAX_CHR]=" ";        //mesaj de raspuns pentru client
    int sd;			//descriptorul de socket

    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server]Eroare la socket().\n");
        return errno;
    }
    // conectare la baza de date
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    // open baza de date
    rc = sqlite3_open("topmusic.db", &db);

    if( rc ) {
        fprintf(stderr, "[server]Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    } else {
        fprintf(stderr, "[Server]Opened database successfully\n");
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
                    bzero(msgrasp, MAX_CHR);
                    if( command == 1 ){ // quit
                        printf("[server] Closing client connection...\n");
                        close(client);
                        break;
                    }
                    else
                        if( command == 2 ){ // login
                            if( adminORuser == 0 ) {
                                loginCommand(*msgclient, msgrasp, adminORuser); // execute login command
                            } else {
                                strcpy(msgrasp,"Already logged in!\n");
                                }
                            }
                        else
                            if( command == 3 ){ // register
                                if( adminORuser == 0 )
                                    strcpy(msgrasp,"Already logged in!\n");
                               // else registerCommand(msgclient,&msgrasp,&adminORuser); // execute register command
                                }
                            else
                                if( command == 4 ){
                                //delete song
                                }
                                else
                                    if( command == 5 ){
                                        // restrict vote for ...
                                    }
                                    else
                                        if( command == 6 ){
                                            // add song
                                        }
                                        else
                                            if( command == 7 ){
                                                // vote song
                                            }
                                            else
                                                if( command == 8 ){
                                                    // add comment
                                                }
                                                else
                                                    if( command == 9 ){
                                                        // see general top
                                                    }
                                                    else
                                                        if( command == 10 ){
                                                            // see top by genre
                                                        }
                                                        else strcpy(msgrasp,"Wrong format or inexistent command. Try again!\n");

                    /* returnam mesajul clientului */
                    if (write(client, msgrasp, MAX_CHR) <= 0) {
                        perror("[server]Eroare la write() catre client.\n");
                        continue;        /* continuam sa ascultam */
                    } else
                        printf("[server]Mesajul a fost trasmis cu succes.\n");

                }
                exit(0);
            }
            else continue;
    }				/* while */
    sqlite3_close(db); // inchidere conexiunea cu baza de date
}				/* main */
