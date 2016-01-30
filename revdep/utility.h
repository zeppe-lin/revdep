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

#pragma once

#define new(T,N)     ((T*)xmalloc(sizeof(T)*(N)))
#define renew(P,T,N) ((T*)xrealloc((P),sizeof(T)*(N)))

typedef void (*parse_cb_t) (int record, int field, char *start, char *end);

enum
{
	LOG_BRIEF,
	LOG_INFO_1,
	LOG_INFO_2,
	LOG_ERROR,
	LOG_DEBUG
};

extern void *xmalloc(unsigned int bytes);
extern void *xrealloc(void *p, unsigned int bytes);
extern char *xstrdup(const char *in);
extern void logger(int level, const char *fmt, ...) __attribute__((format(printf,2,3)));
extern char *strtrim(char *s);
extern unsigned int string_crc32(const char *s);
extern int open_file_in_memory(const char *path, char **text, unsigned int *length);
extern void close_file_in_memory(char *text, unsigned int length);
extern int parse_file_in_memory(char *text, parse_cb_t cb);
extern int get_ld_for_file(const char *pkg, const char *path, const char **ld);
