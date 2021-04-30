
#ifndef _GETOPT_H

# define no_argument            0
# define required_argument      1
# define optional_argument      2
struct option
{
	const char* name;
	int has_arg;
	int* flag;
	int val;
};
extern char* optarg;
extern int optind;
extern int opterr;
extern int getopt(int ___argc, char* const* ___argv, const char* __shortopts);
#endif /* getopt.h */
