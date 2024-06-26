/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo                           *
 *   massimiliano.torromeo@gmail.com                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "instrumentfunctions.h"
#include "debug.h"

#ifdef INSTRUMENT

#include <bfd.h>

/*
 * Generate instrumentation calls for entry and exit to functions.
 * Just after function entry and just before function exit, the
 * following profiling functions are called with the address of the
 * current function and its call site (currently not use).
 *
 * The attribute 'no_instrument_function' causes this instrumentation is
 * not done.
 *
 * These profiling functions call print_debug(). This function prints the
 * current function name with indentation and call level.
 * If entering in a function it prints also the calling function name with
 * file name and line number.
 *
 * If the environment variable INSTRUMENT is
 * unset or set to an empty string, print nothing, like with no instrumentation
 * set to "all" or "a", print all the functions names
 * set to "global" or "g", print only the global functions names
 */

#define ND_NO_INSTRUMENT __attribute__((no_instrument_function))

/* Store the function call level, used also in pretty_print_packet() */
extern int profile_func_level;
int profile_func_level = -1;

typedef enum {
	ENTER,
	EXIT
} action_type;

extern "C" void __cyg_profile_func_enter(void *this_fn, void *call_site) ND_NO_INSTRUMENT;
extern "C" void __cyg_profile_func_exit(void *this_fn, void *call_site) ND_NO_INSTRUMENT;
static void print_debug(void *this_fn, void *call_site, action_type action) ND_NO_INSTRUMENT;

extern "C" void __cyg_profile_func_enter(void *this_fn, void *call_site) {
	print_debug(this_fn, call_site, ENTER);
}

extern "C" void __cyg_profile_func_exit(void *this_fn, void *call_site) {
	print_debug(this_fn, call_site, EXIT);
}

static void print_debug(void *this_fn, void *call_site, action_type action) {
	static bfd *abfd = NULL;
	static asymbol **symtab = NULL;
	static long symcount = 0;
	static asection *text = NULL;
	static bfd_vma vma;
	static int instrument_set = 0;
	static int instrument_off = 0;
	static int instrument_global = 0;

	if (!instrument_set) {
		/* Get the configuration environment variable INSTRUMENT value if any */
		const char *instrument_type = getenv("INSTRUMENT");
		/* unset or set to an empty string ? */
		if (instrument_type == NULL ||
			!strncmp(instrument_type, "", sizeof(""))) {
			instrument_off = 1;
		} else {
			/* set to "global" or "g" ? */
			if (!strncmp(instrument_type, "global", sizeof("global")) || !strncmp(instrument_type, "g", sizeof("g"))) {
				instrument_global = 1;
			} else if (strncmp(instrument_type, "all", sizeof("all")) && strncmp(instrument_type, "a", sizeof("a"))) {
				fprintf(stderr, "INSTRUMENT can be only \"\", \"all\", \"a\", \"global\" or \"g\".\n");
				exit(1);
			}
		}
		instrument_set = 1;
	}

	if (instrument_off)
		return;

	/* If no errors, this block should be executed one time */
	if (!abfd) {
		char pgm_name[1024];
		long symsize;

		ssize_t ret = readlink("/proc/self/exe", pgm_name, sizeof(pgm_name));
		if (ret == -1) {
			perror("failed to find executable");
			return;
		}
		if (ret == sizeof(pgm_name)) {
			/* no space for the '\0' */
			printf("truncation may have occurred\n");
			return;
		}
		pgm_name[ret] = '\0';

		bfd_init();

		abfd = bfd_openr(pgm_name, NULL);
		if (!abfd) {
			bfd_perror("bfd_openr");
			return;
		}

		if (!bfd_check_format(abfd, bfd_object)) {
			bfd_perror("bfd_check_format");
			return;
		}

		if((symsize = bfd_get_symtab_upper_bound(abfd)) == -1) {
			bfd_perror("bfd_get_symtab_upper_bound");
			return;
		}

		symtab = (asymbol **)malloc((size_t)symsize);
		symcount = bfd_canonicalize_symtab(abfd, symtab);
		if (symcount < 0) {
			free(symtab);
			bfd_perror("bfd_canonicalize_symtab");
			return;
		}

		if ((text = bfd_get_section_by_name(abfd, ".text")) == NULL) {
			bfd_perror("bfd_get_section_by_name");
			return;
		}
		vma = text->vma;
	}

	if (instrument_global) {
		symbol_info syminfo;
		int found = 0;
		long i = 0;
		while (i < symcount && !found) {
			bfd_get_symbol_info(abfd, symtab[i], &syminfo);
			if ((void *)syminfo.value == this_fn) {
				found = 1;
			}
			i++;
		}
		/* type == 'T' for a global function */
		if (found == 1 && syminfo.type != 'T')
			return;
	}

	/* Current function */
	if ((bfd_vma)this_fn < vma) {
		printf("[ERROR address this_fn]");
	} else {
		const char *file;
		const char *func;
		unsigned int line;

		if (!bfd_find_nearest_line(abfd, text, symtab, (bfd_vma)this_fn - vma, &file, &func, &line)) {
			printf("[ERROR bfd_find_nearest_line this_fn]");
		} else {
			if (action == ENTER)
				profile_func_level += 1;
			/* Indentation */
			for (int i = 0 ; i < profile_func_level ; i++)
				putchar(' ');
			if (action == ENTER)
				printf("[>> ");
			else
				printf("[<< ");
			/* Function name */
			if (func == NULL || *func == '\0')
				printf("???");
			else
				printf("%s", func);
			printf(" (%d)", profile_func_level);
			/* Print the "from" part except for the main function) */
			if (action == ENTER && func != NULL &&
				strncmp(func, "main", sizeof("main"))) {
				/* Calling function */
				if ((bfd_vma)call_site < vma) {
					printf("[ERROR address call_site]");
				} else {
					if (!bfd_find_nearest_line(abfd, text, symtab, (bfd_vma)call_site - vma, &file, &func, &line)) {
						printf("[ERROR bfd_find_nearest_line call_site]");
					} else {
						printf(" from ");
						/* Function name */
						if (func == NULL || *func == '\0')
							printf("???");
						else
							printf("%s", func);
						/* File name */
						if (file == NULL || *file == '\0')
							printf(" ??:");
						else {
							const char *slashp = strrchr(file, '/');
							if (slashp != NULL)
								file = slashp + 1;
							printf(" %s:", file);
						}
						/* Line number */
						if (line == 0)
							printf("?");
						else
							printf("%u", line);
						printf("]");
					}
				}
			}
			putchar('\n');
			if (action == EXIT)
				profile_func_level -= 1;
		}
	}
	fflush(stdout);
}

#else

# warning Instruments not available.

#endif // INSTRUMENT
