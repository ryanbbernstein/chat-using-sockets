#include "InputHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ShowHelp() 
{ 
	printf("\n***CHAT HELP***"
		"\nList of Commands supported:"
		"\n> help"
		"\n> myip"
		"\n> myport"
		"\n> connect <destination> <port> "
		"\n> list"
		"\n> terminate <connection id>"
		"\n> send <connection id> <message>\n");
}


int CmdHandler(char** parsed) 
{ 
	int i, switchArg = 0; 
	char* ListOfCmds[NUM_CMDS]; 
	char* username; 

	ListOfCmds[0] = "exit";
    ListOfCmds[1] = "help";
	ListOfCmds[2] = "myip"; 
	ListOfCmds[3] = "myport"; 
	ListOfCmds[4] = "connect";
    ListOfCmds[5] = "list";
    ListOfCmds[6] = "terminate";
    ListOfCmds[7] = "send";
     

	for (i = 0; i < NUM_CMDS; i++) { 
		if (strcmp(parsed[0], ListOfCmds[i]) == 0) { 
			switchArg = i + 1; 
			break; 
		} 
	}

	return switchArg; 
}
 
void parseSpaces(char* str, char** parsed) 
{ 
	int i; 

	for (i = 0; i < MAXLIST; i++) { 
		parsed[i] = strsep(&str, " "); 

		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	} 
} 

int processString(char* str, char** parsed) 
{
    parseSpaces(str, parsed); 
	return CmdHandler(parsed);
}