/*********************************
* Class: MAGSHIMIM C2			 *
* Week:                			 *
* Name:                          *
* Credits:                       *
**********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dirent.h"

#define FOLDER 1
#define VIRUS_SIGNATURE 2
#define MAX_STR_SIZE 1024
#define SEPERATOR "/"
#define REGULAR_READ 1
#define FIRST_TWENTY 2
#define LAST_TWENTY 3
#define CLEAN 0
#define SIZE_STRING 20

typedef struct fileDetails{
	char fileName[MAX_STR_SIZE];
	int status;
} fileDetails;

int chekIfInfected(FILE* fileToCheck, FILE* signatureFile, int startByte, int endByte);
void changeBackSlashToSlash(char* path);
char* createFilePath(char* nameFile, char* dirPath);
void sortFilesAlphabetically(fileDetails* files, int numFiles);
void printFileInFormat(fileDetails* files, int numFiles, char* pathDir);
void createLogFile(fileDetails* files, int numFiles, char* pathDir, char* virusSignature, char* scanningOption);



int main(int argc, char** argv)
{
	struct dirent* dir = 0;
	DIR* dirFilesToCheck = 0;
	FILE* currentFile = NULL;
	FILE* virusFileSignature = fopen(argv[VIRUS_SIGNATURE], "rb");
	fileDetails* files = NULL;
	char* filePath = NULL;
	int infectedOrNot = 0, sizeOfCurrentFile = 0 , numFiles = 1 , fileNum = 0 , twenty = 0, lastTwenty = 0;
	char choice = ' ';
	char choiceString[SIZE_STRING] = { 0 };

	changeBackSlashToSlash(argv[FOLDER]);
	changeBackSlashToSlash(argv[VIRUS_SIGNATURE]);
	printf("%s\n", argv[FOLDER]);
	dirFilesToCheck = opendir(argv[FOLDER]);
	if (dirFilesToCheck ==NULL)
	{
		printf("Erorr opening directory\n");
		return 1;
	}

	printf("Welcome to my virus Scan!\n\n");
	printf("Folder to scan: %s\n", argv[FOLDER]);
	printf("Virus signature: %s\n\n",argv[VIRUS_SIGNATURE]);
	printf("Press 0 for normal scan or ant other key for a quick scan: ");
	scanf("%c", &choice);
	getchar();
	printf("Scanning began...\n");
	printf("This process may take several minutes...\n\n");

	files = (fileDetails*)malloc(sizeof(fileDetails));// malloctaing memory for array of structurs.

	while((dir = readdir(dirFilesToCheck)) != NULL)// scrolling through the files in the directory.
	{
		if(strcmp(dir->d_name,".") && strcmp(dir->d_name, ".."))// not inckuding the .. and . directorys because not importent.
		{
			filePath = createFilePath(dir->d_name, argv[FOLDER]);// turning the file name to the right format.
			currentFile = fopen(filePath, "rb");
			if(currentFile == NULL)
			{
				printf("File didn't work or path not right\n");
				return 1;
			}
			if(choice == '0')// if normal scan was chosen.
			{
				strcpy(choiceString, "Normal scan");
				sizeOfCurrentFile = calculateSizeOfFile(currentFile);
				infectedOrNot = chekIfInfected(currentFile, virusFileSignature, 0, sizeOfCurrentFile);// checking if the file is infected.
				files = (fileDetails*)realloc(files, sizeof(fileDetails) * numFiles);// realocating memory to add a structure.
				if(files == NULL)
				{
					printf("realloc doesn't worked \n");
					return 1;
				}	
				strcpy(files[fileNum].fileName, dir->d_name);// adding the file name to the struct.
				files[fileNum].status = infectedOrNot;
				numFiles++;// adding one more file to the amount of files
				fileNum++;// adding one to my files index.
			}
			else
			{
				strcpy(choiceString, "Quick scan");// changing the string with the option i chose to my option.
				files = (fileDetails*)realloc(files, sizeof(fileDetails) * numFiles);// realocating memory to add a structure.
				if (files == NULL)
				{
					printf("realloc doesn't worked \n");
					return 1;
				}
				strcpy(files[fileNum].fileName, dir->d_name);// adding the file name to the struct.
				sizeOfCurrentFile = calculateSizeOfFile(currentFile);
				twenty = sizeOfCurrentFile / 5;//getting the size of 20% from the file
				infectedOrNot = chekIfInfected(currentFile, virusFileSignature, 0, twenty);// checking if the file is infected in is first 20%.
				if(infectedOrNot != CLEAN)
				{
					files[fileNum].status = FIRST_TWENTY;
				}
				else// if not infected in first twenty.
				{
					lastTwenty = sizeOfCurrentFile - twenty;// the first byte of the last 20% of the file.
					infectedOrNot = chekIfInfected(currentFile, virusFileSignature, lastTwenty, sizeOfCurrentFile);// checking if the last 20% of the file is infected.				
					if (infectedOrNot != CLEAN)// if infected.
					{
						files[fileNum].status = LAST_TWENTY;
					}
					else// checking if the file is infected at all.
					{
						infectedOrNot = chekIfInfected(currentFile, virusFileSignature, 0, sizeOfCurrentFile);// checking if the file is infected.
						if (infectedOrNot != CLEAN)// if the file is infected the status will update.
						{
							files[fileNum].status = REGULAR_READ;
						}
						else// if not the file is clean.
						{
							files[fileNum].status = CLEAN;
						}
					}
				}
				numFiles++;// adding one more file to the amount of files
				fileNum++;// adding one to my files index.
			}
			fclose(currentFile);
			free(filePath);
		}
	}

	printf("Scanning:\n");
	sortFilesAlphabetically(files, numFiles - 1); // im lowering the numfiles by one because in the last iteration we are adding him one which is his wrong contect because there are less files.
	printFileInFormat(files, numFiles - 1, argv[FOLDER]);
	printf("scan completed.\n");
	createLogFile(files, numFiles - 1, argv[FOLDER], argv[VIRUS_SIGNATURE], choiceString);

	free(files);
	fclose(virusFileSignature);
	getchar();
	return 0;
}


/*
This function checks if a given signatureVirus contect is inside a file , if yes it returns 1 as positive or 0 as negetive.
input:
	fileToCheck - the file im checking for virus.
	signatureFile - the virus im checking for.
	startByte - the starting byte to read from.
	endByte - the byte number that i stop reading when i reach him.
output:
	1 if infrcted 0 if not.
*/
int chekIfInfected(FILE* fileToCheck, FILE* signatureFile, int startByte, int endByte)
{
	int sizeOfVirusFile = calculateSizeOfFile(signatureFile);
	char* dataOfVirusSignature = (char*)malloc(sizeOfVirusFile * sizeof(char));// allocating memory to hold the signature as a string.
	if (dataOfVirusSignature == NULL)
	{
		printf("malloc failed!\n");
		return 1;
	}
	fread(dataOfVirusSignature, sizeof(char), sizeOfVirusFile, signatureFile);// reading the whole signatue.
	for (int i = startByte; i + sizeOfVirusFile < endByte; i++)// going through the file im checking.
	{
		fseek(fileToCheck, i, SEEK_SET);// starting the file from the right byte.
		char* possibleVirus = (char*)malloc(sizeOfVirusFile * sizeof(char));// allocating enough memory to store the current set of bytes im going to read.
		if(possibleVirus == NULL)
		{
			printf("malloc failed!\n");
			return 1;
		}
		fread(possibleVirus, sizeof(char), sizeOfVirusFile, fileToCheck);// reading bytes with the size of the file.
		if(strcmp(possibleVirus, dataOfVirusSignature) == 0)
		{
			free(dataOfVirusSignature);
			free(possibleVirus);
			return 1;
		}
		else
		{
			free(possibleVirus);
		}
	}
	free(dataOfVirusSignature);
	return 0;
}


/*
This function changes evert \ to / because of the check system of magshimim that doesn't allow \.
input:
		path - path of a file or dir.
output:
	none.
*/
void changeBackSlashToSlash(char* path)
{
	for (int i = 0; i < strlen(path); i++)
	{
		if(path[i] == '\\')
		{
			path[i] = '/';
		}
	}
}


/*
This function taked the path of the file and it's name and connect's them together with /.
input:
	nameFile - string with the file name.
	dirPath - string with the directory full path.
output: 
	path - the connected path.
*/
char* createFilePath(char* nameFile, char* dirPath)
{
	char* path = (char*)malloc(strlen(nameFile) * sizeof(char) + strlen(dirPath) * sizeof(char) + 3); // allocating enough space for the full path.
	if(path == NULL)
	{
		printf("erorr mallocating memoy\n");
		return 1;
	}
	strncpy(path, dirPath, strlen(dirPath) + 1);
	strncat(path, SEPERATOR, 2);// putting the path together with /.
	strncat(path, nameFile, strlen(nameFile));
	return path;

}


/*
This function calculates the size of the given file.
input:
	fileToCalc - file to calculate the size of.
output:
	size - the calculated size of file.
*/
int calculateSizeOfFile(FILE* fileToCalc)
{
	int size = 0;
	fseek(fileToCalc, 0, SEEK_END);
	size = ftell(fileToCalc);
	fseek(fileToCalc, 0, SEEK_SET);
	return size;
}


/*
This function sort the files by they're name in an alphbetical order.
input:
	files - a array of structs , which holds the details of each file in the dir.
	numFiles - the amount of files in the directory.
output:
	none.
*/
void sortFilesAlphabetically(fileDetails* files, int numFiles)
{
	for (int i = 0; i < numFiles - 1; i++)
	{
		for (int j = i + 1; j < numFiles; j++)
		{
			if (strcmp(files[i].fileName, files[j].fileName) > 0)
			{
				fileDetails temp = files[i];
				files[i] = files[j];
				files[j] = temp;
			}
		}
	}
}


/*
This function prints the files in they're correct file , if infected , it will be printed i , if not clean will be printed , and if the virus is in the last 20% or first 20% it will also be printed.
input:
	file - a array of structs , which holds the details of each file in the dir.
	numFiles - the amount of files in the directory.
	pathDir - the path of the directory.
output:
	none.
*/
void printFileInFormat(fileDetails* files, int numFiles, char* pathDir)
{
	char* path = NULL;
	for (int i = 0; i < numFiles; i++)
	{
		path = createFilePath((files[i].fileName), pathDir);
		if(files[i].status == CLEAN)
		{
			printf("%s - clean\n", path);
		}
		else if(files[i].status == REGULAR_READ)
		{
			printf("%s - infected!\n", path);
		}
		else if(files[i].status == FIRST_TWENTY)
		{
			printf("%s - infected! (first 20%%)\n", path);
		}
		else if(files[i].status == LAST_TWENTY)
		{
			printf("%s - infected! (last 20%%)\n", path);
		}
		free(path);
	}
}


/*
This function craetes a log file and ebtering it the details of the file check in a nuce looking format.
input:
	files - array of structs with the details of the files.
	numFiles - the number of files in the array of structures.
	pathDir - the path of the direcotry of the files.
	virusSignature - the path of the virus signatue.
	scanningOption - either normal scan or quick scan.
output:
	none.
*/
void createLogFile(fileDetails* files, int numFiles, char* pathDir, char* virusSignature, char* scanningOption)
{
	char nameOfLog[] = "AntiVirusLog.txt";
	char* pathLog = createFilePath(nameOfLog, pathDir);
	FILE* logFile = fopen(pathLog, "w");
	char* path = NULL;
	if (logFile == NULL)
	{
		printf("problam opening file!");
		return 1;
	}
	fprintf(logFile, "Anti-virus began! Welcome!\n\nFolder to scan: \n%s\nVirus signature: \n%s\n\nScanning option: %s\n\nresults: \n", pathDir, virusSignature, scanningOption);
	for (int i = 0; i < numFiles; i++)
	{
		path = createFilePath((files[i].fileName), pathDir);
		if (files[i].status == CLEAN)
		{
			fprintf(logFile, "%s - clean\n", path);
		}
		else if (files[i].status == REGULAR_READ)
		{
			fprintf(logFile, "%s - infected!\n", path);
		}
		else if (files[i].status == FIRST_TWENTY)
		{
			fprintf(logFile, "%s - infected! (first 20%%)\n", path);
		}
		else if (files[i].status == LAST_TWENTY)
		{
			fprintf(logFile, "%s - infected! (last 20%%)\n", path);
		}
		free(path);
	}
	printf("See log path for results: %s\n", pathDir);
	fclose(logFile);
}