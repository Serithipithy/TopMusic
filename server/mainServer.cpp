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
sqlite3 *db;
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
int main ()
{
    struct sockaddr_in server{};	// structura folosita de server
    struct sockaddr_in from{};
    char clientMessage[MAX_CHR];		          //mesajul primit de la client
    char serverResponse[MAX_CHR]=" ";        //mesaj de raspuns pentru client
    int sd;			//descriptorul de socket
    // conectare la baza de date
    char *zErrMsg = nullptr;
    char sql[MAX_CHR];
    char result[MAX_CHR];
    int rc;

    rc = sqlite3_open("topmusic.db", &db); // deschidere baza de date

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
    server.sin_family = AF_INET;                        /* stabilirea familiei de socket-uri */
    server.sin_addr.s_addr = htonl (INADDR_ANY);        /* acceptam orice adresa */
    server.sin_port = htons (PORT);                     /* utilizam un port utilizator */

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
                //bzero(clientMessage, MAX_CHR);
                memset(clientMessage,0,MAX_CHR);
                printf("[server]Asteptam mesajul...\n");
                fflush(stdout);
                memset(sql,0,sizeof (sql));
                memset(result,0,sizeof (result));
                /* citirea mesajului */
                if (read(client, clientMessage, MAX_CHR) <= 0) {
                    perror("[server]Eroare la read() de la client.\n");
                    close(client);    /* inchidem conexiunea cu clientul */
                    continue;        /* continuam sa ascultam */
                }
                parssingMsg(clientMessage); /* scoatem spatiile care nu ne sunt necesare */
                command = getCommand (clientMessage);
                printf("Dupa parsare: %s\n",clientMessage);
                memset(serverResponse, 0, sizeof (serverResponse));           /*pregatim mesajul de raspuns */

                if( command == 1 ){ // quit
                    printf("[server] Closing client connection...\n");
                    strcpy(serverResponse, "Quit!\n");
                }
                else
                if( command == 2 ){ // login user
                    if( adminORuser == 0 ) {
                        strcpy(clientMessage, clientMessage + strlen("login user "));
                        printf("%s\n",clientMessage);
                        sprintf(sql, "SELECT username FROM users WHERE username like '%s%c';", clientMessage,'%');
                        loginCommand(db,sql,adminORuser,serverResponse,clientMessage,2);
                    } else {
                        strcpy(serverResponse, "Already logged in!\n");
                    }
                }
                else
                if( command == 3 ) { // login admin
                    if (adminORuser == 0) {
                        strcpy(clientMessage, clientMessage + strlen("login admin "));
                        printf("%s\n", clientMessage);
                        sprintf(sql, "SELECT * FROM admins WHERE username='%s%c';", clientMessage,'%');
                        loginCommand(db,sql,adminORuser,serverResponse,clientMessage,1);
                    } else {
                     strcpy(serverResponse, "Already logged in!\n");
                    }
                }
                else
                if( command == 4 ){ // register
                    if (adminORuser == 0){
                        strcpy(clientMessage, clientMessage + strlen("register "));
                        printf("in register command: '%s'\n",clientMessage);
                        registerCommand(db,adminORuser,serverResponse,clientMessage);
                    }
                    else {
                        strcpy(serverResponse, "Logout first!\n");
                    }
                }
                else
                if( command == 5 ){ // delete song
                    if ( adminORuser == 0 ){
                        strcpy(serverResponse, "Login first!\n");
                    }
                    else
                        if ( adminORuser == 2 ){
                            strcpy(serverResponse, "You don't have the permission to use this command.\n");
                        }
                        else
                        {
                        strcpy(clientMessage, clientMessage + strlen("delete song "));
                        sprintf(sql, "SELECT * FROM songs WHERE ID='%s';", clientMessage);
                        rc = sqlite3_exec(db, sql, callback, result, &zErrMsg);

                        if( rc != SQLITE_OK ){
                            fprintf(stderr, "SQL error: %s\n", zErrMsg);
                            sqlite3_free(zErrMsg);
                        }
                        else{
                            if (strstr(clientMessage, result) != nullptr){
                                strcpy(serverResponse, "There is no song with this ID in the database. Try again!\n");
                            }
                            else{
                                sprintf(sql, "DELETE FROM songs WHERE ID=%s;", clientMessage); // TODO sterge si din restul tabelelor
                                rc = sqlite3_exec(db, sql, callbackInsert, 0, &zErrMsg);

                                if( rc != SQLITE_OK ){
                                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                }
                                sprintf(serverResponse, "The song with the ID:%s was deleted!\n",clientMessage);
                            }
                        }
                    }
                }
                else
                if( command == 6 ){ // restrict vote
                    if (adminORuser == 0){      // in cazul in care nu este conectat niciun fel de utilizator
                        strcpy(serverResponse, "Login first!\n");
                    }
                    else
                        if(adminORuser == 2){        // in cazul in care clietntul este conectat ca un user common
                            strcpy(serverResponse, "You don't have the permission to use this command.\n");
                        }
                        else
                        {
                            strcpy(clientMessage, clientMessage + strlen("restrict vote for "));
                            char* userName=getUserName(clientMessage);      // preluam usernameul utilizatorului caruia vrom sa ii schimba dreptul de a vota
                            strcpy(clientMessage,clientMessage + strlen(userName) + 1);         // pastram doar dreptul pe care vrem sa il schimbam
                            if(strcmp("yes",clientMessage) == 0 || strcmp("no",clientMessage) == 0){        // verificam daca comanda a fost introdusa corect si putem sa ii schimba drepturi
                                sprintf(sql, "SELECT * FROM users WHERE ID='%s';", userName);       // cautam utilizatorul
                                rc = sqlite3_exec(db, sql, callback, result, &zErrMsg);

                                if( rc != SQLITE_OK ){
                                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                }
                                else{
                                    if (strstr(clientMessage, result) != nullptr){      // usernameul nu s-a gasit
                                        strcpy(serverResponse, "There is no user with this ID in the database. Try again!\n");
                                    }
                                    else{       // usernameul s-a gasit, de ci se poate efectua schimbarea
                                        sprintf(sql, "UPDATE users SET canvote=%s WHERE ID=%s;", clientMessage, userName);      // modificam dreptul sau la vot
                                        rc = sqlite3_exec(db, sql, callbackInsert, 0, &zErrMsg);

                                        if( rc != SQLITE_OK ){
                                            fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                            sqlite3_free(zErrMsg);
                                        }
                                        sprintf(serverResponse, "The song with the ID:%s was deleted!\n", userName);        // mesaj de succes
                                    }
                                }
                            }
                            else
                                strcpy(serverResponse,"Wrong format. Try again!\n");

                        }
                }
                else
                if( command == 7 ){
                    strcpy(clientMessage, clientMessage + strlen("add song "));
                    printf("in add song command: '%s'\n",clientMessage);
                    // add song
                }
                else
                if( command == 8 ){
                    strcpy(clientMessage, clientMessage + strlen("vote song "));
                    // vote song
                }
                else
                if( command == 9 ){
                    strcpy(clientMessage, clientMessage + strlen("add comment "));
                    // add comm
                }
                else
                if( command == 10 ){
                    strcpy(clientMessage, clientMessage + strlen("see top general "));
                    // see top general
                }
                else
                if( command == 11 ){
                    strcpy(clientMessage, clientMessage + strlen("see top by "));
                    // see top by genre
                }
                else
                if ( command == 12 ){
                    strcpy(clientMessage, clientMessage + strlen("see users "));
                    // see users
                }
                else strcpy(serverResponse, "Wrong format or inexistent command. Try again!\n"); // no known command

                /* returnam mesajul clientului */
                if (write(client, serverResponse, strlen(serverResponse)) <= 0) {
                    perror("[server]Eroare la write() catre client.\n");
                    continue;        /* continuam sa ascultam */
                } else
                    printf("[server]Mesajul a fost trasmis cu succes.\n");

                if (strcmp(serverResponse, "Quit!\n") == 0){
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
