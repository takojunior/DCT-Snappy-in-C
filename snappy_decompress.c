
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <tistdtypes.h>
#include <c6x.h>

void copy_buf(char *uncompressed, long copylength, char *startcopy);
void copy_buf_asm(char *uncompressed, long copylength, char *startcopy);
void copy_buf_asm_offset_1(char *uncompressed, long copylength, char *startcopy);
int snappy_uncompress(char *compressed, long compressed_length, char *uncompressed, long uncompressed_length);


void copy_buf(char *uncompressed, long copylength, char *startcopy)
{
    long i;
	for(i=0;i<copylength;i++){
	    uncompressed[i]=startcopy[i];
	}
}


int snappy_uncompress(char *compressed, long compressed_length, char *uncompressed, long uncompressed_length)
{
    unsigned char a,b,c;
	char *uncompressed_start;
//	Uint32 t1, tcopy;
	unsigned long temp,temp1,k,oplength;
	uncompressed_start=uncompressed;
	
	TSCL=0;
	 
	//tcopy=0;
	while ((uncompressed-uncompressed_start)<=uncompressed_length){              // decide whether the decoding finishes
	  a=*compressed++;
	  //printf("\n %x",a);
	  b=a&0x03;
	  temp=0;
	  temp1=0;
	  switch (b) {
	  case 0 :  //printf(" : Literal");                                         // if the element is literal, then read the length out
	            c=a>>2;
				if (c<60) temp=c+1;
	            else {
	                 switch (c) {
	                 case 60 : temp=(unsigned char)(*compressed++)+1;break;
	                 case 61 : temp=temp+(unsigned char)(*compressed++);
	                           temp=temp+256*(unsigned char)(*compressed++);
				               temp=temp+1;
				               break;
	                 case 62 : temp=temp+(unsigned char)(*compressed++);
	                           temp=temp+256*(unsigned char)(*compressed++);
				               temp=temp+65536*(unsigned char)(*compressed++);
				               temp=temp+1;
				               break;
	                 case 63 : temp=temp+(unsigned char)(*compressed++);
	                           temp=temp+256*(unsigned char)(*compressed++);
				               temp=temp+65536*(unsigned char)(*compressed++);
				               temp=temp+16777216*(unsigned char)(*compressed++);
				               temp=temp+1;
				               break;
	                 default : printf("Error: length of literal.");
	            }
				}

				//t1=TSCL;
				for(k=0;k<temp;k++)                                            // copy directly from compressed stream
				{(*uncompressed++)=(*compressed++);}
				//copy_buf_asm(uncompressed, temp,compressed);
				//tcopy+=(TSCL-t1);
				break;
	  case 1 :  temp1=temp1+(unsigned char)(*compressed++);                     // if the element is 1 byte copy, then get the length and offset
				temp1=temp1+(a>>5)*256;
				oplength=((a&0x1c)>>2)+4;
				//if (temp1==1){
				//t1=TSCL;
				//copy_buf_asm_offset_1(uncompressed,oplength,uncompressed-temp1);
				//tcopy+=(TSCL-t1);
				//}
				//else {
				//t1=TSCL;
				copy_buf(uncompressed,oplength,uncompressed-temp1);            // call the function to do the copy operation
				//tcopy+=(TSCL-t1);
				//}
				uncompressed+=oplength;
                break;
	  case 2 :  temp1=temp1+(unsigned char)(*compressed++);                    // if the element is 2 byte copy, then get the length and offset
		        temp1=temp1+(unsigned char)(*compressed++)*256;
				oplength=(a>>2)+1;
				//if (temp1==1){
				//t1=TSCL;
				//copy_buf_asm_offset_1(uncompressed,oplength,uncompressed-temp1);
				//tcopy+=(TSCL-t1);
				//}
				//else {
				//t1=TSCL;
				copy_buf(uncompressed,oplength,uncompressed-temp1);            // call the function to do the copy operation
				//tcopy+=(TSCL-t1);
				//}
				uncompressed+=oplength;
			   	break;
	  case 3 :  temp1=temp1+(unsigned char)(*compressed++);                    //if the element is 4 byte copy, then get the length and offset
		        temp1=temp1+(unsigned char)(*compressed++)*256;
				temp1=temp1+(unsigned char)(*compressed++)*65536;
				temp1=temp1+(unsigned char)(*compressed++)*16777216;
				oplength=(a>>2)+1;
				//if (temp1==1){
				//t1=TSCL;
				//copy_buf_asm_offset_1(uncompressed,oplength,uncompressed-temp1);
				//tcopy+=(TSCL-t1);
				//}
				//else {
				//t1=TSCL;
				copy_buf_asm(uncompressed,oplength,uncompressed-temp1);
				//tcopy+=(TSCL-t1);
				//}
				uncompressed+=oplength;
				break;
	  default : printf(" Not a tag byte");
	  }
	  //m++;
	}

	return 1;
}

#pragma FUNC_EXT_CALLED ( the_main_task );

void the_main_task()
{
    Uint32 t;
    FILE *read_fp, *write_fp;
	unsigned char a,b;
	//char c;
	char *outbuf,*compressed,*tempcompressed;
	int i=0,numpreamble;
	long snappy_uncompressed_ok;
	long compressed_length;
	long preamble=0;
	if((read_fp=fopen("E:\\alice29.txt.comp","rb"))==NULL)               // open the file
	{
	  printf("\n Can not open the file.");
	  exit(1);
	}
	if((write_fp=fopen("E:\\alice29.txt","wb"))==NULL)
	{
	  printf("\n Can not open the file.");
	  exit(1);
	}

	
	a=fgetc(read_fp);                                              // get the preamble from the file
	b=a&(0x7F);
	while ((a>>7)&1) {
	  preamble=preamble+b*pow(128,i);
	  i++;
	  a=fgetc(read_fp);
	  b=a&(0x7F);
	}
	preamble=preamble+b*pow(128,i);
	numpreamble=i+1;
	printf("\n The length of uncompressed data is %d", preamble);
	printf("\n The number of bytes for preamble is %d", numpreamble);

			
	rewind(read_fp);                                                      // get the compressed_length and initialize the memory space to save the data
	fseek(read_fp,0,2);
	compressed_length=ftell(read_fp);
	compressed=malloc((compressed_length+preamble)*sizeof(char));
	if (compressed==NULL){
	    printf("Not enough memory space for compressed data.");
		exit(1);
	}
	outbuf=compressed+compressed_length*sizeof(char);
	tempcompressed=compressed;

	rewind(read_fp);                                                 // save the compressed data to char * compressed, and then exclude preamble
	fread(compressed,sizeof(char),compressed_length,read_fp);
	printf("\n The length of compressed data is %d",compressed_length-numpreamble);
	compressed=tempcompressed+numpreamble;

	TSCL=0; // initialize the timer
	
	// start depressing
	t=TSCL;
	snappy_uncompressed_ok=snappy_uncompress(compressed, compressed_length, outbuf,preamble);   // call snappy_uncompressed
	printf(" \n Bytes decompressed per cycle are %.4f", (double)(preamble)/(TSCL-t));
	printf(" \n Total execution time is %.3f ms", (double) (TSCL - t) / 300000.0);
	if (snappy_uncompressed_ok==1) printf("\n complete the decompression.");

	
	fwrite(outbuf,sizeof(char),preamble,write_fp);
	fclose(write_fp);
	fclose(read_fp);
	free(compressed);

	
}


// Main function 
void main() 
{
	the_main_task();
    return;
}
