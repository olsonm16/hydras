#include "proc.h"

/*
 * This function initializes all of the global variables and 
 * structures defined above.
 * All entries in the memory map should be marked as FREE.
 * 
 * All of the PCBs in the pcbPool should have the first character of their 
 * names set to null, their state set to DEFUNCT and their segment and stack
 * pointers set to 0x00. 
 * 
 * The PCB for the idle process should be initialized with the name IDLE, 
 * the state READY, and both the segment and stack pointers set to 0x00.
 *
 * The PCB for the running process should refer to the PCB for the idle process.
 *
 * The ready queue should be empty.
 */ 
void initializeProcStructures() {
    int i;
  
    for(i=0; i<8; i++) {
        pcbPool[i].name[0] = 0x00;
        pcbPool[i].state = DEFUNCT;
        pcbPool[i].segment = 0x00;
        pcbPool[i].stackPointer = 0x00;
        memoryMap[i] = FREE;
    }

    /*
     * The PCB for the idle process should be initialized with the name IDLE, 
     * the state READY, and both the segment and stack pointers set to 0x00.
     */
    strCpy("IDLE\0", idleProc.name, 5);
    idleProc.state = READY;
    idleProc.segment = 0x00;
    idleProc.stackPointer = 0x00;
    
    /*
     * The PCB for the running process should refer to the PCB for the idle process     
    */  
    running = &idleProc;
     
    // the ready queue should be empty.
    readyHead = NULL;
    readyTail = NULL;
}

/*
 * Returns the index of the first free memory segment or -1 if 
 * all of the memory segments are used.  The returned memory segment
 * should be marked as used.
 */
int getFreeMemorySegment() {
	int i;
	for (i = 0; i < 8; i++) {
		if (memoryMap[i] == FREE) {
			memoryMap[i] = USED;
			return i;
		};
	}
	return -1;
}

/*
 * Release the memory segment indicated by the given index into
 * the memory map.
 */
void releaseMemorySegment(int seg) {
	memoryMap[seg] = FREE;

}

/*
 * Return a pointer to an available PCB from the PCB pool.  All of the
 * available PCBs should be marked as DEFUNCT.  The returned PCB should
 * be marked as STARTING. Return NULL if there are no PCBs available.
 */
struct PCB *getFreePCB() {

	int i;
  
    for(i=0; i<8; i++) {
        pcbPool[i].name[0] = 0x00;
        if (pcbPool[i].state == DEFUNCT) {
        	return &pcbPool[i];
		};
    }
    return NULL;
}

/*
 * Release the provided PCB by setting its state to DEFUNCT, setting
 * its next and prev pointers to NULL and setting the first character
 * of its name to 0x00.
 */
void releasePCB(struct PCB *pcb) {

	pcb -> name[0] = 0x00;
	pcb -> state = DEFUNCT;
	pcb -> next = NULL;
	pcb -> prev = NULL;
}

/*
 * Add the provided PCB to the tail of the ready queue.
 */
void addToReady(struct PCB *pcb) {
	if (readyTail == NULL && readyHead == NULL) {
		readyTail = pcb;
		readyHead = pcb;
	} else if (readyHead != NULL) {
		readyTail = pcb;
	};
		
	
}

/*
 * Remove the PCB at the head of the ready queue and return a 
 * pointer to it.
 */
struct PCB *removeFromReady() {
    return NULL;
}


/*
 * Copy len characters from str1 into str2.
 * Return len
 */
int strCpy(char *str1, char *str2, int len) {
    int i=0;
    for (i=0; i<len; i++) {
            str2[i] = str1[i];
    }
    return len;
}
