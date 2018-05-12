/*
   FrameParser3
   Copyright (C) 2008,2009:
         Daniel Roggen, droggen@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
	This library provides basic functions to parse fixed-length frame-based data streams. 
	
	It is motivated by the need to decode data streams originating from USB, Bluetooth or serial sensor nodes.
	In such applications, the loss of data samples is not critical to the operation of the whole system, but it is
	important to keep a synchronized decoding of the data stream.
	The use of a frame-based data stream is a simple way to provide a synchronization mechanism.

	A frame-based data stream has the following format:

	<FRAMEHEADER><data0><data1><data2><data3>...(<CHECKSUM><FRAMEFOOTER>)

	With :
		FRAMEHEADER:	A set of characters delimiting the start of the frame.
		data0...n		A set of payload data, each of which may be in a different data format.
      CHECKSUM:		The frame checksum.
		FRAMEFOOTER:	Not implemented - optional - delimits frame end - The checksum or a longer header can play this role.

	The advantages of a frame-based data format are:
		- Frame-based data streams are well suited to devices continuously streaming data from power on.
		- Data can be continuously streamed over a (possibly lossy) communication channel.
		- The frame header identifies the data start: this allows to recover synchronization when the data sink connects to the data source, or if bytes are lost over the channel.
		- It avoids bidirectionnal communication, to e.g. order a "start of data streaming"
      - The complexity is limited, both on the sender and receiver, which makes it suitable for micro-controller systems.
		- The data format is flexible: char, short, int, long....

	What this library is not:
		- A way to reliably transport data over a lossy channel: corrupt frames are discarded, 
        possibly incorrect frames may be decoded - as in any streaming protocol.

	Notes:
      The frame header should not appear in the data payload to avoid synchronization errors (unless a checksum is used).
      A checksum  minimizes synchronization errors by checking frame consistency and rejecting corrupt frames, or frames including the header in the payload.
      A sufficiently long frame header should be selected so that the probability that the data payload is identical
      to the frame header is kept low. Alternatively a checksm can be employed.
      Usually, a 2 to 6 bytes frame header is sufficient for most applications.

      Note that even with a checksum, there is no guarantee that synchronization errors are avoided: their probability is simply (strongly) reduced.

   Class:

   This library provides the class FrameParser3.
   That class is instanciated with a format string as a parameter:
         FrameParser3(format)

      The 'format' string defines the frame header and data format as follows:
         format: "FRAMEHEADER;<data0><data1><data2>...[;checksum]"
		The frame header is delimited by a semicolon from the definition of the data types <data0><data1>....
		Data types have the following format:
         <datan>: [-]c¦s¦S¦i¦I¦b<num>¦B<num>
         -:		     indicates that the next item is signed; by default unsigned is assumed.
         c:		     8-bit character
         s:		     16-bit short, little endian
         S:		     16-bit short, big endian
         i:		     32-bit int, little endian
         I:		     32-bit int, big endian
         b<num>:    a num-bit long value, with the most significant bit first, and least significant byte first (little endian)
         B<num>:    a num-bit long value, with the most significant bit first, and most significant byte first (big endian)

      Note that endianness in format b and B is only valid when <num> is a multiple of 8 bits, in which case the format
      behaves as one of c, s, S, i, I.
      Little/Big endian is undefined

      Checksum has the following format:
         f/F:       16-bit Fletcher-16 (little/big endian)
         x:         8-bit checksum


      When data types are mixed between c/s/i and num-bit fields, the endiannes of c/s/i is respected although the data is not necessarily
      aligned on byte boundaries anymore.
      The checksum is an exception: it is always aligned on byte boundaries.


   std::vector<std::vector<int> > Parser(const char *data,int n);

      Parses a single synchronized frame 'stream' of format 'format' and returns the decoded
      data items in the variable argument list '...'.
      Assumption: synchronized frame, i.e. the frame header is at the start of the frame.

         format:	null-terminated string defining the frame-based data format
         stream:	synchronized data stream (i.e. the frame header is at the start of the stream)
         ...:		variable argument list - pointers to ints (int*) that receive the decoded data

		Returns:
			0:			No errors
			other:	Error

		Example:
*/
/*
History:
   1.3	2009-09-14	Added checksum, and custom field sizes
   1.2	2008-10-09	Converted to class.
	1.1	2007-11-29	Added vectorial version of parser, and many support functions.
	1.0	2007-11-28	First release.
*/



#ifndef __FRAMEPARSER3_H
#define __FRAMEPARSER3_H

#define FRAMEPARSER_NOERROR		0
#define FRAMEPARSER_ERROR_FORMAT	-1

#include <string>
#include <vector>

class FrameParser3
{
	private:
		std::string format;				// format string
		std::string frame;				// placeholder for incoming frame data
		std::string header;				// frame header
		std::vector<bool> sign;			// Sign of channel
		std::vector<bool> littleendian;	// Endianness of channel
      std::vector<int> bitpc;       // Bit per channel
      std::vector<int> bitpos;      // Bit per channel
      int framesize;                // Total frame size in bytes, incl. header, data and optional checksum
      int channel;                  // Number of channels
      int framesizebits;            // Frame size in bits
      bool checksum;                // Indicates whether a checksum is needed.
      int checksumtype;             // 0: 8-bit, 1: 16-bit.
      bool checksumlittle;          // true if the checksum is in little endian format
      int checksumbitpos;           // bit position of the checksum


      int FrameParser_AnalyzeFormat(int &channel,int &framesize,std::string &header,std::vector<bool> &sign,std::vector<bool> &littleendian,std::vector<int> &bitpc,std::vector<int> &bitpos);
		bool ok;

      unsigned int getbits(unsigned char *data,unsigned int firstbit,unsigned int n);
      unsigned int getbits_little(unsigned char *data,unsigned int firstbit,unsigned int n);

      unsigned short CheckSum(unsigned char *ptr,unsigned n);
      unsigned short fletcher16(unsigned char *data, int len );

      int GetFramesize();
      int GetFramesizeWithoutChecksum();

		
	public:
      FrameParser3(std::string _format);
      ~FrameParser3();
		
		void Status();
      unsigned int getbits(unsigned firstbit,unsigned n);
		std::vector<std::vector<int> > Parser(const char *data,int n);
		bool IsValid();
};



#endif



