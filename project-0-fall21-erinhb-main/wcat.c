#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

	//if only one argument given, no file to read, therefore exit
	if(argc == 1){
		exit(0);
	}

	//iterate through argv to read each file given in the command
	//open, read, and print each one
	int i;
	for(i=1; i<argc; i++){
		//treating command at argv[i] as a file name and opening file
		FILE *fid = fopen(argv[i], "r");
		
		//checking if can open file
		if(fid == NULL){
			printf("wcat: cannot open file \n");
			exit(1);
		}
		
		//getting length of file
		int len = 0;
		while(getc(fid)!=EOF){
			len++;
		}
		
		//rewind and read file
		char read_array[len+1];
		rewind(fid);
		int i;
		for(i=0; i<len; i++){
			read_array[i] = getc(fid);
		}

		read_array[i] = '\0';
		printf("%s", read_array);
		
		fclose(fid);
	}
	return 0;
}
