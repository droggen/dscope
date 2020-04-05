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


#include <QtGlobal>
#ifndef Q_OS_ANDROID
#include <stdio.h>
#include <stdlib.h>
#endif
//#include <unistd.h>

#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "FrameParser3.h"


FrameParser3::FrameParser3(std::string _format)
{
    // Assign the string
    format=_format;

    // Check if the format is valid and get the number of channels and buffer length
   int r= FrameParser_AnalyzeFormat(channel,framesize,header,sign,littleendian,bitpc,bitpos);
    if(r==FRAMEPARSER_NOERROR)
    {
        ok=true;
        frame.resize(framesize,0);
    }
    else
    {
        ok=false;
    }

    Status();
}
FrameParser3::~FrameParser3()
{


}
void FrameParser3::Status()
{
   if(!ok)
   {
#ifndef Q_OS_ANDROID
      printf("Invalid format string\n");
#endif
      return;
   }
#ifndef Q_OS_ANDROID
   printf("FrameParser: Channel configuration\n");
   printf("----------------------------------\n");
    printf("ok: %d\n",(int)ok);
    printf("Channel: %d\n",channel);
    printf("Framesize: %d\n",framesize);
    printf("Header: '%s'\n",header.c_str());
   printf("Sign:         ");
    for(int i=0;i<sign.size();i++)
      printf("%c ",sign[i]?'s':'u');
    printf("\n");
    printf("LittleEndian: ");
    for(int i=0;i<littleendian.size();i++)
      printf("%c ",littleendian[i]?'y':'n');
    printf("\n");
   printf("Bitpc: ");
   for(int i=0;i<bitpc.size();i++)
      printf("%d ",(int)bitpc[i]);
   printf("\n");
   printf("Bitpos: ");
   for(int i=0;i<bitpos.size();i++)
      printf("%d ",(int)bitpos[i]);
   printf("\n");
   printf("Checksum: %s\n",checksum?"Yes":"No");
   if(checksum)
   {
      printf("   Checksum type: ");
      switch(checksumtype)
      {
         case 0:
            printf("8-bit checksum\n");
            break;
         case 1:
            printf("Fletcher 16\n");
            break;
         default:
            printf("N/A\n");
      }
      printf("   Checksum endianness: %s\n",checksumlittle?"Little":"Big");
      printf("   Checksum bit position: %d\n",checksumbitpos);
   }
#endif
}
/*
    Parse the data up to n bytes and decode frames as they are found.
    Returns the number of frames decoded.
*/
std::vector<std::vector<int> > FrameParser3::Parser(const char *data,int n)
{
    unsigned int dataunsigned;
    std::vector<std::vector<int> > v;
    std::vector<int> channels(channel,0);
    if(!ok)
        return v;


    for(int i=0;i<n;i++)
    {
        // Feed in a byte.
        char *framedata = (char*)frame.data();
        memmove(framedata,framedata+1,framesize-1);
        framedata[framesize-1]=data[i];

        if(strncmp(framedata,header.data(),header.size()) == 0)
        {
            //printf("match\n");
         // Found a frame, get pointer to data
         unsigned char *dataptr = (unsigned char*)framedata+header.size();

         // Test the optional checksum
         if(checksum)
         {
            unsigned int check,payloadcheck;
            switch(checksumtype)
            {
               case 0:
                  check=CheckSum((unsigned char*)framedata,GetFramesizeWithoutChecksum());
                  payloadcheck=getbits(dataptr,checksumbitpos,8);
                  break;
               case 1:
                  check=fletcher16((unsigned char*)framedata,GetFramesizeWithoutChecksum());
                  if(checksumlittle)
                     payloadcheck=getbits_little(dataptr,checksumbitpos,16);
                  else
                     payloadcheck=getbits(dataptr,checksumbitpos,16);
                  break;
               default:
                  check=0;
                  assert(0);
            }
            /*if(payloadcheck!=check)
            {
               printf("Computing checksum on %d bytes\n",GetFramesizeWithoutChecksum());
               printf("payloadchecksum: %d\n",payloadcheck);
               printf("Computed check: %d\n",check);
            }*/
            if(payloadcheck!=check)
               continue;
         }



         // Decode all the channels
            for(int c=0;c<channel;c++)
            {
            // Get the value
            if(littleendian[c])
               dataunsigned = getbits_little(dataptr,bitpos[c],bitpc[c]);
            else
               dataunsigned = getbits(dataptr,bitpos[c],bitpc[c]);


            // Sign extension
            if(sign[c])
            {
               // If the left-most bit is 1, sign-extend
               if( dataunsigned & (1<<(bitpc[c]-1)) )
               {
                  unsigned int signext=0xFFFFFFFF;
                  for(int i=0;i<bitpc[c];i++)
                     signext<<=1;
                  dataunsigned |= signext;
               }
            }
                channels[c]=dataunsigned;
            } // for channel
            v.push_back(channels);
        } // if header

    } // for input data
    return v;
}

bool FrameParser3::IsValid()
{
    return ok;
}

int FrameParser3::FrameParser_AnalyzeFormat(int &channel,int &framesize,std::string &header,std::vector<bool> &sign,std::vector<bool> &littleendian,std::vector<int> &bitpc,std::vector<int> &bitpos)
{
    unsigned frameheaderlen;
    char type;
    unsigned formatptr;
   unsigned bitptr=0;




   // Find frame header length
    size_t t = format.find_first_of(';');
    if(t==std::string::npos)
        return FRAMEPARSER_ERROR_FORMAT;
    if(t==0)
    {
        //printf("No header - match always\n");
    }


   // Look for the separator for the checksum (optional)
   size_t t2 = format.find_last_of(';');
   if(t2==std::string::npos)
      return FRAMEPARSER_ERROR_FORMAT;
   if(t2==t)   // Only one delimiter, thus no checksum
   {
      framesizebits=0;
      checksum=false;
   }
   else
   {
      checksum=true;
      if(t2+1>=format.size())
         return FRAMEPARSER_ERROR_FORMAT;
      // look for the type of checksum
      char c=format[t2+1];
      switch(c)
      {
         case 'x':
            checksumtype=0;
            framesizebits=8;
            //checksumlittle=false;
            checksumlittle=true;
            break;
         case 'f':
         case 'F':
            checksumtype=1;
            framesizebits=16;
            checksumlittle=(c=='f')?true:false;
            break;
         default:
            return FRAMEPARSER_ERROR_FORMAT;
      }

   }

    // keep the header
    header=format.substr(0,t);

    frameheaderlen=t;

    // Decode the format string
    formatptr=frameheaderlen+1;
    channel=0;
   framesizebits += frameheaderlen*8;
    while(1)
    {
        // Get next data type
      type = format[formatptr++];
      if(type==0 || type==';')      // Last item processed, defined by end of string or ; separator -> return.
         break;
        if(type=='-')
        {
            sign.push_back(true);
            // Signed element. We have to read the data type.
         type=format[formatptr++];
            if(type==0)
            {
                assert(0);
                return FRAMEPARSER_ERROR_FORMAT;		// Error: we have a sign, but no type specifier -> crash.
            }
        }
        else
            sign.push_back(false);


      bitpos.push_back(bitptr);
      switch(type)
      {
         case 'b':
         case 'B':
            // User-define bit field, Parse the format number
            unsigned startnum,bits;
            startnum=formatptr;
            for(;format[formatptr]>='0' && format[formatptr]<='9';formatptr++);
            // The number is comprised between [startnum;formatptr[
            bits=atoi(format.substr(startnum,formatptr-startnum).c_str());
            if(bits>32)
               return FRAMEPARSER_ERROR_FORMAT;
            if(type=='b')
               littleendian.push_back(true);
            else
               littleendian.push_back(false);
            bitpc.push_back(bits);
            framesizebits+=bits;
            break;
         case 'c':
            littleendian.push_back(true);
            bitpc.push_back(8);
            framesizebits+=8;
            break;
         case 's':
            littleendian.push_back(true);
            bitpc.push_back(16);
            framesizebits+=16;
            break;
         case 'S':
            littleendian.push_back(false);
            bitpc.push_back(16);
            framesizebits+=16;
            break;
         case 'i':
            littleendian.push_back(true);
            bitpc.push_back(32);
            framesizebits+=32;
            break;
         case 'I':
            littleendian.push_back(false);
            bitpc.push_back(32);
            framesizebits+=32;
            break;
         default:
            printf("Invalid format spec!\n");
            //assert(0);
            printf("Invalid format spec! after assert\n");
            return FRAMEPARSER_ERROR_FORMAT;
      }
      bitptr += *(bitpc.end()-1);
      //formatptr++;
      channel++;
    }	// do

   // Define location of checksum,
   checksumbitpos=(bitptr/8)*8;
   if(bitptr&0x7)
      checksumbitpos+=8;


   // Round framesize to upper byte.
   framesize=framesizebits/8;
   if(framesizebits&0x07)
      framesize++;



    return FRAMEPARSER_NOERROR;
}





/*
  Returns n bits from data, starting at firstbit.

  Bit/byte numbering:
  b0 b1 b2 b3 b4 b5 b6 b7   b8 b9 b10 b11 b12 b13 b14 b15   b16 b17 b18 b19 b20 b21 b22 b23
  -----------------------   -----------------------------   -------------------------------
            B0                           B1                                B2

  firstbit=0: bit 7 of byte 0 (b0)
  firstbit=7: bit 0 of byte 0 (b7)
  firstbit=8: bit 7 of byte 1 (b8)

   The format is MSB first (big endian).

*/
unsigned int FrameParser3::getbits(unsigned char *data,unsigned int firstbit,unsigned int n)
{
   unsigned int byteptr;
   unsigned int bitptr;
   unsigned int result;
   unsigned int bit;

   result=0;

   byteptr=firstbit>>3;           // firstbit is located in data[byteptr]
   bitptr=7-(firstbit&0x07);      // firstbit is bit number bitptr
   //printf("byteptr/bitptr: %d %d, data at byte: %X\n",byteptr,bitptr,data[byteptr]&0xff);

   for(unsigned int i=0;i<n;i++)
   {
      result<<=1;
      bit = (data[byteptr]&(1<<bitptr))?1:0;
      result|=bit;
      if(bitptr==0)
         byteptr++;
      bitptr=(bitptr-1)&0x07;
   }
   return result;
}
/*
  Returns n bits from data, starting at firstbit, with little-endian ordering.



   The format is LSB first (little endian).

*/
unsigned int FrameParser3::getbits_little(unsigned char *data,unsigned int firstbit,unsigned int n)
{
   unsigned int byteptr;      // bype pointer in the input stream
   unsigned int bitptr;       // bit pointer in the input stream
   unsigned int bitpos;       // bit position to set in the result.
   unsigned int result;
   unsigned int bit;

   result=0;

   byteptr=firstbit>>3;           // firstbit is located in data[byteptr]
   bitptr=7-(firstbit&0x07);      // firstbit is bit number bitptr
   //printf("byteptr/bitptr: %d %d, data at byte: %X\n",byteptr,bitptr,data[byteptr]&0xff);

   if(n<8)                       // position at which to put the bit in the results
      bitpos=n-1;
   else
     bitpos=7;

   for(unsigned int i=0;i<n;i++)
   {
      bit = (data[byteptr]&(1<<bitptr))?1:0;    // read the bit from the input stream
      result |= (bit<<bitpos);                  // place the bit in the output result at the right position
      if(bitptr==0)
         byteptr++;
      bitptr=(bitptr-1)&0x07;
      if((i&0x07)==0x07)
      {
         bitpos=8+i;                            // little endian: point to next byte in the result
         if(bitpos>=n)
            bitpos=n-1;
      }
      else
         bitpos--;
   }
   return result;
}

unsigned short FrameParser3::CheckSum(unsigned char *ptr,unsigned n)
{
   unsigned i;
   unsigned char lrc;

   lrc=0;
   for(i=0;i<n;i++)
   {
      lrc=lrc^ptr[i];
   }
   return lrc;
}

unsigned short FrameParser3::fletcher16(unsigned char *data, int len )
{
   unsigned short sum1,sum2;
   unsigned short check;
   sum1 = 0xff;
   sum2 = 0xff;

   while (len)
   {
      int tlen = len > 21 ? 21 : len;
      len -= tlen;
      do
      {
         sum1 += *data++;
         sum2 += sum1;
      }
      while (--tlen);
      sum1 = (sum1 & 0xff) + (sum1 >> 8);
      sum2 = (sum2 & 0xff) + (sum2 >> 8);
   }
   /* Second reduction step to reduce sums to 8 bits */
   sum1 = (sum1 & 0xff) + (sum1 >> 8);
   sum2 = (sum2 & 0xff) + (sum2 >> 8);


   check=sum1<<8|sum2;

   return check;
}
int FrameParser3::GetFramesize()
{
   return framesize;
}
int FrameParser3::GetFramesizeWithoutChecksum()
{
   if(!checksum)
      return framesize;
   switch(checksumtype)
   {
      case 0:
         return framesize-1;
      case 1:
         return framesize-2;
      default:
         assert(0);
   }
   return 0;
}





