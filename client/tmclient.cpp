//
// Created by alexandra on 12/5/20.
//

#include "tmclient.h"
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
#include <json/value.h>
#include <fstream>

int loginCommand(){
    bzero (msg, 100);
    printf ("Username: ");
    fflush (stdout);
    read (0, msg, 100);

    std::ifstream accounts_file("people.json", std::ifstream::binary);
    accounts_file >> people;
}