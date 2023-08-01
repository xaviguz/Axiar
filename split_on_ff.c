/*	split_on_reset.c -- separates stdin into files at each PCL reset
		called with one argument containing the name of the backer (trailer) to be added to each file -- found in /usr/spool/formfont/hpmacros */  

/* Copyright 2016, LBM Systems Corp.*/
static char cpy_rgt_str[] = "Copyright 2016, LBM Systems Corp.";

#include <stdio.h>
#include <string.h>

#include <sys/param.h>
#include <unistd.h>
#include <stdlib.h>

#include <ctype.h>

#include <errno.h>

FILE *input_file;
FILE *output_file;
FILE *backer_file;

char backer_filename[512];
char output_filename[512];
char backer_name[128];

char logfilepath[257];
char fffilepath[257];

int main(argc, argv)
int argc;
char *argv[];

{
	char ch;
	int letter_count;
	int first_time=1;
	
	// Get path to axiar
	if (getenv("AXIAR_CYG")) {
			strcpy(logfilepath, getenv("AXIAR_CYG"));
			strcpy(fffilepath, getenv("AXIAR_CYG"));
			strcat(logfilepath, "/uprint/");
			strcat(fffilepath, "/formfont/hpmacros");
	} else if (getenv("USPOOL")) {
			strcpy(logfilepath, getenv("USPOOL"));
			strcat(logfilepath, "/");
			strcpy(fffilepath, getenv("FFPATH"));
			strcat(fffilepath, "/hpmacros");
	} else if (getenv("AXIAR_POSIX")) {
			strcpy(logfilepath, getenv("AXIAR_POSIX"));
			strcpy(fffilepath, getenv("AXIAR_POSIX"));
			strcat(logfilepath, "/uprint/");
			strcat(fffilepath, "/formfont/hpmacros");
	} else {
			strcpy(logfilepath, "/usr/spool/uprint/");
			strcat(fffilepath, "/usr/spool/formfont/hpmacros");
	}

	strcpy(backer_name, argv[1]);			// open backer PCL file for appending on each letter
	strcat(fffilepath, "/%s");
	sprintf(backer_filename, fffilepath , backer_name);
	
	if ((!(backer_file=fopen(backer_filename, "rb")))) {
		fprintf(stderr, "Could not open %s (%s)\n", backer_filename, strerror(errno));
		exit(16);
	}

	input_file=stdin;			// read stdin

	letter_count=1;
	start_new_file(letter_count);		// get output file /usr/spool/uprint/printfiles/{backer_name}.count

	ch=fgetc(input_file);			// get first character
	while (feof(input_file)==0) {			// loop until end of file
		if ((ch == '\033')) {			// test for letter separator -- <esc>E
			ch=fgetc(input_file);		
			if ((!first_time) && (ch == 'E')) {		// if PCL reset and not beginning of file, this is the end of a letter
				letter_count++;
				fputc('\f', output_file);		// add page feed to keep letter simplex
				start_new_file(letter_count);			// get next output file name
			} else {
				first_time=0;	
			}
			fputc('\033', output_file);		// write out the <esc>
		} else if (ch == '\f') {			// if page feed, write another to keep the letter simplex
			fputc('\f', output_file);
		}
		fputc(ch, output_file);		// write out the character
		ch=fgetc(input_file);		// read the next character
	}
	return(letter_count);
}

int start_new_file(letter_count) 
{
	char buffer[1024];
	int bytes_read;
	if (output_file) {   //  append the backer if this is not the first letter
		while ((bytes_read=fread(buffer, 1, sizeof(buffer), backer_file))) {
			fwrite(buffer, 1, bytes_read, output_file);
		}
		rewind(backer_file);  // rewind the file so it can be appended to the next letter
		fclose(output_file);
	}
	strcat(logfilepath, "printfiles/%s.%03d");
	sprintf(output_filename, logfilepath, backer_name, letter_count);
	if ((!(output_file=fopen(output_filename, "wb")))) {
		fprintf(stderr, "Could not open %s (%s)\n", output_filename, strerror(errno));
		exit(16);
	}
}



