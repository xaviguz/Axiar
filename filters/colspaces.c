/* filter to remove leading spaces at beginning of each line based on the minimum number of spaces */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>


main()
{
char buffer[2048];

char *ptr;
int min_spaces=100;
int temp_fd, i;
FILE *temp_file;
char temp_file_path[256];

	sprintf(temp_file_path, "rem_spaces.%d", getpid());
	temp_file=fopen(temp_file_path, "a+");
	if (!temp_file) {
		fprintf(stderr, "Couldn't open temp file (%s)\n", strerror(errno));
		exit(16);
	}
/* read file line-by-line */
	while (fgets(buffer, sizeof(buffer), stdin)) {
		i=0;
		ptr=buffer;
		while ((*ptr) && ((*ptr) == ' ')) {
			ptr++;
			i++;
		}
		if ((i) && (i < min_spaces))
			min_spaces=i;
		fputs(buffer, temp_file);
	}
	rewind(temp_file);
	while (fgets(buffer, sizeof(buffer), temp_file)) {
		i=0;
		ptr=buffer;
		while ((i<min_spaces) && (*ptr) && ((*ptr) == ' ')) {
			ptr++;
			i++;
		}
		fputs(ptr, stdout);
	}
	unlink(temp_file_path);
	exit(0);
}
