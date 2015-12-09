main() {
	enableInterrupts();
	while (1) {
		char buf[512];
		char file_contents[13000];
		char file_name[76];
		char directory[512];
		char source[76];
		char destination[76];
		int result;
		int j;
		int s;
		int d;
		int z;
		int sectors;
		int countsource;
		double sectdiv;
		int blank;

		for (blank = 0; blank < 76; blank ++) {
			file_name[blank] = '\0';
			source[blank] = '\0';
			destination[blank] = '\0';
		};

		for (blank = 0; blank < 512; blank ++ ) {
			buf[blank] = '\0';
			directory[blank] = '\0';
		};
		
		print("Hydras> ");
		read(buf);
		if (strCmp(buf, "type\0", 4)) {
			result = readFile(buf+5, file_contents);
			if (result > 0) {
				print("\n\r\0");
				print(file_contents);
				print("\n\r\0");
			} else {
				print("\n\r\0");
				print("\n\rFile not found\0\r\n");
			}

		} else if (strCmp(buf, "execute\0", 7)) {
			print("\n\r\0");
			result = execute(buf + 8);
			if (result < 0) {
				print("\n\r\0");
				print("Bad file name\r\n");
			}
		} else if (strCmp(buf, "dir\0", 3)) {
			result = dir(directory);
			//print("\r\n\0");
			//print(directory);
			//print("\r\n\0");

		} else if (strCmp(buf, "delete\0", 6)) {
			result = delete(buf + 7);
			if (result < 1) {
				print(buf + 7);
				print("File not found\0");
			} else {
				print("\r\n\0");
				print(buf + 7);
				print(" was deleted.\r\n\0");
			};
		} else if (strCmp(buf, "copy\0", 4)) {
			d = 0;
			s = 0;
			for (j = 5; j < 512; j ++) {
				if (buf[j] != ' ') {
					source[s] = buf[j];
					s++;
				} else {
					break;
				};
			};

			j++;

			while (buf[j] != '\0') {
				destination[d] = buf[j];
				j++;
				d++;
			};

			result = readFile(source, file_contents);

			if (result < 1) {
				print("File not found\r\n\0");
			} else {
				
				result = write(destination, file_contents, result);
				
				if (result == -1) {
					print("Disk directory is full.\0");
				} else if (result == -2) {
					print("Disk is full.\0");
				} else {
					print("\r\nFile copied\r\n\0");
				}
			}
		} else if (strCmp(buf, "help\0", 4)) {
			print("\r\nHydras is a basic OS that can do a few things for you. \r\n\0");
			print("The current implementation (Version 0.4) can work with files. \r\n\0");
			print("To see the files you have on the disk, type dir. \r\n\0");
			print("To view the contents of a file, enter type <filename> (no braces). \r\n\0");
			print("To execute a program, type execute <progname>. \r\n\0");
			print("One such program is txtedt. You may create and save a txt file using txtedt. \r\n\0");
			print("Enter copy <source filename> <destination filename> to copy a file. \r\n\0");
			print("Enter delete <filename> to delete a file. \r\n\0");
			print("In version 0.5, we will implement processes explicitly. \r\n\0");
		
		} else {
			print("\n\r\0");
			print("Unrecognized command.\r\n\0");
		
		}
	
	}
}
