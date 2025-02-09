/* Image manipulation functions
 *
 * Project : minidlna
 * Website : http://sourceforge.net/projects/minidlna/
 * Author  : Justin Maggard
 *
 * MiniDLNA media server
 * Copyright (C) 2009  Justin Maggard
 *
 * This file is part of MiniDLNA.
 *
 * MiniDLNA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * MiniDLNA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MiniDLNA. If not, see <http://www.gnu.org/licenses/>.
 */
#include <sys/types.h>

typedef u_int32_t pix;

typedef struct {
	int32_t width;
	int32_t height;
	pix     *buf;
} image;

void
image_free(image *pimage);

int
image_get_jpeg_date_xmp(const char * path, char ** date);

int
image_get_jpeg_resolution(const char * path, int * width, int * height);

image *
image_new_from_jpeg(const char * path, int is_file, const char * ptr, int size, int scale);

image *
image_resize(image * src_image, int32_t width, int32_t height);

unsigned char *
image_save_to_jpeg_buf(image * pimage, int * size);

int
image_save_to_jpeg_file(image * pimage, const char * path);
