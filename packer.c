/*
 * Timothy Merfry Tiwow. March 16, 2021
 *
 * rev March 17, 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

void packIt(char *filename) {
	char **fileBytes;
	char **fileNames;
	int numOfFiles;
	unsigned long totalBytesToWrite;

	printf("How many files to pack? ");
	scanf("%d", &numOfFiles);

	fileBytes = (char**) calloc(sizeof(char*), numOfFiles);
	fileNames = (char**) calloc(sizeof(char*), numOfFiles);

	char temp;			//catch the trailing- 
	scanf("%c", &temp); //newline from scanf

	int i;
	totalBytesToWrite = 0;
	for(i=0; i<numOfFiles; i++) {
		char pathname[128];
		printf("Input filename %d: ", i+1);
		fgets(pathname, 128, stdin);
		pathname[strcspn(pathname, "\n")] = 0; //remove trailing newline from fgets
		int pathnameLen = strnlen(pathname, 128);

		fileNames[i] = (char*) calloc(sizeof(char), pathnameLen + 4);	//+4 to store integer containing
																		//length of pathname
		memcpy(&fileNames[i][0], &pathnameLen, sizeof(int));			//store the strlen of pathname
		strncpy(&fileNames[i][4], pathname, sizeof(char) * pathnameLen);//store the pathname

		struct stat st;
		stat(pathname, &st); //acquire filesize
		if(errno == ENOENT) {
			printf("ERROR: File doesn't exist\n");
			return;
		}

		if(S_ISDIR(st.st_mode)) {
			printf("ERROR: You specified a directory\n");
			return;
		}

		int fileByteSize = st.st_size;

		printf("file byte size: %d\n", fileByteSize);
		totalBytesToWrite += fileByteSize;

		fileBytes[i] = (char*) calloc(sizeof(char), st.st_size + 4);
		memcpy(&fileBytes[i][0], &fileByteSize, sizeof(int));

		FILE *fptr;
		fptr = fopen(pathname, "rb");
		fread(&fileBytes[i][4], fileByteSize, 1, fptr);
		fclose(fptr);
	}

	int fileNamesLen = 0;
	int fileBytesLen = 0;
	for(i=0; i<numOfFiles; i++) {
		int len = *((int*)(&fileNames[i][0]));	//cast char pointer to integer pointer, then take the value.
		fileNamesLen += len + 4;				//+4 to compensate storing integer on beginning of each-
												//char array
		len = *((int*)(&fileBytes[i][0]));		//same as above
		fileBytesLen += len + 4;				//same as above
	}

	printf("total bytes to pack : %d\n", 4 + fileNamesLen + 4 + fileBytesLen);

	FILE * fptr;
	fptr = fopen(filename, "wb");
	fwrite(&fileNamesLen, sizeof(int), 1, fptr);
	for(i=0; i<numOfFiles; i++)
		fwrite(&fileNames[i][0], *((int*)(&fileNames[i][0])) + 4, 1, fptr);
	fwrite(&fileBytesLen, sizeof(int), 1, fptr);
	for(i=0; i<numOfFiles; i++)
		fwrite(&fileBytes[i][0], *((int*)(&fileBytes[i][0])) + 4, 1, fptr);
	fclose(fptr);
	
	//free used memory
	for(i=0; i<numOfFiles; i++)
		free(fileNames[i]);
	free(fileNames);

	for(i=0; i<numOfFiles; i++)
		free(fileBytes[i]);
	free(fileBytes);

	printf("packed to %s\n", filename);
}

void listIt(char *filename) {
	FILE * fptr;
	fptr = fopen(filename, "rb");
	int fileNamesLen;
	fread(&fileNamesLen, sizeof(int), 1, fptr);
	while(fileNamesLen > 0) {
		int len;
		char *fileName;

		fread(&len, sizeof(int), 1, fptr);
		fileName = (char*) calloc(sizeof(char), len);
		fileNamesLen -= sizeof(int);
		
		fread(fileName, sizeof(char), len, fptr);
		fileNamesLen -= len;
		printf("%s\n", fileName);

		free(fileName);
	}
	fclose(fptr);
	printf("Listed all files packed in %s\n", filename);
}

void unpackIt(char *filename) {
	FILE * fptr;
	fptr = fopen(filename, "rb");

	int fileNamesLen;
	fread(&fileNamesLen, sizeof(int), 1, fptr);
	char **fileNames = NULL;
	int i = 0;
	while(fileNamesLen > 0) {
		fileNames = (char**) realloc(fileNames, sizeof(char*) * (i + 1));
		i++;
		int len;
		fread(&len, sizeof(int), 1, fptr);
		fileNamesLen -= sizeof(int);

		fileNames[i - 1] = (char*) calloc(sizeof(char), len);
		fread(&fileNames[i - 1][0], sizeof(char), len, fptr);
		fileNamesLen -= len;
	}

	printf("Detected %d packed files\n", i);

	int fileBytesLen;
	fread(&fileBytesLen, sizeof(int), 1, fptr);
	int j;
	for(j=0; j<i; j++) {
		int len;
		char *fbytes;

		fread(&len, sizeof(int), 1, fptr);

		fbytes = (char*) calloc(sizeof(char), len);
		fread(fbytes, sizeof(char), len, fptr);
		
		FILE * saveFptr;
		saveFptr = fopen(fileNames[j], "wb");
		fwrite(fbytes, sizeof(char), len, saveFptr);
		fclose(saveFptr);

		printf("Unpacked %s to current directory\n", fileNames[j]);
		free(fbytes);
	}
	fclose(fptr);

	for(j=0; j<i; j++)
		free(fileNames[j]);
	free(fileNames);

	printf("Unpacked %s\n", filename);
}

int main(int argc, char **argv) {
	if(argc < 3) {
		printf("packer pack|list|unpack filename\n\nExample: packer pack fileKemasan.bin\n         packer list fileKemasan.bin\n         packer unpack fileKemasan.bin\n");
		return 0;
	}

	if(strncmp(argv[1], "pack", sizeof(char) * 4) == 0) {
		packIt(argv[2]);
	} else if (strncmp(argv[1], "list", sizeof(char) * 4) == 0) {
		listIt(argv[2]);
	} else if (strncmp(argv[1], "unpack", sizeof(char) * 6) == 0) {
		unpackIt(argv[2]);
	} else {
		printf("packer pack|list|unpack filename\n\nExample: packer pack fileKemasan.bin\n         packer list fileKemasan.bin\n         packer unpack fileKemasan.bin\n");
	}

	return 0;
}
