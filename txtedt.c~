main() 
{
	char filename[6];
	char buffer[13312];
	char fileToSave[13312];
	int i;
	int sectors;
	int numchars;
	int bufferchars;

	numchars = 0;

	print("Enter filename: \0");
	read(filename, 6);
	print("\r\nEditing\r\n\0");

	while (fileToSave[numchars - 3] != 0x04) {
		bufferchars = read(buffer, 13312);
		for (i = 0; i < bufferchars; i++) {
			fileToSave[numchars + i] = buffer[i];
		};
		numchars += bufferchars;
		fileToSave[numchars] = '\r';
		fileToSave[numchars + 1] = '\n';
		print("\r\n\0");
		numchars += 2;

	};

	fileToSave[numchars - 3] = '\0';

	sectors = numchars/512 + 1;

	write(filename, fileToSave, sectors);
		
	terminate();
}
	
