int strCmp(char * a, char * b, int len);
int print(char *str);
int read(char *buf);
int readFile(char *fname, char *buf);
int execute(char *fname);
int delete(char *fname);
int write(char *fname, char *buffer, int sectors);
int dir(char * directory);
void terminate();
void yield();
void showProcesses();
int kill(int pid);

int strCmp(char * a, char * b, int len) {
	int i;
	for (i = 0; i < len; i++) {	
		if (a[i] != b[i]) {
			return 0;
		}	
	}
	return 1;
}

int print(char *str) {
	return interrupt(0x21, 0x00, str, 0, 0);
};

int read(char *buf, int max) {
	return interrupt(0x21, 0x01, buf, max, 0);
};

int readFile(char *fname, char *buf) {
	return interrupt(0x21, 0x03, fname, buf, 0);
};

int execute(char *fname) {
	return interrupt(0x21, 0x04, fname, 0, 0);
};

int delete(char *fname) {
	return interrupt(0x21, 0x07, fname, 0, 0);
};

int write(char *fname, char *buffer, int sectors) {
	return interrupt(0x21, 0x08, fname, buffer, sectors);
}

int dir(char * directory) {
	return interrupt(0x21, 0x09, directory, 0, 0);
};

void terminate() {
	return interrupt(0x21, 0x05, 0, 0, 0);
};

void showProcesses() {
	return interrupt(0x21, 0x0A, 0, 0, 0);
};

void yield() {
	return interrupt(0x21, 0x0C, 0, 0, 0);
};

int kill(int pid) {
	return interrupt(0x21, 0x0B, pid, 0, 0);
};




