#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

	//if argc == 1, no arguments given, exit with status 1
	if(argc == 1){
		printf("wzip: [file1] [file2]...\n");
		exit(1);
	}//if argc > 1, files were given
	else{
		//iterate over each file
		int i;
		for(i = 1; i < argc; i++){
			
			//open and check the file
			FILE *fid = fopen(argv[i],"r");
			if(fid == NULL){
				printf("wzip: cannot open file \n");
				exit(1);
			}

			//reading each char, tracking pervious char, and incrementing count
			int count = 0;
			char previousChar = 0;
			char currentChar = 0;
			
			//go through file, documenting characters until eof is reached
			//breaks when current char is equal to EOF
			while(1){
				
				//get next char
				//char can be new, old, or the eof
				currentChar = fgetc(fid);
				
				//if next char is eof, reached end of file
				//make sure to write any info
				//then break
				if(currentChar == EOF){
					//write if we have counted occurrences of char
					if(previousChar != 0){
						fwrite(&count, sizeof(int), 1, stdout);//4 byte integer
						fwrite(&previousChar, 1, 1, stdout);//1 character
					}
					break;
				}

				//if current char is new, write previous char and info
				//reset counter for newly found char
				if(currentChar != previousChar){
					if(previousChar != 0){
						fwrite(&count, 1, 4, stdout);
						fwrite(&previousChar, 1, 1, stdout);
					}
					//resetting counter for next char
					count = 0;
				}

				//if char not new or not eof, continue
				//increment count
				//update perivous char to current
				previousChar = currentChar;
				count++;
			}
			//close file
			fclose(fid);
		}
	}

}
