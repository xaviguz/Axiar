#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#define OPTSTR "i:s:Lh?"

char seq_filename[256];
int increase_by=1;
int logging=0;
int new_starting_value=0;


main(argc, argv)
int argc;
char *argv[];
{
	char inbuf[100];
	FILE *seq_file;
	FILE *errfile;
	int sequence_no;

	errfile=stderr;

	parseopts(argc, argv);


	if (!strchr(seq_filename, '/')) {
		char work_path[256];
		if (getenv("AXIAR_POSIX")) {
			sprintf(work_path, "%s/uprint/%s", getenv("AXIAR_POSIX"), seq_filename);
			strcpy(seq_filename, work_path);
		} else if (getenv("AXIAR_CYG")) {
			sprintf(work_path, "%s/uprint/%s", getenv("AXIAR_CYG"), seq_filename);
			strcpy(seq_filename, work_path);
		} else if (getenv("USPOOL")) {
			sprintf(work_path, "%s/%s", getenv("USPOOL"), seq_filename);
			strcpy(seq_filename, work_path);
		} else {
			sprintf(work_path, "/usr/spool/uprint/%s", seq_filename);
			strcpy(seq_filename, work_path);
		}
	}

	seq_file = fopen(seq_filename, "r+");
	if (!seq_file) {
		seq_file = fopen(seq_filename, "w+");
		if (!seq_file) {
			fprintf(errfile,"fopen failed (%s): errno=%d (%s)\n", seq_filename, errno, strerror(errno));
		}	
		if (chmod(seq_filename, 0666) < 0) {
			fprintf(errfile,"chmod failed (%s): errno=%d (%s)\n", seq_filename, errno, strerror(errno));
		}
	}
	if (logging) fprintf(errfile, "Using sequence_file %s\n", seq_filename);
	if (seq_file) {
#ifndef NOLOCK
		struct flock lock;
		lock.l_type = F_WRLCK;
		lock.l_start = 0;
		lock.l_whence = 0;
		lock.l_len = 0;
		errno = 0;
		if (fcntl(fileno(seq_file), F_SETLKW, &lock) < 0) {
			fprintf(errfile,"fcnt for lock failed: errno=%d (%s)\n", errno, strerror(errno));
			exit(0);
		} 
	
		if (logging) fprintf(errfile, "Locked sequence_file %s\n", seq_filename);
#endif

		if (fgets(inbuf, sizeof(inbuf), seq_file) != NULL) {
			sequence_no = atoi(inbuf);
			sequence_no += increase_by;
			rewind(seq_file);
			fprintf(seq_file, "%d\n", sequence_no);
#ifndef NOLOCK
			lock.l_type = F_UNLCK;
			fcntl(fileno(seq_file), F_SETLK, &lock);
#endif
			fclose(seq_file);
			sequence_no -= increase_by;
		} else {
			sequence_no = 1;
			fprintf(seq_file, "2\n");
#ifndef NOLOCK
			lock.l_type = F_UNLCK;
			fcntl(fileno(seq_file), F_SETLK, &lock);
#endif
			fclose(seq_file);
		}
	}
	fprintf(stdout, "%d", sequence_no);
	if (logging) fprintf(errfile, "Wrote sequence_file %s\n", seq_filename);
	exit(0);
}

parseopts(cnt,v)
int cnt;
char *v[];
{
	int c;
	extern int optind, opterr;
	extern char *optarg;

	if (cnt > 4) usage();
	if (cnt < 2) usage();

	seq_filename[0] = '\0';

	while ((c=getopt(cnt, v, OPTSTR))!=EOF) {
		switch (c) {
			case 's':
				strcpy(seq_filename, optarg);
				break;
			case 'i':
				increase_by=atoi(optarg);
				break;
			case 'L':
				logging++;
				break;
			case '?':
			case 'h':
			default:
				usage();
				break;
		}
	}
	if (strlen(seq_filename)==0) {
		fprintf(stderr, "filename must be specified for sequence file number\n");
		exit(0);
	}
	return 0;
}

usage()
{
	fprintf(stderr, "serial_no: increments serial_no stored in file (with locking)\n");
	fprintf(stderr, "usage: serial_no -i[increment] -s[sequence file path] -L\n");
	fprintf(stderr, "arguments:\n");
	fprintf(stderr, "          -i 999          value to increase stored sequence number (optional, default is 1)\n");
	fprintf(stderr, "          -s file_path    file name or full path of starting number (default path is $AXIAR_POSIX/uprint)\n");
	fprintf(stderr, "          -L              logging enabled\n");
	fprintf(stderr, "          -h              show this message\n");
	exit(0);
}
