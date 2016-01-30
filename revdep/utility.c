// Copyright (C) 2015 James Buren
//
// This file is part of revdep.
//
// revdep is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// revdep is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with revdep.  If not, see <http://www.gnu.org/licenses/>.

#define _IN_UTILITY_
#include "utility.h"
#include "global.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <gelf.h>
#include <zlib.h>

extern void *xmalloc(unsigned int bytes)
{
	void *p;

	if(bytes == 0)
	{
		logger(LOG_BRIEF, "%s: bytes is zero\n", __func__);
		abort();
	}
	
	if((p = malloc(bytes)) == NULL)
	{
		logger(LOG_BRIEF, "%s: out of memory\n", __func__);
		abort();
	}

	return p;
}

extern void *xrealloc(void *p, unsigned int bytes)
{
	if(p == NULL)
	{
		logger(LOG_BRIEF, "%s: p is NULL\n", __func__);
		abort();
	}

	if(bytes == 0)
	{
		logger(LOG_BRIEF, "%s: bytes is zero\n", __func__);
		abort();
	}

	if((p = realloc(p, bytes)) == NULL)
	{
		logger(LOG_BRIEF, "%s: out of memory\n", __func__);
		abort();
	}

	return p;
}

extern char *xstrdup(const char *in)
{
	char *out;

	if(in == NULL)
	{
		logger(LOG_BRIEF, "%s: s is NULL\n", __func__);
		abort();
	}

	if((out = strdup(in)) == NULL)
	{
		logger(LOG_BRIEF, "%s: out of memory\n", __func__);
		abort();
	}

	return out;
}

extern void logger(int level, const char *fmt, ...)
{
	va_list args;

	if(level > rd.verbose)
		return;

	va_start(args, fmt);

	vfprintf(stdout, fmt, args);

	va_end(args);
}

extern char *strtrim(char *s)
{
	static const char SPACE[] = " \t\r\n\v\f";
	unsigned int left;
	unsigned int middle;
	//unsigned int right;

	left = strspn(s, SPACE);

	middle = strcspn(s + left, SPACE);

	//right = strspn(s + left + middle, SPACE);

	memmove(s, s + left, middle);

	s[middle] = '\0';

	return s;
}

extern unsigned int string_crc32(const char *s)
{
	unsigned int hash;

	if(s == NULL || s[0] == '\0')
	{
		errno = EINVAL;
		return -1;
	}

	hash = crc32(0, Z_NULL, 0);

	hash = crc32(hash, (Bytef *) s, strlen(s) + 1);

	return hash;
}

extern int open_file_in_memory(const char *path, char **text, unsigned int *length)
{
	struct stat st;
	int fd;

	if(path == NULL || path[0] == '\0' || text == NULL || length == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if((fd = open(path, O_RDONLY)) == -1)
	{
		return -1;
	}

	if(fstat(fd, &st) == -1)
	{
		close(fd);
		return -1;
	}

	if((text[0] = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == NULL)
	{
		close(fd);
		return -1;
	}

	// We don't need to keep the fd around.
	close(fd);

	length[0] = st.st_size;
	
	return 0;
}

extern void close_file_in_memory(char *text, unsigned int length)
{
	if(text == NULL || length == 0)
		return;

	munmap(text, length);
}

extern int parse_file_in_memory(char *text, parse_cb_t cb)
{
	static const char RS[] = "\n\n";
	static const char FS[] = "\n";
	char *r_start;
	char *r_end;
	char *f_start;
	char *f_end;
	int record;
	int field;

	if(text == NULL || text[0] == '\0' || cb == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	inline int next_record(void)
	{
		// Test for EOF.
		// Return EOF if so.
		if(r_start[0] == '\0')
			return -1;

		// Test for RS.
		// Skip RS if so.
		if(strncmp(r_start, RS, strlen(RS)) == 0)
			r_start += strlen(RS);

		// Find the RS.
		// If RS is not found, then use the end of the file.
		if((r_end = strstr(r_start, RS)) == NULL)
			r_end = r_start + strlen(r_start);

		return 0;
	}

	inline int next_field(void)
	{
		// Test for RS and EOF.
		// Return EOF if either is found.
		if(strncmp(f_start, RS, strlen(RS)) == 0 || strcmp(f_start, FS) == 0 || f_start[0] == '\0')
			return -1;

		// Test for FS.
		// Skip FS if so.
		if(f_start[0] == FS[0])
			f_start += 1;

		// Find the FS.
		// If FS is not found, then use the end of the file.
		if((f_end = strchr(f_start, FS[0])) == NULL)
			f_end = f_start + strlen(f_start);

		return 0;
	}

	for( r_start = text, record = 0 ; next_record() == 0 ; r_start = r_end, ++record )
	{
		for( f_start = r_start, field = 0 ; next_field() == 0 ; f_start = f_end, ++field )
		{
			cb(record, field, f_start, f_end);
		}
	}

	return 0;
}

extern int get_ld_for_file(const char *pkg, const char *path, const char **ld)
{
	struct stat st;
	int fd = -1;
	Elf *elf = NULL;
	GElf_Ehdr ehdr;
	size_t i;
	size_t phdrnum;
	GElf_Phdr phdr;
	int rv = -1;

	if(pkg == NULL || pkg[0] == '\0' || path == NULL || path[0] == '\0' || ld == NULL)
	{
		errno = EINVAL;
		goto bail;
	}

	if(lstat(path, &st) == -1)
	{
		logger(LOG_ERROR, "%s:%s: could not stat file\n", pkg, path);
		goto bail;
	}

	if(!S_ISREG(st.st_mode))
	{
		goto bail;
	}

	if((fd = open(path, O_RDONLY)) == -1)
	{
		logger(LOG_ERROR, "%s:%s: could not open file\n", pkg, path);
		goto bail;
	}

	if((elf = elf_begin(fd, ELF_C_READ_MMAP, NULL)) == NULL)
	{
		goto bail;
	}

	if(elf_kind(elf) != ELF_K_ELF)
	{
		goto bail;
	}

	if(gelf_getehdr(elf, &ehdr) == NULL)
	{
		goto bail;
	}

	// Check if file is executable or shared library
	switch(ehdr.e_type)
	{
		case ET_EXEC:
			break;

		case ET_DYN:
			break;

		default:
			goto bail;
	}

	// Check if file has SYSTEMV or LINUX ABI
	switch(ehdr.e_ident[EI_OSABI])
	{
		case ELFOSABI_NONE:
			break;

		case ELFOSABI_LINUX:
			break;

		default:
			goto bail;
	}

	// Check if file is supported by this host's architecture
	switch(ehdr.e_machine)
	{

#if defined(__i386__) || defined(__x86_64__)
		case EM_386:
			break;
#endif

#if defined(__x86_64__)
		case EM_X86_64:
			break;
#endif

#if defined(__arm__)
		case EM_ARM:
			break;
#endif

		default:
			goto bail;

	}

	// Check if file has a dynamic section
	if(elf_getphdrnum(elf, &phdrnum) == -1)
	{
		goto bail;
	}

	for( i = 0 ; i < phdrnum ; ++i )
	{
		if(gelf_getphdr(elf, i, &phdr) != NULL && phdr.p_type == PT_DYNAMIC)
		{
			break;
		}
	}

	// If true, then a dynamic section was not found.
	if(i == phdrnum)
	{
		goto bail;
	}

	// Check if file has a known LD binary
	switch(ehdr.e_ident[EI_CLASS])
	{

#if defined(__i386__)
		case ELFCLASS32:
			ld[0] = "/lib/ld-linux.so.2";
			break;
#elif defined(__x86_64__)
		case ELFCLASS32:
			ld[0] = "/lib32/ld-linux.so.2";
			break;
		case ELFCLASS64:
			ld[0] = "/lib/ld-linux-x86-64.so.2";
			break;
#elif defined(__arm__)
		case ELFCLASS32:
			ld[0] = "/lib/ld-linux-armhf.so.3";
			break;
#else
#error "Unsupported Architecture"
#endif

		default:
			goto bail;

	}

	rv = 0;

bail:

	if(elf != NULL)
		elf_end(elf);

	if(fd != -1)
		close(fd);

	return rv;
}
