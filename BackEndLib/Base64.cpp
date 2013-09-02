// $Id: Base64.cpp 8019 2007-07-14 22:30:11Z trick $


//*********************************************************************
//* Base64 - a simple base64 encoder and decoder.
//*
//*     Copyright (c) 1999, Bob Withers - bwit@pobox.com
//*
//* This code may be freely used for any purpose, either personal
//* or commercial, provided the authors copyright notice remains
//* intact.
//*
//* Enhancements by Stanley Yamane:
//*     o reverse lookup table for the decode function 
//*     o reserve string buffer space in advance
//*
//* Contributors:
//* Mike Rimer (mrimer) -- added two encode and two decode methods
//*                           for other types
//*
//*
//*********************************************************************

#include "Base64.h"

using namespace std;

namespace {
	const char          fillchar = '=';
	const string::size_type np = string::npos;

								  // 0000000000111111111122222222223333333333444444444455555555556666
								  // 0123456789012345678901234567890123456789012345678901234567890123
	const string Base64Table("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

	// Decode Table gives the index of any valid base64 character in the Base64 table]
	// 65 == A, 97 == a, 48 == 0, 43 == +, 47 == /

															  // 0  1  2  3  4  5  6  7  8  9 
	const string::size_type DecodeTable[] = {     np,np,np,np,np,np,np,np,np,np,  // 0 - 9
																 np,np,np,np,np,np,np,np,np,np,  //10 -19
																 np,np,np,np,np,np,np,np,np,np,  //20 -29
																 np,np,np,np,np,np,np,np,np,np,  //30 -39
																 np,np,np,62,np,np,np,63,52,53,  //40 -49
																 54,55,56,57,58,59,60,61,np,np,  //50 -59
																 np,np,np,np,np, 0, 1, 2, 3, 4,  //60 -69
																  5, 6, 7, 8, 9,10,11,12,13,14,  //70 -79
																 15,16,17,18,19,20,21,22,23,24,  //80 -89
																 25,np,np,np,np,np,np,26,27,28,  //90 -99
																 29,30,31,32,33,34,35,36,37,38,  //100 -109
																 39,40,41,42,43,44,45,46,47,48,  //110 -119
																 49,50,51,np,np,np,np,np,np,np,  //120 -129
																 np,np,np,np,np,np,np,np,np,np,  //130 -139
																 np,np,np,np,np,np,np,np,np,np,  //140 -149
																 np,np,np,np,np,np,np,np,np,np,  //150 -159
																 np,np,np,np,np,np,np,np,np,np,  //160 -169
																 np,np,np,np,np,np,np,np,np,np,  //170 -179
																 np,np,np,np,np,np,np,np,np,np,  //180 -189
																 np,np,np,np,np,np,np,np,np,np,  //190 -199
																 np,np,np,np,np,np,np,np,np,np,  //200 -209
																 np,np,np,np,np,np,np,np,np,np,  //210 -219
																 np,np,np,np,np,np,np,np,np,np,  //220 -229
																 np,np,np,np,np,np,np,np,np,np,  //230 -239
																 np,np,np,np,np,np,np,np,np,np,  //240 -249
																 np,np,np,np,np,np               //250 -256
												};


}

namespace Base64 {

	string encode(const WSTRING& data)
	{
		string text = string(reinterpret_cast<char const*>(data.c_str()),
				data.size()*sizeof(WCHAR));
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
		for (unsigned i=0; i<text.size(); i+=2) {
			char temp = text[i];
			text[i] = text[i+1];
			text[i+1] = temp;
		}
#endif
		return encode(text);
	}

	string encode(const unsigned char* data, const unsigned long dwLength)
	{
		 unsigned long i;
		 char          c;
		 string        ret;

		ret.reserve(dwLength * 2);

		 for (i = 0; i < dwLength; ++i)
		 {
			  c = char((data[i] >> 2) & 0x3f);
			  ret.append(1, Base64Table[c]);
			  c = char((data[i] << 4) & 0x3f);
			  if (++i < dwLength)
					c |= char((data[i] >> 4) & 0x0f);

			  ret.append(1, Base64Table[c]);
			  if (i < dwLength)
			  {
					c = char((data[i] << 2) & 0x3f);
					if (++i < dwLength)
						 c |= char((data[i] >> 6) & 0x03);

					ret.append(1, Base64Table[c]);
			  }
			  else
			  {
					++i;
					ret.append(1, fillchar);
			  }

			  if (i < dwLength)
			  {
					c = char(data[i] & 0x3f);
					ret.append(1, Base64Table[c]);
			  }
			  else
			  {
					ret.append(1, fillchar);
			  }
		 }

		 return ret;
	}

	string encode(const string& data)
	{
		 string::size_type  i;
		 char               c;
		 string::size_type  len = data.length();
		 string             ret;

		ret.reserve(len * 2);

		 for (i = 0; i < len; ++i)
		 {
			  c = char((data[i] >> 2) & 0x3f);
			  ret.append(1, Base64Table[c]);
			  c = char((data[i] << 4) & 0x3f);
			  if (++i < len)
					c |= char((data[i] >> 4) & 0x0f);

			  ret.append(1, Base64Table[c]);
			  if (i < len)
			  {
					c = char((data[i] << 2) & 0x3f);
					if (++i < len)
						 c |= char((data[i] >> 6) & 0x03);

					ret.append(1, Base64Table[c]);
			  }
			  else
			  {
					++i;
					ret.append(1, fillchar);
			  }

			  if (i < len)
			  {
					c = char(data[i] & 0x3f);
					ret.append(1, Base64Table[c]);
			  }
			  else
			  {
					ret.append(1, fillchar);
			  }
		 }

		 return(ret);
	}

	void decode(const string& data, WSTRING &returnvalue)
	{
		string text = decode(data);
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
		for (unsigned i=0; i<text.size(); i+=2) {
			char temp = text[i];
			text[i] = text[i+1];
			text[i+1] = temp;
		}
#endif
		returnvalue=WSTRING(reinterpret_cast<const WCHAR *>(text.c_str()),
				text.size()/2*sizeof(char));  //# Unicode chars
	}

	void decode(const string& data, string &returnvalue) {
		returnvalue=decode(data);
	}

	string decode(const string &data)
	{
		string::size_type  i;
		char               c;
		char               c1;
		string::size_type  len = data.length();
		string             ret;

		ret.reserve(len);

		for (i = 0; i < len; ++i)
		{
			c = (char) DecodeTable[(unsigned char)data[i]];
			++i;
			c1 = (char) DecodeTable[(unsigned char)data[i]];
			c = char((c << 2) | ((c1 >> 4) & 0x3));
			ret.append(1, c);
			if (++i < len)
			{
				c = data[i];
				if (fillchar == c)
					break;
				
				c = (char) DecodeTable[(unsigned char)data[i]];
				c1 = char(((c1 << 4) & 0xf0) | ((c >> 2) & 0xf));
				ret.append(1, c1);
			}
			
			if (++i < len)
			{
				c1 = data[i];
				if (fillchar == c1)
					break;
				
				c1 = (char) DecodeTable[(unsigned char)data[i]];
				c = char(((c << 6) & 0xc0) | c1);
				ret.append(1, c);
			}
		 }

		 return ret;
	}

	unsigned long decode(const string &data, unsigned char* &returnvalue)
	{
		string::size_type i;
		char              c;
		char              c1;
		string::size_type len = data.length();

		//Allocate memory for decoded data.  Must be deleted by caller.
		returnvalue = new unsigned char[len * 4/3 + sizeof(unsigned int)];
		unsigned long dwLength = 0;

		for (i = 0; i < len; ++i)
		{
			c = (char) DecodeTable[(unsigned char)data[i]];
			++i;
			c1 = (char) DecodeTable[(unsigned char)data[i]];
			c = char((c << 2) | ((c1 >> 4) & 0x3));
			returnvalue[dwLength++] = c;
			if (++i < len)
			{
				c = data[i];
				if (fillchar == c)
					break;
				
				c = (char) DecodeTable[(unsigned char)data[i]];
				c1 = char(((c1 << 4) & 0xf0) | ((c >> 2) & 0xf));
				returnvalue[dwLength++] = c1;
			}
			
			if (++i < len)
			{
				c1 = data[i];
				if (fillchar == c1)
					break;
				
				c1 = (char) DecodeTable[(unsigned char)data[i]];
				c = char(((c << 6) & 0xc0) | c1);
				returnvalue[dwLength++] = c;
			}
		}

		//Null-terminate with sizeof(unsigned int) null chars.
		for (i = sizeof(unsigned int); i--; )
			returnvalue[dwLength++] = 0;

		return dwLength - 4; //return size (w/o null chars)
	}
}
