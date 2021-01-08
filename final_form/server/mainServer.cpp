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
#define PORT 2023
#define MAX_CHR 2048

extern int errno;

sqlite3 *db;

static int callbackInsert(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main() {
    struct sockaddr_in server{};    // structura folosita de server
    struct sockaddr_in from{};
    char clientMessage[MAX_CHR];                  //mesajul primit de la client
    char serverResponse[MAX_CHR];        //mesaj de raspuns pentru client
    int sd;            //descriptorul de socket
    // conectare la baza de date
    char *zErrMsg = nullptr;
    char sql[MAX_CHR];
    char result[MAX_CHR];
    int rc;

    rc = sqlite3_open("topmusic.db", &db); // deschidere baza de date

    if (rc) {
        fprintf(stderr, "[server]Can't open database: %s\n", sqlite3_errmsg(db));
        return (0);
    } else {
        fprintf(stderr, "[Server]Opened database successfully\n");
    }

    memset(sql, 0, sizeof(sql)); // pentru delete cascade
    sprintf(sql, "PRAGMA foreign_keys=on;");

    rc = sqlite3_exec(db, sql, callbackInsert, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stderr, "[Server]Foreign_keys activated.\n");
    }

    /* crearea unui socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[server]Eroare la socket().\n");
        return errno;
    }

    /* pregatirea structurilor de date */
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    /* umplem structura folosita de server */
    server.sin_family = AF_INET;                        /* stabilirea familiei de socket-uri */
    server.sin_addr.s_addr = htonl(INADDR_ANY);        /* acceptam orice adresa */
    server.sin_port = htons(PORT);                     /* utilizam un port utilizator */

    /* atasam socketul */
    if (bind(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 5) == -1) {
        perror("[server]Eroare la listen().\n");
        return errno;
    }

    while (true) {
        int client;
        socklen_t length = sizeof(from);

        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        client = accept(sd, (struct sockaddr *) &from, &length);

        /* eroare la acceptarea conexiunii de la un client */
        if (client < 0) {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        // creare fork()
        pid_t pid = fork();

        // tratare erori
        if (pid == -1) {
            perror("> Error forking \n");
            exit(-1);
        } else if (pid == 0) { // procesul copil
            int adminORuser = 0;
            int command;
            char loggedIn[100];

            while (true) {
                /* s-a realizat conexiunea, se astepta mesajul */
                //bzero(clientMessage, MAX_CHR);
                memset(clientMessage, 0, MAX_CHR);
                printf("[server]Asteptam mesajul...\n");
                fflush(stdout);
                memset(sql, 0, sizeof(sql));
                memset(result, 0, sizeof(result));
                /* citirea mesajului */
                if (read(client, clientMessage, MAX_CHR) <= 0) {
                    perror("[server]Eroare la read() de la client.\n");
                    close(client);    /* inchidem conexiunea cu clientul */
                    continue;        /* continuam sa ascultam */
                }
                parsingMsg(clientMessage); /* scoatem spatiile care nu ne sunt necesare */
                command = getCommand(clientMessage);
                printf("[server]Dupa parsare: %s\n", clientMessage);
                memset(serverResponse, 0, sizeof(serverResponse));           /*pregatim mesajul de raspuns */

                if (command == 1) { // quit
                    printf("[server] Closing client connection...\n");
                    strcpy(serverResponse, "Quit!\n");
                } else if (command == 2) { // login user
                    if (adminORuser == 0) {
                        strcpy(clientMessage, clientMessage + strlen("login user "));
                        printf("[server]%s\n", clientMessage);
                        memset(sql, 0, sizeof(sql));
                        sprintf(sql, "SELECT username FROM user WHERE username like '%s';", clientMessage);
                        loginCommand(db, sql, adminORuser, serverResponse, clientMessage, 2);
                        if (strcmp("Logged in!\n", serverResponse) == 0)
                            strcpy(loggedIn, clientMessage);
                        fprintf(stderr, "%s\n", loggedIn);
                    } else {
                        strcpy(serverResponse, "Already logged in!\n");
                    }
                } else if (command == 3) { // login admin
                    if (adminORuser == 0) {
                        strcpy(clientMessage, clientMessage + strlen("login admin "));
                        printf("[server]%s\n", clientMessage);
                        memset(sql, 0, sizeof(sql));
                        sprintf(sql, "SELECT * FROM admins WHERE username='%s';", clientMessage);
                        loginCommand(db, sql, adminORuser, serverResponse, clientMessage, 1);
                    } else {
                        strcpy(serverResponse, "Already logged in!\n");
                    }
                } else if (command == 4) { // register
                    if (adminORuser == 0) {
                        strcpy(clientMessage, clientMessage + strlen("register "));
                        printf("[server]in register command: '%s'\n", clientMessage);
                        registerCommand(db, adminORuser, serverResponse, clientMessage,loggedIn);
                    } else {
                        strcpy(serverResponse, "Logout first!\n");
                    }
                } else if (command == 5) { // delete song
                    if (adminORuser == 0) {
                        strcpy(serverResponse, "Login first!\n");
                    } else if (adminORuser == 2) {
                        strcpy(serverResponse, "You don't have the permission to use this command.\n");
                    } else {
                        strcpy(clientMessage, clientMessage + strlen("delete song "));
                        deleteSongCommand(db, clientMessage, serverResponse);
                        erase_empty_tables(db);
                    }
                } else if (command == 6) { // restrict vote
                    if (adminORuser == 0) {      // in cazul in care nu este conectat niciun fel de utilizator
                        strcpy(serverResponse, "Login first!\n");
                    } else if (adminORuser == 2) {        // in cazul in care clietntul este conectat ca un user common
                        strcpy(serverResponse, "You don't have the permission to use this command.\n");
                    } else {
                        char userName[100];
                        char right[4];
                        strcpy(clientMessage, clientMessage + strlen("restrict vote for "));

                        getUserName(clientMessage, userName);      // preluam usernameul utilizatorului caruia vrom sa ii schimba dreptul de a vota
                        strcpy(right, clientMessage + strlen(userName) + 1);         // pastram doar dreptul pe care vrem sa il schimbam
                        restrictVoteCommand(db, userName, right, serverResponse);
                    }
                } else if (command == 7) {     // add song
                    if (adminORuser == 0) {      // in cazul in care nu este conectat niciun fel de utilizator
                        strcpy(serverResponse, "Login first!\n");
                    } else if (adminORuser == 1) {
                        strcpy(serverResponse, "You don't have the permission to use this command.");
                    } else {
                        strcpy(clientMessage, clientMessage + strlen("add song "));
                        addSongCommand(db, clientMessage, serverResponse);
                    }
                } else if (command == 8) {
                    if (adminORuser == 0) {      // in cazul in care nu este conectat niciun fel de utilizator
                        strcpy(serverResponse, "Login first!\n");
                    } else if (adminORuser == 1) { // vote song
                        strcpy(serverResponse, "You don't have the permission to use this command.\n");
                    } else {
                        strcpy(clientMessage, clientMessage + strlen("vote song "));
                        if (verify_vote_right(db, loggedIn) == 1)
                            voteSongCommand(db, clientMessage, serverResponse);
                        else {
                            strcpy(serverResponse,
                                   "You can't vote right now because an admin took this right from you.\n");
                        }

                    }
                } else if (command == 9) { // add comments
                    if (adminORuser == 0) {      // in cazul in care nu este conectat niciun fel de utilizator
                        strcpy(serverResponse, "Login first!\n");
                    } else if (adminORuser == 1) {
                        strcpy(serverResponse, "You don't have the permission to use this command.\n");
                    } else {
                        strcpy(clientMessage, clientMessage + strlen("add comment ")); // add comment <comment> <ID>
                        addCommentCommand(db, clientMessage, serverResponse, loggedIn);
                    }
                } else if (command == 10) { // see top by genre
                    if (adminORuser == 0) {
                        strcpy(serverResponse, "Login first!\n");
                    } else if (adminORuser == 1) {
                        strcpy(serverResponse, "You don't have the permission to use this command.\n");
                    } else {
                        strcpy(clientMessage, clientMessage + strlen("see top by "));
                        if (strlen(clientMessage) < 1) {
                            strcpy(serverResponse, "Wrong format! see top by choose_genre\n");
                        } else
                            see_top_genre(db, clientMessage, serverResponse);
                    }
                } else if (command == 11) { // see top general
                    if (adminORuser == 0) {
                        strcpy(serverResponse, "Login first!\n");
                    } else {
                        strcpy(clientMessage, clientMessage + strlen("see top general "));
                        if (strlen(clientMessage) > 0) {
                            strcpy(serverResponse, "Wrong format! see top general\n");
                        } else
                            see_top_general(db, serverResponse);
                    }
                } else if (command == 12) { //see users
                    if (adminORuser == 0) {
                        strcpy(serverResponse, "Login first!\n");
                    } else if (adminORuser == 1) {
                        strcpy(clientMessage, clientMessage + strlen("see users "));
                        if (strlen(clientMessage) > 0) {
                            strcpy(serverResponse, "Wrong format! see users\n");
                        } else {
                            see_users_command(db, serverResponse);
                        }
                    } else {
                        strcpy(serverResponse, "You don't have the permission to use this command.\n");
                    }
                } else if (command == 13) { // help
                    if (adminORuser == 0)
                        strcpy(serverResponse, "\n\tCommands available:\n"
                                               "1. help \t\t\t\\\\will show you what commands you can execute while not logged in or logged in as user or admin\n"
                                               "2. login user your_username \t\\\\if you want to login as an user\n"
                                               "3. login admin your_username \t\\\\if you want to login as an admin\n"
                                               "4. register your_new_username \t\\\\will create a new username\n"
                                               "5. quit\t\t\t\t\\\\will close the application\n");
                    else if (adminORuser == 2)
                        strcpy(serverResponse, "\n\tCommands available for users:\n"
                                               "1. help \t\t\t\t\t\t\\\\will show you what commands you can execute while not logged in or logged in as user or admin\n"
                                               "2. add song <title> <description> <link> <genre(s)> \t\\\\you can add a new song to the top (genres need to be separated by ',')\n"
                                               "3. vote song id_song \t\t\t\t\t\\\\you can vote a song based on its id if you have the privilege\n"
                                               "4. add comment <your_comment> <song_ID> \t\t\\\\you can add a comment to a song based on its id\n"
                                               "5. see top general \t\t\t\t\t\\\\will show you the general top based on votes\n"
                                               "6. see to by chosen_genre \t\t\t\t\\\\will show you the top based on a genre and votes\n"
                                               "7. see genres \t\t\t\t\t\t\\\\will show you the genres that exist in the database\n"
                                               "8. see comments for song id_song \t\t\t\\\\will show the comments for the chosen song\n"
                                               "9. see details for song id_song \t\t\t\\\\will show the details for the chosen song\n"
                                               "10. logout \t\t\t\t\t\t\\\\will log you out\n"
                                               "11. quit\t\t\t\t\t\t\\\\will close the application\n");
                    else
                        strcpy(serverResponse, "\n\tCommands available for admins:\n"
                                               "1. help \t\t\t\t\t\\\\will show you what commands you can execute while not logged in or logged in as user or admin\n"
                                               "2. see top general \t\t\t\t\\\\will show you the general top based on votes\n"
                                               "3. delete song id_song \t\t\t\t\\\\will delete the song with the id given if exists\n"
                                               "4. see users \t\t\t\t\t\\\\will show you the list of usernames\n"
                                               "5. restrict vote for chosen_username yes/no \t\\\\will change the right to vote of the chosen user\n"
                                               "6. see comments for song id_song \t\t\\\\will show the comments for the chosen song\n"
                                               "7. see details for song id_song \t\t\\\\will show the details for the chosen song\n"
                                               "8. logout \t\t\t\t\t\\\\will log you out\n"
                                               "9. quit\t\t\t\t\t\t\\\\will close the application\n");
                } else if (command == 14) { // logout
                    if (adminORuser == 0) {
                        strcpy(serverResponse, "Your are not logged in yet.\n");
                    } else {
                        adminORuser = 0;
                        memset(loggedIn, 0, sizeof(loggedIn));
                        strcpy(serverResponse, "Logout! See you next time.\n");
                    }
                } else if (command == 15) { // see genres
                    if (adminORuser == 0) {
                        strcpy(serverResponse, "Login first!\n");
                    } else if (adminORuser == 1) {
                        strcpy(serverResponse, "You don't have the permission to use this command.\n");
                    } // doar pe cele care au ceva in ele
                    else {
                        see_genres_command(db, serverResponse);
                    }
                } else if (command == 16) { // see comments
                    if (adminORuser == 0) {
                        strcpy(serverResponse, "Your are not logged in yet.\n");
                    } else {
                        strcpy(clientMessage, clientMessage + strlen("see comments for song "));
                        if (strlen(clientMessage) < 1) {
                            strcpy(serverResponse, "Wrong format! see comments for song id_song\n");
                        } else {
                            see_comments(db, clientMessage, serverResponse);
                        }
                    }
                } else if (command == 17) { // see details
                    if (adminORuser == 0) {
                        strcpy(serverResponse, "Your are not logged in yet.\n");
                    } else {
                        strcpy(clientMessage, clientMessage + strlen("see details for song "));
                        if (strlen(clientMessage) < 1) {
                            strcpy(serverResponse, "Wrong format! see details for song id_song\n");
                        } else {
                            see_details(db, clientMessage, serverResponse);
                        }
                    }
                } else
                    strcpy(serverResponse, "Wrong format or nonexistent command. Try again!\n"); // no known command
                /* returnam mesajul clientului */
                if (write(client, serverResponse, strlen(serverResponse)) <= 0) {
                    perror("[server]Eroare la write() catre client.\n");
                    continue;        /* continuam sa ascultam */
                } else
                    printf("[server]Mesajul a fost trasmis cu succes.\n");

                if (strcmp(serverResponse, "Quit!\n") == 0) {
                    close(client);
                    break;
                }
            }
            exit(0);
        } else continue;
    }                /* while */
    sqlite3_close(db); // inchidere conexiunea cu baza de date
}                /* main */