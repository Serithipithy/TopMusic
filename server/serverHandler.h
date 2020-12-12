//
// Created by alexandra on 12/10/20.
//

#ifndef PROIECT_2_SERVERHANDLER_H
#define PROIECT_2_SERVERHANDLER_H

void parssingMsg ( char *message );
int getCommand ( const char *message );
void loginCommand ( const char message, char *messageToSend,int& userType );
#endif //PROIECT_2_SERVERHANDLER_H
