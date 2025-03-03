/*
 *  nandwrite.c
 *
 *  Copyright (C) 2000 Steven J. Hill (sjhill@realitydiluted.com)
 *   		  2003 Thomas Gleixner (tglx@linutronix.de)
 *
 * $Id: nandwrite.c,v 1.1 2008/03/03 07:28:32 ql Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Overview:
 *   This utility writes a binary image directly to a NAND flash
 *   chip or NAND chips contained in DoC devices. This is the
 *   "inverse operation" of nanddump.
 *
 * tglx: Major rewrite to handle bad blocks, write data with or without ECC
 *	 write oob data only on request
 *
 * Bug/ToDo:
 */

#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <getopt.h>

#include <asm/types.h>
#include <mtd/mtd-user.h>


#define PROGRAM "nandwrite"
#define VERSION "$Revision: 1.32 $"

#define MAX_PAGE_SIZE       4096
#define MAX_OOB_SIZE 128


static void display_help (void)
{
       printf(
"Usage: nandwrite [OPTION] MTD_DEVICE [INPUTFILE|-]\n"
	       "Writes to the specified MTD device.\n"
	       "\n"
"  -m, --markbad           Mark blocks bad if write fails\n"
"  -o, --oob               Image contains oob data\n"
"  -r, --raw               Image contains the raw oob data dumped by nanddump\n"
"  -s addr, --start=addr   Set start address (default is 0)\n"
"  -p, --pad               Pad to page size\n"
"  -b, --blockalign=1|2|4  Set multiple of eraseblocks to align to\n"
"  -q, --quiet             Don't display progress messages\n"
"      --help              Display this help and exit\n"
"      --version           Output version information and exit\n"
       );
       exit (EXIT_SUCCESS);
}

static void display_version (void)
{
	printf(PROGRAM " " VERSION "\n"
	       "\n"
	       "Copyright (C) 2003 Thomas Gleixner \n"
	       "\n"
	       PROGRAM " comes with NO WARRANTY\n"
	       "to the extent permitted by law.\n"
	       "\n"
	       "You may redistribute copies of " PROGRAM "\n"
	       "under the terms of the GNU General Public Licence.\n"
	       "See the file `COPYING' for more information.\n");
       exit (EXIT_SUCCESS);
}

static const char    *standard_input = "-";
static const char    *mtd_device, *img;
static int           mtdoffset = 0;
static bool          quiet = false;
static bool          writeoob = false;
static bool          rawoob = false;
static bool          markbad = false;
static bool          pad = false;
static int           blockalign = 1; /*default to using 16K block size */

static void process_options (int argc, char * const argv[])
{
	int error = 0;

	for (;;) {
		int option_index = 0;
        static const char *short_options = "b:mopqs:";
		static const struct option long_options[] = {
			{"help", no_argument, 0, 0},
			{"version", no_argument, 0, 0},
		   	{"blockalign", required_argument, 0, 'b'},
			{"markbad", no_argument, 0, 'm'},
			{"oob", no_argument, 0, 'o'},
			{"pad", no_argument, 0, 'p'},
			{"quiet", no_argument, 0, 'q'},
            {"raw", no_argument, 0, 'r'},
			{"start", required_argument, 0, 's'},
			{0, 0, 0, 0},
		};

		int c = getopt_long(argc, argv, short_options,
				    long_options, &option_index);
		if (c == EOF) {
			break;
		}

		switch (c) {
		case 0:
			switch (option_index) {
			case 0:
				display_help();
				break;
			case 1:
				display_version();
				break;
			}
			break;
		case 'q':
             quiet = true;
			break;
		case 'm':
			markbad = true;
			break;
		case 'o':
			writeoob = true;
			break;
		case 'p':
			pad = true;
			break;
		case 'r':
			rawoob = true;
			writeoob = true;
			break;
		case 's':
			mtdoffset = strtol (optarg, NULL, 0);
			break;
		case 'b':
			blockalign = atoi (optarg);
			break;
		case '?':
			error++;
			break;
		}
	}

       if (mtdoffset < 0) {
              fprintf(stderr, "Can't specify a negative device offset `%d'\n",
                            mtdoffset);
              exit (EXIT_FAILURE);
       }

       argc -= optind;
       argv += optind;

       /*
        * There must be at least the MTD device node positional
        * argument remaining and, optionally, the input file.
        */

       if (argc < 1 || argc > 2 || error)
		display_help ();

       mtd_device = argv[0];

       /*
        * Standard input may be specified either explictly as "-" or
        * implicity by simply omitting the second of the two
        * positional arguments.
        */

       img = ((argc == 2) ? argv[1] : standard_input);
}

static void erase_buffer(void *buffer, size_t size)
{
       const uint8_t kEraseByte = 0xff;

       if (buffer != NULL && size > 0) {
              memset(buffer, kEraseByte, size);
       }
}

/*
 * Main program
 */
int main(int argc, char * const argv[])
{
	int cnt = 0;
	int fd = -1;
	int ifd = -1;
	int imglen = 0, pagelen;
	bool baderaseblock = false;
	int blockstart = -1;
	struct mtd_info_user meminfo;
	struct mtd_oob_buf oob;
	loff_t offs;
	int ret;

	struct nand_ecclayout ecclayout;
	bool failed = true;
	// contains all the data read from the file so far for the current eraseblock
	unsigned char *filebuf = NULL;
	size_t filebuf_max = 0;
	size_t filebuf_len = 0;
	// points to the current page inside filebuf
	unsigned char *writebuf = NULL;
	// points to the OOB for the current page in filebuf
	unsigned char *oobreadbuf = NULL;
	unsigned char oobbuf[MAX_OOB_SIZE];

	process_options(argc, argv);

	erase_buffer(oobbuf, sizeof(oobbuf));

	if (pad && writeoob) {
		fprintf(stderr, "Can't pad when oob data is present.\n");
              exit (EXIT_FAILURE);
	}

	/* Open the device */
	if ((fd = open(mtd_device, O_RDWR)) == -1) {
              perror(mtd_device);
              exit (EXIT_FAILURE);
	}

	/* Fill in MTD device capability structure */
	if (ioctl(fd, MEMGETINFO, &meminfo) != 0) {
		perror("MEMGETINFO");
		close(fd);
              exit (EXIT_FAILURE);
	}

	/* Set erasesize to specified number of blocks - to match jffs2
	* (virtual) block size */
	meminfo.erasesize *= blockalign;

	/* Make sure device page sizes are valid */
	if (!(meminfo.oobsize == 16 && meminfo.writesize == 512) &&
		!(meminfo.oobsize == 8 && meminfo.writesize == 256) &&
		!(meminfo.oobsize == 64 && meminfo.writesize == 2048) &&
		!(meminfo.oobsize == 64 && meminfo.writesize == 4096) &&
		!(meminfo.oobsize == 128 && meminfo.writesize == 4096)) {
		fprintf(stderr, "Unknown flash (not normal NAND)\n");
		close(fd);
		exit (EXIT_FAILURE);
	}

	if (mtdoffset & (meminfo.writesize - 1)) {
		fprintf(stderr, "The start address is not page-aligned !\n"
                            "The pagesize of this NAND Flash is 0x%x.\n",
                            meminfo.writesize);
		close(fd);
		exit(EXIT_FAILURE);
	}
	   
	oob.length = meminfo.oobsize;
	oob.ptr =  oobbuf;

	if (ioctl(fd, ECCGETLAYOUT, &ecclayout) != 0) {
		perror("ECCGETLAYOUT");
		close(fd);
		exit(EXIT_FAILURE);
	}
	 
	/* Determine if we are reading from standard input or from a file. */
	if (strcmp(img, standard_input) == 0) {
		ifd = STDIN_FILENO;
	} else {
		ifd = open(img, O_RDONLY);
	}

	if (ifd == -1) {
		perror(img);
		goto restoreoob;
	}

	pagelen = meminfo.writesize + ((writeoob) ? meminfo.oobsize : 0);

       /*
        * For the standard input case, the input size is merely an
        * invariant placeholder and is set to the write page
        * size. Otherwise, just use the input file size.
        *
        * TODO: Add support for the -l,--length=length option (see
        * previous discussion by Tommi Airikka <tommi.airikka@ericsson.com> at
        * <http://lists.infradead.org/pipermail/linux-mtd/2008-September/
        * 022913.html>
        */

	if (ifd == STDIN_FILENO) {
		imglen = pagelen;
	} else {
		imglen = lseek(ifd, 0, SEEK_END);
		lseek (ifd, 0, SEEK_SET);
	}

       // Check, if file is page-aligned
	if ((!pad) && ((imglen % pagelen) != 0)) {
              fprintf (stderr, "Input file is not page-aligned. Use the padding "
                             "option.\n");
		goto closeall;
	}

	// Check, if length fits into device
	if ( ((imglen / pagelen) * meminfo.writesize) > (meminfo.size - mtdoffset)) {
		fprintf (stderr, "Image %d bytes, NAND page %d bytes, OOB area %u bytes, device size %u bytes\n",
                            imglen, pagelen, meminfo.writesize, meminfo.size);
		perror ("Input file does not fit into device");
		goto closeall;
	}

       // Allocate a buffer big enough to contain all the data (OOB included) for one eraseblock
	filebuf_max = pagelen * meminfo.erasesize / meminfo.writesize;
	filebuf = (unsigned char*)malloc(filebuf_max);
	if (!filebuf) {
		fprintf(stderr, "Failed to allocate memory for file buffer (%d bytes)\n",
                            pagelen * meminfo.erasesize / meminfo.writesize);
		goto closeall;
	}
	erase_buffer(filebuf, filebuf_max);

       /*
        * Get data from input and write to the device while there is
        * still input to read and we are still within the device
        * bounds. Note that in the case of standard input, the input
        * length is simply a quasi-boolean flag whose values are page
        * length or zero.
        */
	while (((imglen > 0) || (writebuf < (filebuf + filebuf_len)))
              && (mtdoffset < meminfo.size))
	{
		// new eraseblock , check for bad block(s)
		// Stay in the loop to be sure if the mtdoffset changes because
		// of a bad block, that the next block that will be written to
		// is also checked. Thus avoiding errors if the block(s) after the
		// skipped block(s) is also bad (number of blocks depending on
		// the blockalign
		while (blockstart != (mtdoffset & (~meminfo.erasesize + 1))) {
			blockstart = mtdoffset & (~meminfo.erasesize + 1);
			offs = blockstart;

			// if writebuf == filebuf, we are rewinding so we must not
			// reset the buffer but just replay it
			if (writebuf != filebuf) {
				erase_buffer(filebuf, filebuf_len);
				filebuf_len = 0;
				writebuf = filebuf;
			}

			baderaseblock = false;
			if (!quiet)
				fprintf (stdout, "Writing data to block %d at offset 0x%x\n",
					blockstart / meminfo.erasesize, blockstart);

			/* Check all the blocks in an erase block for bad blocks */
			do {
			   	if ((ret = ioctl(fd, MEMGETBADBLOCK, &offs)) < 0) {
					perror("ioctl(MEMGETBADBLOCK)");
					goto closeall;
				}
				if (ret == 1) {
					baderaseblock = true;
				   	if (!quiet)
						fprintf (stderr, "Bad block at %x, %u block(s) "
                                                        "from %x will be skipped\n",
                                                        (int) offs, blockalign, blockstart);
					}

				if (baderaseblock) {
					mtdoffset = blockstart + meminfo.erasesize;
				}
			        offs +=  meminfo.erasesize / blockalign ;
			} while ( offs < blockstart + meminfo.erasesize );

		}

		// Read more data from the input if there isn't enough in the buffer
		if ((writebuf + meminfo.writesize) > (filebuf + filebuf_len)) {
			int readlen = meminfo.writesize;

			int alreadyread = (filebuf + filebuf_len) - writebuf;
			int tinycnt = alreadyread;

			while (tinycnt < readlen) {
				cnt = read(ifd, writebuf + tinycnt, readlen - tinycnt);
				if (cnt == 0) { // EOF
					break;
				} else if (cnt < 0) {
					perror ("File I/O error on input");
					goto closeall;
				}
				tinycnt += cnt;
			}

			/* No padding needed - we are done */
			if (tinycnt == 0) {
			// For standard input, set the imglen to 0 to signal
			// the end of the "file". For non standard input, leave
			// it as-is to detect an early EOF
				if (ifd == STDIN_FILENO) {
					imglen = 0;
				}
				break;
			}

			/* Padding */
			if (tinycnt < readlen) {
				if (!pad) {
					fprintf(stderr, "Unexpected EOF. Expecting at least "
	                                                 "%d more bytes. Use the padding option.\n",
	                                                 readlen - tinycnt);
				goto closeall;
			}
			erase_buffer(writebuf + tinycnt, readlen - tinycnt);
		}

		filebuf_len += readlen - alreadyread;
		if (ifd != STDIN_FILENO) {
			imglen -= tinycnt - alreadyread;
		}else if (cnt == 0) {
			/* No more bytes - we are done after writing the remaining bytes */
			imglen = 0;
			}
		}

		if (writeoob) {
			int i, start, len;
			oobreadbuf = writebuf + meminfo.writesize;

			// Read more data for the OOB from the input if there isn't enough in the buffer
			if ((oobreadbuf + meminfo.oobsize) > (filebuf + filebuf_len)) {
				int readlen = meminfo.oobsize;

				int alreadyread = (filebuf + filebuf_len) - oobreadbuf;
				int tinycnt = alreadyread;

				while (tinycnt < readlen) {
					cnt = read(ifd, oobreadbuf + tinycnt, readlen - tinycnt);
					if (cnt == 0) { // EOF
						break;
					} else if (cnt < 0) {
						perror ("File I/O error on input");
						goto closeall;
					}
						tinycnt += cnt;
				}

				if (tinycnt < readlen) {
					fprintf(stderr, "Unexpected EOF. Expecting at least "
	                                                 "%d more bytes for OOB\n", readlen - tinycnt);
					goto closeall;
				}

				filebuf_len += readlen - alreadyread;
				if (ifd != STDIN_FILENO) {
					imglen -= tinycnt - alreadyread;
				}
	 			else if (cnt == 0) {
					/* No more bytes - we are done after writing the remaining bytes */
					imglen = 0;
				}
			}


			for (i = 0; ecclayout.oobfree[i].length; i++) {
				/* Set the reserved bytes to 0xff */
				start = ecclayout.oobfree[i].offset;
				len = ecclayout.oobfree[i].length;
				memcpy(oobbuf + start,
						oobreadbuf + start,
						len);
			}

			/* Write OOB data first, as ecc will be placed in there*/
			oob.start = mtdoffset;
			if (ioctl(fd, MEMWRITEOOB, &oob) != 0) {
				perror ("ioctl(MEMWRITEOOB)");
				goto closeall;
			}
		}

		/* Write out the Page data */
		if (pwrite(fd, writebuf, meminfo.writesize, mtdoffset) != meminfo.writesize) {
                     erase_info_t erase;

			if (errno != EIO) {
				perror ("pwrite");
				goto closeall;
			}

			/* Must rewind to blockstart if we can */
			writebuf = filebuf;

			erase.start = blockstart;
			erase.length = meminfo.erasesize;
			fprintf(stderr, "Erasing failed write from %08lx-%08lx\n",
	                            (long)erase.start, (long)erase.start+erase.length-1);
			if (ioctl(fd, MEMERASE, &erase) != 0) {
				int errno_tmp = errno;
				perror("MEMERASE");
				if (errno_tmp != EIO) {
					goto closeall;
				}
			}

			if (markbad) {
				loff_t bad_addr = mtdoffset & (~(meminfo.erasesize / blockalign) + 1);
				fprintf(stderr, "Marking block at %08lx bad\n", (long)bad_addr);
				if (ioctl(fd, MEMSETBADBLOCK, &bad_addr)) {
					perror("MEMSETBADBLOCK");
					goto closeall;
				}
			}
			mtdoffset = blockstart + meminfo.erasesize;

			continue;
			}
			mtdoffset += meminfo.writesize;
			writebuf += pagelen;
		}

       failed = false;

 closeall:
       if (filebuf) {
              free(filebuf);
       }

	close(ifd);

 restoreoob:
	close(fd);
       if (failed
              || ((ifd != STDIN_FILENO) && (imglen > 0))
              || (writebuf < (filebuf + filebuf_len)))
       {
              perror ("Data was only partially written due to error\n");
              exit (EXIT_FAILURE);
	}

	/* Return happy */
       return EXIT_SUCCESS;
}
