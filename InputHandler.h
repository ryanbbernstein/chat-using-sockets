
#define MAXLIST 100
#define NUM_CMDS 8

void ShowHelp();
int CmdHandler(char** parsed);
void parseSpaces(char* str, char** parsed);
int processString(char* str, char** parsed);