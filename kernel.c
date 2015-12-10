/*
Mitch Olson
CS 330 OS Project 5
*/

#define MAIN
#include "proc.h"

void putChar(char ch, int color, int row, int column);
void putStr(char * string, int color, int row, int column);
int printString(char * string);
int readChar();
int mod(int a, int b);
int readSector(char *buf, int absSector);
int writeSector(char *buf, int absSector);
int handleInterrupt21(int ax, int bx, int cx, int dx);
int readFile(char *filename, char *buf);
int strCmp(char * a, char * b, int len);
int printInt(int integer);
int executeProgram(char * filename);
void terminate(c);
int deleteFile(char *filename);
int findSectors(int numSectors, int * sectors);
int freeSectors();
int writeFile(char *fname, char *buffer, int sectors); 
int dir(char * directory);
void handleTimerInterrupt(int segment, int stackPointer);
void kStrCopy(char *src, char *dest, int len);
void yield();
void showProcesses();
int kill(int segment);


/*
Main just runs our shell function using an interrupt.
*/
int main() {

	
	makeInterrupt21();
	makeTimerInterrupt();
	initializeProcStructures();
	
	//printString("WELCOME TO THE HYDRAS OS\r\n\0");
	//printString("Type help for guidance.\r\n\0");


	interrupt(0x21, 0x04, "shell\0", 0, 0);
	//interrupt(0x21, 0x04, "txtedt\0", 0, 0);
	//interrupt(0x21, 0x00, "Done!\n\r\0", 0, 0);

	
				
	while(1){};
};

/*
Readfile takes in a filename, and a buffer to store the file contents.
If the filename is found in the disk directory, the sectors where the file
is stored are read into the buffer. The number of sectors read is returned.
If the file name is not found, -1 is returned.
*/

int readFile(char *filename, char *buf) {
	int i;
	int j;
	char disk_directory[512];
	int sector_nums;
	
	j = readSector(disk_directory, 2);
	
	for (i = 0; i < 512; i+=32) {
		if (strCmp(disk_directory + i, filename, 6)) {
			for (sector_nums = 0; sector_nums < 26; sector_nums++) {
				if (disk_directory[sector_nums + i + 6] != 0x00) {
					readSector(buf + sector_nums*512, disk_directory[sector_nums + i + 6]);
				} else {
					return sector_nums;
				}
			}
		}
		
	};
	
	return -1;
		
};

/*
strCmp takes two strings and a length which to compare them up until.
If disagreeing chars are found at corresponding indices, 0 is returned.
If no disagreement => strings match, 1 is returned.
*/
int strCmp(char * a, char * b, int len) {
	int i;
	for (i = 0; i < len; i++) {	
		if (a[i] != b[i]) {
			return 0;
		}	
	}
	return 1;
}

/*
executeProgram takes a file name.
A free segment is retrieved from the proc.c class.
File name is read into a buffer using readfile.
If the file is not found, -2 is returned.
Otherwise, the file is put in memory at the segment location, character by character.
Then, the program is launched using the kernel command, and 0 is returned.
*/

int executeProgram(char * filename) {

	char buffer[13312];
	int sector;
	int i;
	int j;
	int segment;
	int seg;
	struct PCB *process;
	char procname[7];

	//Grab a free segment from memory
	setKernelDataSegment();
	seg = getFreeMemorySegment();
	if (seg < 0) {
		return -2;
	};
	segment = 0x1000*(seg+2);
	restoreDataSegment();
	//Read in the sector number of the program to be run
	sector = readFile(filename, buffer);
	//If the program was not found
	if (sector < 0) { 
		return -1; 
	};
	
	//Put each bit of the program in memory starting at the memory segment returned.
	for (i = 0; i < sector*512; i++) {
		putInMemory(segment, 0x0000 + i, buffer[i]);
	};
	
	//Grab a free process for the program.
	setKernelDataSegment();
	process = getFreePCB();
	restoreDataSegment();
	//If there are no free processes, return -2
	if (process == NULL) {
		return -2;
	};
	//Set the name to the filename
	kStrCopy(filename, process -> name, 6);
	//Set segment to segment
	setKernelDataSegment();
	process -> segment = segment;
	//Stack pointer to start of source
	process -> stackPointer = 0xFF00;
	//Add the process to the ready Queue;
	addToReady(process);
	restoreDataSegment();

	//Now, initialize the program.
	initializeProgram(segment);
	
	return 1;
}

// Prints out a given integer to the screen at current position
// Returns 1
int printInt(int integer) {
        int num = integer;
        if (num > 9)
                printInt(num /= 10);
        interrupt(0x10, 0x0E * 256 + 0x30 + mod(integer, 10), 0, 0, 0);
        return 1;
}

//From previous lab, puts a char to the screen at the specified location. 
void putChar(char ch, int color, int row, int column) {

	int vmem = 0xB000;
	int charbytes = 2;
	int offset = 0x8000;
	offset += 80*(row-1)*charbytes;
	offset += (column-1)*charbytes;
	putInMemory(vmem, offset, ch);
	putInMemory(vmem, offset+1, color);

};

//Mod helper function, returns a MOD b
int mod(int a, int b) {
	int result;
	result = a - b*(a/b);
	return result;
}
//Reads a string and puts it to the window at the specified location with putChar.

void putStr(char * string, int color, int row, int column) {

	while (*string != '\0') {
		putChar(*string, color, row, column);
		string++;
		column++;
	};
}

/*Given an input string, prints string at cursor location with interrupt function.
Returns counter
Handles backspace by decrementing counter and string so that the final string is correct
Does not 'backspace' the echoed string - not sure how to fix what's already been printed.*/
int printString(char * string) {
	int counter;
	counter = 0; 
	while (*string != '\0') {
		char al = *string;
		char ah = 0x0E;
		int ax = ah * 256 + al;
		interrupt(0x10, ax, 0, 0, 0);
		string++;
		counter ++;
	};
	
	return counter;
}

//Uses interrupt to read a char from the keyboard.
int readChar() {
	return interrupt(0x16, 0, 0, 0, 0);
}

/*Uses interrupt and a check for ENTER key code, as well as a counter, to allow the
user to type a string in and save the final string into the argument char buffer.*/
// Reads user input until ENTER is pressed
// Takes a char* to store input in
// BONUS: Now takes a max number of characters
// NOTE: If you put max at 10, you will have 11 characters
// because '\0' is added at the end of the string.
// This could easily be changed to only allow 9 chars by
// uncommenting the line below decrementing max.
// Returns length of string
// Ryan Grant helped me with the backspace feature, so credit is much due to him for that feature. 
int readString(char *buf, int max) {
        char ch;
        int i = 0;
        char ah = 0x0E;
        int ax;
        int space = ah * 256 + 0x00;
        //max--;
       
        ch = readChar();
 
        // Keeps getting input until 0x0D (enter) is pressed
        while (ch != 0x0D) {
                // Handles backspace
                if (ch == 0x08) {
                        // Only lets you backspace to beginning
                        if (i > 0) {
                                ax = ah * 256 + ch;
                                // Goes back one position
                                // Writes a space
                                // Then goes back again
                                interrupt(0x10, ax, 0, 0, 0);
                                interrupt(0x10, space, 0, 0, 0);
                                interrupt(0x10, ax, 0, 0, 0);
                                i--;
                        }
                }
                else {
                        if (i < max) {
                                ax = ah * 256 + ch;
                                interrupt(0x10, ax, 0, 0, 0);
                                *(buf + i) = ch;
                                i++;
                        }
                }
                // Gets next input character
                ch = readChar();
        }
        // Adds null character to end of string
        *(buf + i) = '\0';
        return i;
}

/* Reads and prints a sector from memory */
int readSector(char *buf, int absSector) {
	int relSector = mod(absSector, 18) + 1;
	int head = mod(absSector/18, 2);
	int track = absSector/36;
	
	int ax = 0x02 * 256 + 0x01;
	int bx = buf;
	int cx = track*256 + relSector;
	int dx = head*256 + 0x00;
	
	interrupt(0x13, ax, bx, cx, dx);
	
	return 1;

};

/* Writes a character array to a sector in memory */
int writeSector(char *buf, int absSector) {
	int relSector = mod(absSector, 18) + 1;
	int head = mod(absSector/18, 2);
	int track = absSector/36;
	
	int ax = 0x03 * 256 + 0x01;
	int bx = buf;
	int cx = track*256 + relSector;
	int dx = head*256 + 0x00;
	
	interrupt(0x13, ax, bx, cx, dx);
	
	return 1;

};

/* Handles many interrupts...
if ax == 0x00, prints the string stored in bx. Returns the counter from printString.
if ax == 0x11, reads a char in, stores it in bx. Returns 1.
if ax == 0x01, reads a string in, stores it in bx. Returns the counter from readString.
if ax == 0x03, file is read with filename bx, stored in cx.
if ax == 0x04, program is executed with filename bx, memory location segment cx.
if ax == 0x05, the terminate function is executed.
Else, -1 is returned.
*/
int handleInterrupt21(int ax, int bx, int cx, int dx) {
	char readch;
	int count;
	if (ax == 0x00) {
		return printString(bx);
	};
	
	if (ax == 0x11) {
		char * buf = bx;
		printString("Type a char: \0");
		readch = readChar();
		buf[0] = readch;
		return 1;
	}
	if (ax == 0x01) {
		char * buf = bx;
		printString("Type a string: \0");
		count = readString(buf, 80);
		return count;
	};
	
	if (ax == 0x03) {
		return readFile(bx, cx);
	};
	
	if (ax == 0x04) {
		return executeProgram(bx);
	};
	
	if (ax == 0x05) {
		terminate();
		return 0;
	};

	if (ax == 0x07) {
		return deleteFile(bx);
	};

	if (ax == 0x08) {
		return writeFile(bx, cx, dx);
	}
	
	if (ax == 0x09) {
		return dir(bx);
	};
	
	if (ax == 0x0A) {
		return showProcesses();
	};

	if (ax == 0x0B) {
		return kill(bx);
	};

	if (ax == 0x0C) {
		return yield();
	};
	
	return -1;
};

int deleteFile(char *filename) {
	int i;
	int j;
	char disk_directory[512];
	char disk_map[512];

	int sector_nums;

	int sectors[26];
	int k;
	int h;

	k = 0;
	
	j = readSector(disk_map, 1);	
	j = readSector(disk_directory, 2);
	
	for (i = 0; i < 512; i+=32) {
		if (strCmp(disk_directory + i, filename, 6)) {
			//Set the first character of the filename in the directory to 0x00
			disk_directory[i] = 0;
			for (sector_nums = 0; sector_nums < 26; sector_nums++) {
				if (disk_directory[sector_nums + i + 6] != 0x00) {
					sectors[k] = disk_directory[sector_nums + i + 6];
					k++;
				}
			}
		}
		
	};

	if (k > 0) {
		//If the filename was found, and sectors were saved, blank out sectors in disk map.
		for (h = 0; h < k; h++) {
			disk_map[sectors[h]] = 0;
		};
		//And write the modified disk map and disk directory to sectors 1 and 2 to save the change.
		writeSector(disk_map, 1);
		writeSector(disk_directory, 2);
		
		return 1;
	
	}
	//If k was 0, we never found the filename and never incremented k in grabbing the sector nums.
	return -1;
		
};

/*

Writing a file requires finding an empty entry in the Disk Directory and finding a free sector in the Disk Map for each sector making up the file. The data for each sector is written into a free sector on the disk and the sector numbers that are used are entered into the Disk Directory entry for the file. The modified Disk Directory and Disk Map must then be written back to the disk to save the changes.
A d d a  w r i t e F i l e  f u n c t i o n t o y o u r k e r n e l w i t h t h e s i g n a t u r e :
int writeFile(char *fname, char *buffer, int sectors);
T h i s f u n c t i o n s h o u l d w r i t e  s e c t o r s  * 5 1 2 b y t e s o f d a t a f r o m t h e b u f f e r i n t o a f i l e w i t h t h e n a m e indicated by  fname. 
● I f t h e f i l e i n d i c a t e d b y  f n a m e  a l r e a d y e x i s t s , t h e n e w f i l e o v e r w r i t e s i t .
● The maximum number of sectors that may be written is 26. If  sectors is larger than 26,
then only the first 26 sectors should be written.
● If the file is successfully written, this function returns the number of sectors that were
written.
● If there is no Disk Directory entry available for the new file, this function should return -1
and the file is not written.
● I f t h e D i s k M a p c o n t a i n s f e w e r t h a n  s e c t o r s  f r e e s e c t o r s , t h i s f u n c t i o n s h o u l d w r i t e a s
many sectors as possible and return -2.

*/
	
int writeFile(char *fname, char *buffer, int sectors) {
	int i;
	int s;
	int j;
	int h;
	int y;
	int z;
	int sector_count;
	int found;
	int namesave;
	int notallwritten;
	char disk_directory[512];
	char disk_map[512];
	char buffer_part[512];
	//Sectors arrays of size 512
	char buffer_parts[26][512];
	int writeSectors[26];

	int b;

	b = 0;
	sector_count = 0;

	j = readSector(disk_directory, 2);
	j = readSector(disk_map, 1);

	//First, we need to split up the buffer into sectors arrays of size 512.
	//So the 0th iteration should copy 0 to 511 to a buffer part.
	//The 1st iteration should copy 512(1) to 1023 = (512)(2) - 1 to a buffer part...
	//The nth interation should copy 512*(n) to 512(n+1) - 1 to a buffer part.
	//So we need n buffer parts of size 512. We'll use the char buffer_parts[sectors][512] initialized above.
	for (y = 0; y < sectors; y++) {	
		for (h = 512*y; h < 512*(y+1); h++) {
			buffer_parts[y][h] = buffer[h];
		};
	};

	//Now that our buffer is broken up into arrays of size 512, we need to find sectors to write the data to.
	//Two cases remain, either the filename already exists, or it's a new file.

	for (i = 0; i < 512; i+=32) {
		//If the file is found, we want to overwrite the sectors related to that file with the new data, and additional sectors if necessary.
		if (strCmp(disk_directory + i, fname, 6)) {
			printString("found match\0");
			//Again, we want to write n sectors to memory. Let's go ahead and write to the ones already taken up, and see if we need to do any more.
			//for (s = 0; s < sectors; s++) {
			s = 0;
			while (s < sectors) {
				//We found a sector to overwrite, hooray!
				if (disk_directory[s + i + 6] != 0x00) {
					writeSector(buffer_parts[s], disk_directory[s + i + 6]);
					s++;
				} else {
				//Otherwise, we ran out of already-written sectors, and need to find sectors-s free ones.
				//Like, imagine the file took up 3 sectors originally. But we called for 5. So we wrote over the 3 already taken, and we need 2 more.
				//s would hit the 0th, 1st, and 2nd sector (3), then be at 3rd sector and need. 5 - 3 = 2 more. So, yes, sectors - s makes sense.
				//however, I don't think I can just initialize an int array of size sectors - s right here. Hmm... I'll just use writeSectors.
					found = findSectors(sectors - s, writeSectors);
					//We found enough sectors
					for (z = s; z < sectors; z++) {
						if (writeSectors[z-s] != 0) {
							sector_count++;
							disk_directory[z + i + 6] = writeSectors[z-s];
							//disk_map[writeSectors[z-s]] = 0xFF;
							writeSector(buffer_parts[z], writeSectors[z-s]);
					};
				}
					s = sectors;
				}
			};
			
			writeSector(disk_directory, 2);
			//writeSector(disk_map, 1);

			if (found) {
				return sector_count;
			} else {
				return -2;
			};
		};
	};

	printString("pls");

	//That was pretty ugly...! Ok. If nothing was returned then the filename wasn't found, so we need to save it.
				
	for (i = 0; i < 512; i += 32) {
		if (disk_directory[i] == 0) {
			for (namesave = 0; namesave < 6; namesave++) {
				disk_directory[i + namesave] = fname[namesave];
			};
			found = findSectors(sectors, writeSectors);
			for (z = 0; z < sectors; z++) {
				if (writeSectors[z] != 0) {
					sector_count++;
					disk_directory[z + i + 6] = writeSectors[z];
					//disk_map[writeSectors[z]] = 0xFF;	
					writeSector(buffer_parts[z], writeSectors[z]);
				};
			};
			
			writeSector(disk_directory, 2);
			//writeSector(disk_map, 1);

			if (found) {
				return sector_count;
			} else {
				return -2;
			};
		};
	};

	return -1;

}

	
	

int findSectors(int numSectors, int * sectors) {
	char disk_map[512];
	int j;
	int i;
	int k;
	int h;

	j = readSector(disk_map, 1);

	i = 0;

	while (i < 256) {
		if (numSectors != 0) {
			if (disk_map[i] != 0xFF) {
				//printInt(disk_map[i]);
				//printString(" \0");
				//printInt(i);
				sectors[h] = i;
				h++;
				numSectors--;
				disk_map[i] = 0xFF;
			};
			i++;
		} else {
			break;
		};
	};

	writeSector(disk_map, 1);
	
	if (numSectors == 0) {
		return 1;
	} else {
		return -2;
	};

}

int freeSectors() {
	char disk_map[512];
	int j;
	int i;
	int sectors;

	j = readSector(disk_map, 1);
	sectors = 0;
	i = 0;

	while (i < 256) {
		if (disk_map[i] != 0xFF) {
			sectors++;
		}
		i++;
	};

	return sectors;

};
	
		
	


int dir(char * directory) {
	int i;
	int j;
	int k;
	int count;
	char disk_directory[512];
	
	j = readSector(disk_directory, 2);
	count = 0;

	directory[count] = '\r';
	count++;
	directory[count] = '\n';
	count++;

	for (i = 0; i < 512; i += 32) {
		if (disk_directory[i] != '\0') {
			for (k = 0; k < 6; k++) {
				if (disk_directory[i + k] != '\0') {
					directory[count] = disk_directory[i + k];
					count++;
				} else {
					directory[count] = ' ';
					count++;
				};
			};
			directory[count] = '\r';
			count++;
			directory[count] = '\n';
			count++;

		};
	};

	printString(directory);

	return count;
		

}

void handleTimerInterrupt(int segment, int stackPointer) {

	struct PCB * process;
	
	setKernelDataSegment();
	
	running -> segment = segment;
	running -> stackPointer = stackPointer;
	running -> state = READY;
	addToReady(running);
	
	process = removeFromReady();
	if (process == NULL) {
		running = &idleProc;
	} else {
		process -> state = RUNNING;
		running = process;
	};
	returnFromTimer(running -> segment, running -> stackPointer);
	restoreDataSegment();
	

}

void terminate() {

	setKernelDataSegment();
	releaseMemorySegment((running -> segment)/0x1000 - 2);
	restoreDataSegment();

	setKernelDataSegment();
	releasePCB(running);
	restoreDataSegment();

	setKernelDataSegment();
	running -> state = DEFUNCT;
	restoreDataSegment();
			
	while(1) {};

};

void kStrCopy(char *src, char *dest, int len) {
	int i=0;
	for (i=0; i<len; i++) {
		putInMemory(0x1000, dest+i, src[i]);
		if (src[i] == 0x00) {
			return;
		}
	}
}

void yield() {

	setKernelDataSegment();
	addToReady(running);
	restoreDataSegment();
	
};

void showProcesses() {
	
	int i;
	
	setKernelDataSegment();

	printString("\r\n\0");

	for (i=0; i<8; i++) {
		if (pcbPool[i].name[0] != 0x00) {
			printString(pcbPool[i].name);
			printString("\t");
			printInt((pcbPool[i].segment)/(0x1000) - 2);
			printString("\r\n\0");
		
		};
	};
	
	restoreDataSegment();
	
};

int kill(int segment) {

	int segAddr;
	int i;

	segAddr = (segment + 2)*0x1000;

	setKernelDataSegment();

	for (i=0; i<8; i++) {
		if (pcbPool[i].segment == segAddr) {
			releaseMemorySegment(segment);
			releasePCB(&pcbPool[i]);
			pcbPool[i].state = DEFUNCT;
			restoreDataSegment();
			return 1;
		};
	};

	restoreDataSegment();	
	
	return -1;

}
	
	






