#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
	
	//if no search term or file provided, exit
	if(argc == 1){
		printf("wgrep: searchterm [filename] \n");
		exit(1);
	}//if only search term provided, read from user input
	else if(argc == 2){
		
		while(1){
			char buffer[500];
			if(fgets(buffer, 500, stdin) != NULL){
				if(strstr(buffer, argv[1]) != NULL){
					printf("%s", buffer);
				}
			}
		}
	//if searchterm and file/files provided
	//go through each file, check if file is open
	//check each line
	//print if search term is in file
	}else if(argc > 2){
		
		int i;
		for(i=2; i<argc; i++){
			
			//opening file
			FILE *fid = fopen(argv[i], "r");
			
			//checking if file opened
			if(fid == NULL){
				printf("wgrep: cannot open file \n");
				exit(1);
			}
			
			//getting length of file
			int len = 0;
			while(getc(fid)!= EOF){
				len++;
			}
			
			rewind(fid);
			char file_line[len+1];
			
			//going through each line of file
			//if searchTerm found in line, print the line
			while (fgets(file_line, len+1, fid)){
				if(strstr(file_line, argv[1])!= NULL){
					printf("%s", file_line);
				}
			}
			fclose(fid);
		}
	}
	return 0;
}
