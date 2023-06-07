---------------------------------------------------------------
XMLIO: a C++ XML input/output library

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

---------------------------------------------------------------

Modification History

v0.1	PTM		December 11, 1999
	Basic support for ASCII XML callback-based parsing of hierarchical XML data

v0.2	PTM		December 16, 1999
	Added intrinsic-data-type handlers Int, UnsignedInt, Float, Double, Bool, CString, List

v0.3	PTM		December 19, 1999
	Reimplemented over expat using an event-queue model

v0.4	PTM		December 22, 1999
	Removed expat and went back to a lighter-weight implementation

v0.5	PTM		January 1, 2000
	Reimplemented in C with the C++ API layered over it
	Renamed to XML::Input
	Added handler chaining
	Switched to LPGL
	Added C version of sample objects and read test
	Added bool and list tests
	Moved sample code to subdirectory

v0.6	PTM		January 8, 2000
	Added data (push access), comment, and CDATA handlers
	Added dump.c which copies one XML file to an output file and times it
	Added convert.cpp which uses XML::Input and XML::Output to mirror a file

v0.7	PTM		January 14, 2000
	Fixed a bug which prevented empty elements from working
	Added accessors to determine if an element is empty
	Skip DOCTYPE chunks

v0.8	PTM		March 2, 2000
	Fixed a bug in Data handlers causing the handler to be called too often
	Support single-quoted attributes (sorry about that)
	Removed anonymous struct in Element union - should make other compilers happy

v0.9	PTM		October 3, 2000
	Removed anonymous union which kept it from compiling with GCC (oops).

v0.91	PTM		October 25, 2000
	Fixed warnings about forward-declared structure typedefs

v0.92	PTM		January 8, 2001
	Incorporated compiler warning fixes by Matthew Wall <mwall@domesolutions.com>
	I want to clean up the std::vector and std::string stuff better for compilers
	that have namespaces disabled, but take a look at Matthew's changes to
	xmloutput in the contrib directory if you need to.

---------------------------------------------------------------

XMLIO is a package to help read and write application data stored
in XML format from within C or C++ applications.

There are two main components:

XML::Output - functions for writing nested elements and attributes
XML::Input - nestable, streaming XML processor

BACKGROUND
I wrote this library because I wanted to store application data 
using a well-documented syntax (why invent my own when one already
exists?), but I also wanted to make it trivial to generate and
parse these files. Existing XML parsers tend to fall into two camps:
	event parsers
	DOM parsers
Event parsers are too general purpose for what I wanted, and DOM
parsers have to load the entire document into memory. What I needed
was a streamable, event-based parser that allowed DOM-like processing
of sub-trees in a nestable fasion.

The interface to the parser is based somewhat on a parser for a 
proprietary hierarchical data-file syntax I created several years ago.
It works well with object hierarchies and allows encapsulating the
parsing of element subtrees by individual objects. Like an event parser,
objects can register handlers for various elements, but it is trivial
for one object to parse sub-objects, each parsing their own sub-objects
with their own element handlers. Handlers for intrinsic data-types such
as integers, booleans, floats, strings, and lists are also provided.
Furthermore, a subclass can easily attach its own list of handlers to a 
chain of handlers defined by its parent class.

USAGE
The easiest way to see how the parser works is to look at a sample
application. I've written a set of test "objects" that can read and
write their own data. The implementation can be found in sample.cpp
and sample.h. A vanilla C implementation of the equivalent functionality
can be found in sample_c.c and sample_c.h.

There are two sets of driver applications. testwrite.cpp (and the C version,
testwrite_c.c) build an in-memory object model and then write the data
to an XML file. testread.cpp (and the C version, testread_c.c) will
read the XML file back in, constructing the in-memory object model using
various element handlers.

API documentation can be found in xmlinput.h and xmloutput.h. 

HANDLERS
There are several types of built-in handlers. The C API uses macros to
initialize the handlers (which contain unions), and generally uses object
member offset and size information to store data for basic types. The C++
API uses XML::Handler() objects with proper type-checking, and can use 
pointers to destination addresses in addition to the object offset/size
information.

* Element
	This handler has two functions:
	1. to look for a specific element and call a user-supplied callback function
	2. to call a user-supplied callback function for any unhandled element

	This is the most common handler for dealing with nested objects. If userData
	is specified in the handler, that data will be passed to the callback. Otherwise,
	the userData passed to the element processor function will be used.

	C interface
		XML_ELEMENT_HANDLER_MEMBER(name, callback, object, member)
		XML_ELEMENT_HANDLER(name, callback, userData)

		name is the element name, or NULL to handle any element
		callback is the function to call of type XML_HandlerProc (see xmlinput.h)
		object, member is the struct and member name of the address
			you want passed as userData to the callback - this is useful
			if you want to branch to a handler for a child structure (see
			Point_Read and relevant code in sample_c.c). Note that you need
			to pass the address of the struct containing the child structure
			in the element process function.
		userData is the userData to pass to the callback, if it is static

	C++ interface
		XML::Handler(name, callback, userData)
		XML::Handler(name, callback, object, member)
		XML::Handler(callback, userData)
		XML::Handler(callback, object, member)

		name is the element name, and is required (use the alternate form that
			does not require a name for a generic handler)
		callback is the function to call of type XML::HandlerProc (see xmlinput.h)
		object, member is the class and member name of the address
			you want passed as userData to the callback - this is useful
			if you want to branch to a handler for a child class
		userData is the userData to pass to the callback - note that this can
			be the address of a class member if the handler list is not initialized
			statically. If the handler list is static, the object/member interface
			should be used

	While inside the handler callback function, a new list of handlers can be
	processed, or custom character data can be read, but not both.

* Int
	Parse an integer from the element data and store it at the specified address,
	with optional range clamping

	C interface
		XML_INT_HANDLER(name, object, member, minVal, maxVal)

	C++ interface
		XML::Handler(name, value, minVal=0, maxVal=0)

		value is the int ADDRESS to place the result

	If minVal OR maxVal is not zero, then range clamping is performed on the
	parsed value.

* UInt
	Parse an unsigned integer from the element data and store it at the specified
	address, with optional range clamping

	C interface
		XML_UINT_HANDLER(name, object, member, minVal, maxVal)

	C++ interface
		XML::Handler(name, value, minVal=0, maxVal=0)

		value is the unsigned int ADDRESS to place the result

	If minVal OR maxVal is not zero, then range clamping is performed on the
	parsed value.

* Float
	Parse a float from the element data and store it at the specified address,
	with optional range clamping

	C interface
		XML_FLOAT_HANDLER(name, object, member, minVal, maxVal)

		object/member should be sizeof(float) bytes

	C++ interface
		XML::Handler(name, value, minVal=0, maxVal=0)

		value is the float ADDRESS to place the result

	If minVal OR maxVal is not zero, then range clamping is performed on the
	parsed value.

* Double
	Parse a double from the element data and store it at the specified address,
	with optional range clamping

	C interface
		XML_DOUBLE_HANDLER(name, object, member, minVal, maxVal)

		object/member should be sizeof(double) bytes

	C++ interface
		XML::Handler(name, value, minVal=NULL, maxVal=NULL)

		value is the double ADDRESS to place the result

	NOTE: minVal and maxVal are POINTERS to the minimum and maximum double
	values to use.
	If minVal is not NULL, then clamping will be performed on the lower bound.
	If maxVal is not NULL, then clamping will be performed on the upper bound.

* Bool
	Parse a boolean value from the element data and store it at the specified address.
	The element data should be either "True" or "False" (without quotes). If
	neither is found, then False is assumed.

	C interface
		XML_BOOL_HANDLER(name, object, member)

		object/member should be sizeof(char), sizeof(short), or sizeof(int)

	C++ interface
		XML::Handler(name, value)

		value is the bool ADDRESS to place the result

* CString
	Parse a string from the element data, up to the ending element tag. No brackets
	can be found in the string, unless they are escaped. The string is always
	NULL-terminated

	C interface
		XML_STRING_HANDLER(name, object, member, maxLen)

		object/member should point to a string containing at least maxLen characters
		(sizeof(XML_Char) bytes in size)

	C++ interface
		XML::Handler(name, value, maxLen)

		value is the address of a string containing at least maxLen characters
		(sizeof(XML::Char) bytes in size)

* List
	Parse a string from the element data and compare against an array of pointers to 
	NULL-terminated strings specified in the handler data, storing the resulting index
	at the specified location

	C interface
		XML_LIST_HANDLER(name, object, member, list, listSize)

		object/member should be sizeof(char), sizeof(short), or sizeof(int)
		list is a pointer to an array of pointers to NULL-terminated strings
		listSize is the number of strings in the array

	C++ interface
		XML::Handler(name, value, list, listSize)

		value is the int ADDRESS to place the result
		list and listSize are the same for the C interface

	If no match is found, no value is stored and no error is raised.

* Chain
	Hook another handler list into this list, allowing handler lists to be
	chained together up an object hierarchy.

	C interface
		XML_CHAIN_HANDLER(handlers, userData)
		XML_CHAIN_HANDLER_MEMBER(handlers, object, member)

		handlers is a const array of XML_Handler structures
		object, member is the struct and member name of the address
			you want passed as the base address for the chained handlers
		userData is the base address to use for the chained handlers			

	C++ interface
		XML::Handler(handlers, userData)

		handlers is a const array of XML::Handler objects
		userData is the base address to use for the chained handlers

	Passing the correct base address to use for the chained handlers is
	very important. For example, if a struct contains a member structure
	for which there is a handler chain available to process, the address
	of the member structure should be passed as the userData in the
	handler specification for the chain. If the chain is for the members
	of a base class (with the same starting address as the current object),
	then NULL can be passed and the userData passed to the process function
	will be used (which is usually the object base address).

* Data
	This handler gets called with the element data as a push-type 
	interface. The prototype of the handler function is:

	XML_Error DataProc(const XML_Char *data, size_t len, void *userData)

	Since this function will be called with "chunks" of data, len will be 0 
	after all of the data has been sent. This provides a way to clean up 
	once all the data has been received.

* CData
	This handler gets called with CDATA as a push-type interface. The
	operation is the same as with a normal Data handler.

* Comment
	This handler gets called with comments as a push-type interface. The
	operation is the same as with a normal Data handler.


IMPLEMENTATION
XML::Input allocates no memory while processing. This is
the primary reason I did not implement it over expat or another 
parser. Because of this, reading and processing XML data is extremely
time and memory efficient.

The C++ implementation is built on top of the C implementation, with three
major differences:

1. the API is in the XML namespace instead of using the XML_ prefix
2. element handlers take a "XML::Element &" instead of a "XML_Element *"
3. exceptions are used instead of error codes


BUGS
* I am sure there are bugs, but it "works for me" as it is now.
* only ASCII encoding right now
* no namespaces
* no PI handlers

AUTHOR
Send me questions, comments, and bug reports at: paul@fxtech.com

