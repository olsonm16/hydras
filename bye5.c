main() {
	int i=0;
	int j=0;
	int k=0;
	
	enableInterrupts();
	
	for(i=0; i<5; i++) {
		print("Goodbye\n\r\0");
		for (j=0; j<10000; j++) {
			for (k=0; k<1000; k++) {
			}
		}
	}
	terminate();
};
