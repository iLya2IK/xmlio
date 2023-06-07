/*
	xmlfile.c
	Sample implementation of InputStream and OutputStream that uses stdio.

	Copyright (c) 2000 Paul T. Miller

	LGPL DISCLAIMER
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA  02111-1307, USA.

	http://www.gnu.org/copyleft/lgpl.html
*/

#include "xmlfile.h"
#include <assert.h>
#include <errno.h>
#include <string.h>

typedef struct FileInputStream_
{
	XML_InputStream stream;
	FILE *file;
} FileInputStream;

typedef struct FileOutputStream_
{
	XML_OutputStream stream;
	FILE *file;
} FileOutputStream;

static int sReadProc(XML_InputStream *str, XML_Char *buf, size_t bufLen)
{
	FileInputStream *fstr = (FileInputStream *)str;
	assert(fstr);
	assert(buf);
	return fread(buf, sizeof(XML_Char), bufLen, fstr->file);
}

static int sWriteProc(XML_OutputStream *str, const XML_Char *buf, size_t bufLen)
{
	FileOutputStream *fstr = (FileOutputStream *)str;
	assert(fstr);
	assert(buf);
	return fwrite(buf, sizeof(XML_Char), bufLen, fstr->file);
}

XML_InputStream *XML_OpenFileInputStream(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (file)
	{
		FileInputStream *str = (FileInputStream *)malloc(sizeof(FileInputStream));
		if (str)
		{
			str->stream.readProc = sReadProc;
			str->file = file;
			return &str->stream;
		}
	}
	return NULL;
}

void XML_CloseFileInputStream(XML_InputStream *str)
{
	FileInputStream *fstr = (FileInputStream *)str;
	assert(fstr);
	fclose(fstr->file);
	free(fstr);
}

XML_OutputStream *XML_OpenFileOutputStream(const char *path)
{
	FILE *file = fopen(path, "wb");
	if (file)
	{
		FileOutputStream *str = (FileOutputStream *)malloc(sizeof(FileOutputStream));
		if (str)
		{
			str->stream.writeProc = sWriteProc;
			str->file = file;
			return &str->stream;
		}
	}
	return NULL;
}

void XML_CloseFileOutputStream(XML_OutputStream *str)
{
	FileOutputStream *fstr = (FileOutputStream *)str;
	assert(fstr);
	fclose(fstr->file);
	free(fstr);
}

