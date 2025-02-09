/* vi: set sw=4 ts=4: */
/*
 * getopt.c - Enhanced implementation of BSD getopt(1)
 *   Copyright (c) 1997, 1998, 1999, 2000  Frodo Looijaard <frodol@dds.nl>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

/*
 * Version 1.0-b4: Tue Sep 23 1997. First public release.
 * Version 1.0: Wed Nov 19 1997.
 *   Bumped up the version number to 1.0
 *   Fixed minor typo (CSH instead of TCSH)
 * Version 1.0.1: Tue Jun 3 1998
 *   Fixed sizeof instead of strlen bug
 *   Bumped up the version number to 1.0.1
 * Version 1.0.2: Thu Jun 11 1998 (not present)
 *   Fixed gcc-2.8.1 warnings
 *   Fixed --version/-V option (not present)
 * Version 1.0.5: Tue Jun 22 1999
 *   Make -u option work (not present)
 * Version 1.0.6: Tue Jun 27 2000
 *   No important changes
 * Version 1.1.0: Tue Jun 30 2000
 *   Added NLS support (partly written by Arkadiusz Mickiewicz
 *     <misiek@misiek.eu.org>)
 * Ported to Busybox - Alfred M. Szmidt <ams@trillian.itslinux.org>
 *  Removed --version/-V and --help/-h in
 *  Removed parse_error(), using bb_error_msg() from Busybox instead
 *  Replaced our_malloc with xmalloc and our_realloc with xrealloc
 *
 */

#include <getopt.h>
#include "libbb.h"

/* NON_OPT is the code that is returned when a non-option is found in '+'
   mode */
enum {
	NON_OPT = 1,
#if ENABLE_GETOPT_LONG
/* LONG_OPT is the code that is returned when a long option is found. */
	LONG_OPT = 2
#endif
};

/* For finding activated option flags. Must match getopt32 call! */
enum {
	OPT_o	= 0x1,	// -o
	OPT_n	= 0x2,	// -n
	OPT_q	= 0x4,	// -q
	OPT_Q	= 0x8,	// -Q
	OPT_s	= 0x10,	// -s
	OPT_T	= 0x20,	// -T
	OPT_u	= 0x40,	// -u
#if ENABLE_GETOPT_LONG
	OPT_a	= 0x80,	// -a
	OPT_l	= 0x100, // -l
#endif
	SHELL_IS_TCSH = 0x8000, /* hijack this bit for other purposes */
};

/* 0 is getopt_long, 1 is getopt_long_only */
#define alternative  (option_mask32 & OPT_a)

#define quiet_errors (option_mask32 & OPT_q)
#define quiet_output (option_mask32 & OPT_Q)
#define quote        (!(option_mask32 & OPT_u))
#define shell_TCSH   (option_mask32 & SHELL_IS_TCSH)

/*
 * This function 'normalizes' a single argument: it puts single quotes around
 * it and escapes other special characters. If quote is false, it just
 * returns its argument.
 * Bash only needs special treatment for single quotes; tcsh also recognizes
 * exclamation marks within single quotes, and nukes whitespace.
 * This function returns a pointer to a buffer that is overwritten by
 * each call.
 */
static const char *normalize(const char *arg)
{
	char *bufptr;
#if ENABLE_FEATURE_CLEAN_UP
	static char *BUFFER = NULL;
	free(BUFFER);
#else
	char *BUFFER;
#endif

	if (!quote) { /* Just copy arg */
		BUFFER = xstrdup(arg);
		return BUFFER;
	}

	/* Each character in arg may take up to four characters in the result:
	   For a quote we need a closing quote, a backslash, a quote and an
	   opening quote! We need also the global opening and closing quote,
	   and one extra character for '\0'. */
	BUFFER = xmalloc(strlen(arg)*4 + 3);

	bufptr = BUFFER;
	*bufptr ++= '\'';

	while (*arg) {
		if (*arg == '\'') {
			/* Quote: replace it with: '\'' */
			*bufptr ++= '\'';
			*bufptr ++= '\\';
			*bufptr ++= '\'';
			*bufptr ++= '\'';
		} else if (shell_TCSH && *arg == '!') {
			/* Exclamation mark: replace it with: \! */
			*bufptr ++= '\'';
			*bufptr ++= '\\';
			*bufptr ++= '!';
			*bufptr ++= '\'';
		} else if (shell_TCSH && *arg == '\n') {
			/* Newline: replace it with: \n */
			*bufptr ++= '\\';
			*bufptr ++= 'n';
		} else if (shell_TCSH && isspace(*arg)) {
			/* Non-newline whitespace: replace it with \<ws> */
			*bufptr ++= '\'';
			*bufptr ++= '\\';
			*bufptr ++= *arg;
			*bufptr ++= '\'';
		} else
			/* Just copy */
			*bufptr ++= *arg;
		arg++;
	}
	*bufptr ++= '\'';
	*bufptr ++= '\0';
	return BUFFER;
}

/*
 * Generate the output. argv[0] is the program name (used for reporting errors).
 * argv[1..] contains the options to be parsed. argc must be the number of
 * elements in argv (ie. 1 if there are no options, only the program name),
 * optstr must contain the short options, and longopts the long options.
 * Other settings are found in global variables.
 */
#if !ENABLE_GETOPT_LONG
#define generate_output(argv,argc,optstr,longopts) \
	generate_output(argv,argc,optstr)
#endif
static int generate_output(char **argv, int argc, const char *optstr, const struct option *longopts)
{
	int exit_code = 0; /* We assume everything will be OK */
	int opt;
#if ENABLE_GETOPT_LONG
	int longindex;
#endif
	const char *charptr;

	if (quiet_errors) /* No error reporting from getopt(3) */
		opterr = 0;

	while (1) {
		opt =
#if ENABLE_GETOPT_LONG
			alternative ?
			getopt_long_only(argc, argv, optstr, longopts, &longindex) :
			getopt_long(argc, argv, optstr, longopts, &longindex);
#else
			getopt(argc, argv, optstr);
#endif
		if (opt == -1)
			break;
		if (opt == '?' || opt == ':' )
			exit_code = 1;
		else if (!quiet_output) {
#if ENABLE_GETOPT_LONG
			if (opt == LONG_OPT) {
				printf(" --%s", longopts[longindex].name);
				if (longopts[longindex].has_arg)
					printf(" %s",
						normalize(optarg ? optarg : ""));
			} else
#endif
			if (opt == NON_OPT)
				printf(" %s", normalize(optarg));
			else {
				printf(" -%c", opt);
				charptr = strchr(optstr, opt);
				if (charptr != NULL && *++charptr == ':')
					printf(" %s",
						normalize(optarg ? optarg : ""));
			}
		}
	}

	if (!quiet_output) {
		printf(" --");
		while (optind < argc)
			printf(" %s", normalize(argv[optind++]));
		bb_putchar('\n');
	}
	return exit_code;
}

#if ENABLE_GETOPT_LONG
/*
 * Register several long options. options is a string of long options,
 * separated by commas or whitespace.
 * This nukes options!
 */
static struct option *add_long_options(struct option *long_options, char *options)
{
	int long_nr = 0;
	int arg_opt, tlen;
	char *tokptr = strtok(options, ", \t\n");

	if (long_options)
		while (long_options[long_nr].name)
			long_nr++;

	while (tokptr) {
		arg_opt = no_argument;
		tlen = strlen(tokptr);
		if (tlen) {
			tlen--;
			if (tokptr[tlen] == ':') {
				arg_opt = required_argument;
				if (tlen && tokptr[tlen-1] == ':') {
					tlen--;
					arg_opt = optional_argument;
				}
				tokptr[tlen] = '\0';
				if (tlen == 0)
					bb_error_msg_and_die("empty long option specified");
			}
			long_options = xrealloc_vector(long_options, 4, long_nr);
			long_options[long_nr].has_arg = arg_opt;
			/*long_options[long_nr].flag = NULL; - xrealloc_vector did it */
			long_options[long_nr].val = LONG_OPT;
			long_options[long_nr].name = xstrdup(tokptr);
			long_nr++;
			/*memset(&long_options[long_nr], 0, sizeof(long_options[0])); - xrealloc_vector did it */
		}
		tokptr = strtok(NULL, ", \t\n");
	}
	return long_options;
}
#endif

static void set_shell(const char *new_shell)
{
	if (!strcmp(new_shell, "bash") || !strcmp(new_shell, "sh"))
		return;
	if (!strcmp(new_shell, "tcsh") || !strcmp(new_shell, "csh"))
		option_mask32 |= SHELL_IS_TCSH;
	else
		bb_error_msg("unknown shell '%s', assuming bash", new_shell);
}


/* Exit codes:
 *   0) No errors, successful operation.
 *   1) getopt(3) returned an error.
 *   2) A problem with parameter parsing for getopt(1).
 *   3) Internal error, out of memory
 *   4) Returned for -T
 */

#if ENABLE_GETOPT_LONG
static const char getopt_longopts[] ALIGN1 =
	"options\0"      Required_argument "o"
	"longoptions\0"  Required_argument "l"
	"quiet\0"        No_argument       "q"
	"quiet-output\0" No_argument       "Q"
	"shell\0"        Required_argument "s"
	"test\0"         No_argument       "T"
	"unquoted\0"     No_argument       "u"
	"alternative\0"  No_argument       "a"
	"name\0"         Required_argument "n"
	;
#endif

int getopt_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int getopt_main(int argc, char **argv)
{
	char *optstr = NULL;
	char *name = NULL;
	unsigned opt;
	const char *compatible;
	char *s_arg;
#if ENABLE_GETOPT_LONG
	struct option *long_options = NULL;
	llist_t *l_arg = NULL;
#endif

	compatible = getenv("GETOPT_COMPATIBLE"); /* used as yes/no flag */

	if (argc == 1) {
		if (compatible) {
			/* For some reason, the original getopt gave no error
			   when there were no arguments. */
			printf(" --\n");
			return 0;
		}
		bb_error_msg_and_die("missing optstring argument");
	}

	if (argv[1][0] != '-' || compatible) {
		char *s;

		option_mask32 |= OPT_u; /* quoting off */
		s = xstrdup(argv[1] + strspn(argv[1], "-+"));
		argv[1] = argv[0];
		return generate_output(argv+1, argc-1, s, long_options);
	}

#if !ENABLE_GETOPT_LONG
	opt = getopt32(argv, "+o:n:qQs:Tu", &optstr, &name, &s_arg);
#else
	applet_long_options = getopt_longopts;
	opt_complementary = "l::";
	opt = getopt32(argv, "+o:n:qQs:Tual:",
					&optstr, &name, &s_arg, &l_arg);
	/* Effectuate the read options for the applet itself */
	while (l_arg) {
		long_options = add_long_options(long_options, llist_pop(&l_arg));
	}
#endif

	if (opt & OPT_s) {
		set_shell(s_arg);
	}

	if (opt & OPT_T) {
		return 4;
	}

	/* All options controlling the applet have now been parsed */
	if (!optstr) {
		if (optind >= argc)
			bb_error_msg_and_die("missing optstring argument");
		optstr = argv[optind++];
	}

	argv[optind-1] = name ? name : argv[0];
	return generate_output(argv+optind-1, argc-optind+1, optstr, long_options);
}
