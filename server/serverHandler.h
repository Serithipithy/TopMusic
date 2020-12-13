//
// Created by alexandra on 12/13/20.
//

#ifndef SERVER_SERVERHANDLER_H
#define SERVER_SERVERHANDLER_H

#include <sqlite3.h>

void parssingMsg (char *message );
int getCommand ( const char *message );

// not in main

#endif //SERVER_SERVERHANDLER_H
