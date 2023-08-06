#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

	//if argc == 1, no arguments given, exit with status 1
	if(argc == 1){
		printf("wunzip: file1 [file2]...\n");
		exit(1);
	}//if more than one argument, then go through each file, read each int and char, then print
	else{
		
		//going through each file
		int i;
		for(i = 1; i < argc; i++){
			
			//open and check file
			FILE *fid = fopen(argv[i], "rb");
			if(fid == NULL){
				printf("wunzip: cannot open file");
				exit(1);
			}
			
			//tracking the current character and its occurrences
			char currentChar = 0;
			int count = 0;
			//while fread returns a value, go through file
			int value;
			while(1){

				value = fread(&count, sizeof(int), 1, fid);
				if(value != 1){
					break;
				}

				fread(&currentChar, 1, 1, fid);
				
				//write out the number of times char appears
				int j;
				for(j = 0; j < count; j++){
					fwrite(&currentChar, 1, 1, stdout);
				}
			}
			
			//close file
			fclose(fid);
		}
	}

}
