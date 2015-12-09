#ifndef USER_LIB
#define USER_LIB

int strCmp(char * a, char * b, int len);
int print(char *str);
int read(char *buf);
int readFile(char *fname, char *buf);
int execute(char *fname);
int delete(char *fname);
int write(char *fname, char *buffer, int sectors);
int dir(char * directory);
void terminate();

#endif
