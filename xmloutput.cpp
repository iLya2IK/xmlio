/*
	xmloutput.cpp
	Utility class for outputting XML data files.

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

#include "xmloutput.h"
#include <stdio.h>
#include <assert.h>

#ifdef __GNUC__
size_t strlen (const char *str)
{
  const char *char_ptr;
  const unsigned long int *longword_ptr;
  unsigned long int longword, himagic, lomagic;

  /* Handle the first few characters by reading one character at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  for (char_ptr = str; ((unsigned long int) char_ptr
                        & (sizeof (longword) - 1)) != 0;
       ++char_ptr)
    if (*char_ptr == '\0')
      return char_ptr - str;

  /* All these elucidatory comments refer to 4-byte longwords,
     but the theory applies equally well to 8-byte longwords.  */

  longword_ptr = (unsigned long int *) char_ptr;

  /* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
     the "holes."  Note that there is a hole just to the left of
     each byte, with an extra at the end:

     bits:  01111110 11111110 11111110 11111111
     bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

     The 1-bits make sure that carries propagate to the next 0-bit.
     The 0-bits provide holes for carries to fall into.  */
  himagic = 0x80808080L;
  lomagic = 0x01010101L;
  if (sizeof (longword) > 4)
    {
      /* 64-bit version of the magic.  */
      /* Do the shift in two steps to avoid a warning if long has 32 bits.  */
      himagic = ((himagic << 16) << 16) | himagic;
      lomagic = ((lomagic << 16) << 16) | lomagic;
    }
  if (sizeof (longword) > 8)
    abort ();

  /* Instead of the traditional loop which tests each character,
     we will test a longword at a time.  The tricky part is testing
     if *any of the four* bytes in the longword in question are zero.  */
  for (;;)
    {
      longword = *longword_ptr++;

      if (((longword - lomagic) & ~longword & himagic) != 0)
        {
          /* Which of the bytes was the zero?  If none of them were, it was
             a misfire; continue the search.  */

          const char *cp = (const char *) (longword_ptr - 1);

          if (cp[0] == 0)
            return cp - str;
          if (cp[1] == 0)
            return cp - str + 1;
          if (cp[2] == 0)
            return cp - str + 2;
          if (cp[3] == 0)
            return cp - str + 3;
          if (sizeof (longword) > 4)
            {
              if (cp[4] == 0)
                return cp - str + 4;
              if (cp[5] == 0)
                return cp - str + 5;
              if (cp[6] == 0)
                return cp - str + 6;
              if (cp[7] == 0)
                return cp - str + 7;
            }
        }
    }
}
#endif // __GNUC__

XML_BEGIN_NAMESPACE

Output::Output(OutputStream &stream) : mStream(stream)
{
	mLevel = 0;
	mAttributes = false;
}

void Output::write(const char *str, size_t len)
{
	mStream.write(str, len);
}

void Output::writeString(const char *str)
{
	assert(str);
	write(str, strlen(str));
}

void Output::writeLine(const char *str)
{
	assert(str);
	write(str, strlen(str));
	write("\n", 1);
}

Output &Output::operator<<(const std::string &str)
{
	write(str.c_str(), str.size());
	return *this;
}

Output &Output::operator<<(const char *str)
{
	assert(str);
	writeString(str);
	return *this;
}

Output &Output::operator<<(int value)
{
	char tmp[50];
	sprintf(tmp, "%d", value);
	writeString(tmp);
	return *this;
}

Output &Output::operator<<(unsigned int value)
{
	char tmp[50];
	sprintf(tmp, "%d", value);
	writeString(tmp);
	return *this;
}

Output &Output::operator<<(double value)
{
	char tmp[50];
	sprintf(tmp, "%g", value);
	writeString(tmp);
	return *this;
}

Output &Output::operator<<(bool value)
{
	writeString(value ? "True" : "False");
	return *this;
}

void Output::BeginDocument(const char *version, const char *encoding, bool standalone)
{
	assert(version);
	assert(encoding);

	(*this) << "<?xml version=\"" << version << "\" encoding=\"" << encoding << "\"";
	(*this) << " standalone=\"" << (standalone ? "yes" : "no") << "\"?>\n";
}

void Output::EndDocument()
{
	assert(!mAttributes);
	assert(mElements.empty());
}

void Output::Indent()
{
	for (int i = 0; i < mLevel; i++)
		(*this) << "\t";
}

void Output::BeginElement(const char *name, Mode mode)
{
	assert(name);
	assert(!mAttributes);
	Indent();
	mLevel++;
	(*this) << "<" << name << ">";
	if (mode != terse)
		(*this) << "\n";

	mElements.push_back(name);
}

void Output::BeginElementAttrs(const char *name)
{
	assert(name);
	assert(!mAttributes);
	Indent();
	mLevel++;
	(*this) << "<" << name;
	mAttributes = true;

	mElements.push_back(name);
}

void Output::EndAttrs(Mode mode)
{
	assert(mAttributes);
	mAttributes = false;
	(*this) << ">";
	if (mode != terse)
		(*this) << "\n";
}

void Output::EndElement(Mode mode)
{
	assert(mElements.size() > 0);
	assert(!mAttributes);
	assert(mLevel > 0);
	--mLevel;

	if (mode != terse)
		Indent();

	const char *name = mElements.back();
	mElements.pop_back();

	(*this) << "</" << name << ">" << "\n";
}

void Output::WriteElement(const char *name, const std::string &value)
{
	assert(name);
	BeginElement(name, terse);
	(*this) << value;
	EndElement(terse);
}

void Output::WriteElement(const char *name, const char *value)
{
	assert(name);
	assert(value);
	BeginElement(name, terse);
	(*this) << value;
	EndElement(terse);
}

void Output::WriteElement(const char *name, int value)
{
	assert(name);
	BeginElement(name, terse);
	(*this) << value;
	EndElement(terse);
}

void Output::WriteElement(const char *name, unsigned int value)
{
	assert(name);
	BeginElement(name, terse);
	(*this) << value;
	EndElement(terse);
}

void Output::WriteElement(const char *name, double value)
{
	assert(name);
	BeginElement(name, terse);
	(*this) << value;
	EndElement(terse);
}

void Output::WriteElement(const char *name, bool value)
{
	assert(name);
	BeginElement(name, terse);
	(*this) << value;
	EndElement(terse);
}

void Output::WriteAttr(const char *name, const std::string &value)
{
	assert(mAttributes);
	assert(name);
	(*this) << " " << name << "=\"" << value << "\"";
}

void Output::WriteAttr(const char *name, const char *value)
{
	assert(mAttributes);
	assert(name);
	assert(value);

	(*this) << " " << name << "=\"" << value << "\"";
}

void Output::WriteAttr(const char *name, int value)
{
	assert(mAttributes);
	assert(name);

	(*this) << " " << name << "=\"" << value << "\"";
}

void Output::WriteAttr(const char *name, double value)
{
	assert(mAttributes);
	assert(name);

	(*this) << " " << name << "=\"" << value << "\"";
}

void Output::WriteAttr(const char *name, bool value)
{
	assert(mAttributes);
	assert(name);

	(*this) << " " << name << "=\"" << value << "\"";
}

XML_END_NAMESPACE

