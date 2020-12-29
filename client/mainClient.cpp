#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <fstream>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
    int sd;			// descriptorul de socket
    struct sockaddr_in server;	// structura folosita pentru conectare
    char msgToSend[2048];		// mesajul trimis
    char msgRead[2048];          // mesajul citit

    /* exista toate argumentele in linia de comanda? */
    if (argc != 3){
        printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    port = atoi (argv[2]);

    /* cream socketul */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1){
        perror ("Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons (port);

    /* ne conectam la server */
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1){
        perror("[client]Eroare la connect().\n");
        return errno;
    }
    // TODO :mesaj de intampinare
    while(1) {
        // citire comanda client
        bzero (msgToSend, 2048);
        printf ("::: ");
        fflush (stdout);
        read (0, msgToSend, 2048);
        msgToSend[strlen(msgToSend)-1]='\0';

        // trimitere mesaj catre server
        if (write (sd, msgToSend, 2048) <= 0){
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
        }

        // asteapta pana cand primeste raspuns/mesaj de la server
        bzero(msgRead,2048);
        if (read (sd, msgRead, 2048) < 0){
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }

        // afisare mesaj trimis de catre server
        printf (">> %s\n", msgRead);
        fflush(stdout);
        if (strcmp(msgRead,"Quit!\n") == 0) {
            break;
        }
    }

    close (sd);
}
