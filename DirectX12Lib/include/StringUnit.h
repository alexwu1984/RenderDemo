#pragma once

#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <tchar.h>


namespace core
{
    std::vector<std::string> split(const std::string & src, const char & delimiter);
	std::vector<std::wstring> split(const std::wstring & src, const wchar_t & delimiter);
    std::string & ltrim(std::string & str, const std::string & chars = "\t\n\v\f\r ");
    std::string & rtrim(std::string & str, const std::string & chars = "\t\n\v\f\r ");
    std::string & trim(std::string & str, const std::string & chars = "\t\n\v\f\r ");

	std::wstring & ltrim(std::wstring & str, const std::wstring & chars = L"\t\n\v\f\r ");
	std::wstring & rtrim(std::wstring & str, const std::wstring & chars = L"\t\n\v\f\r ");
	std::wstring & trim(std::wstring & str, const std::wstring & chars = L"\t\n\v\f\r ");

    std::string ansi_u8(const char * text, int32_t length);
    std::string u8_ansi(const char * text, int32_t length);
    std::string ucs2_u8(const wchar_t * text, int32_t length);
    std::string ucs2_ansi(const wchar_t * text, int32_t length);
    std::wstring u8_ucs2(const char * text, int32_t length);
    std::wstring ansi_ucs2(const char * text, int32_t length);

    std::string ansi_u8(std::string str);
    std::string u8_ansi(std::string str);
    std::string usc2_u8(std::wstring str);
    std::string usc2_ansi(std::wstring str);
    std::wstring u8_ucs2(std::string str);
    std::string ucs2_u8(std::wstring str);

	/** Returns Char value of Nibble */
	TCHAR NibbleToTChar(uint8_t Num);


	/**
 * Convert a byte to hex
 * @param In byte value to convert
 * @param Result out hex value output
 */
	inline void ByteToHex(uint8_t In, std::basic_string<TCHAR>& Result)
	{
		Result += NibbleToTChar(In >> 4);
		Result += NibbleToTChar(In & 15);
	}

	/**
	 * Convert an array of bytes to hex
	 * @param In byte array values to convert
	 * @param Count number of bytes to convert
	 * @return Hex value in string.
	 */
	inline std::basic_string<TCHAR> BytesToHex(const uint8_t* In, int32_t Count)
	{
		std::basic_string<TCHAR> Result;
		Result.resize(Count * 2);

		while (Count)
		{
			ByteToHex(*In++, Result);
			Count--;
		}
		return Result;
	}

	/**
 * Convert a TChar to equivalent hex value as a uint8
 * @param Char		The character
 * @return	The uint8 value of a hex character
 */
	const uint8_t TCharToNibble(const TCHAR Char);

	/**
	 * Convert FString of Hex digits into the byte array.
	 * @param HexString		The FString of Hex values
	 * @param OutBytes		Ptr to memory must be preallocated large enough
	 * @return	The number of bytes copied
	 */
	inline int32_t HexToBytes(const std::basic_string<TCHAR>& HexString, uint8_t* OutBytes)
	{
		int32_t NumBytes = 0;
		const bool bPadNibble = (HexString.length() % 2) == 1;
		const TCHAR* CharPos = HexString.data();
		if (bPadNibble)
		{
			OutBytes[NumBytes++] = TCharToNibble(*CharPos++);
		}
		while (*CharPos)
		{
			OutBytes[NumBytes] = TCharToNibble(*CharPos++) << 4;
			OutBytes[NumBytes] += TCharToNibble(*CharPos++);
			++NumBytes;
		}
		return NumBytes;
	}

    struct less_ic
    {
        bool operator() (const std::string & lhs, const std::string & rhs) const
        {
            std::string str1(lhs.length(), ' ');
            std::string str2(rhs.length(), ' ');
            std::transform(lhs.begin(), lhs.end(), str1.begin(), tolower);
            std::transform(rhs.begin(), rhs.end(), str2.begin(), tolower);
            return  str1 < str2;
        }
    };

    struct equal_ic
    {
        bool operator() (const std::string& s1, const std::string& s2) const
        {
            if (s1.length() != s2.length())
                return false;

            std::string str1(s1.length(), ' ');
            std::string str2(s2.length(), ' ');
            std::transform(s1.begin(), s1.end(), str1.begin(), tolower);
            std::transform(s2.begin(), s2.end(), str2.begin(), tolower);
            return  str1 == str2;
        }

        bool operator() (const char * s1, const char * s2) const
        {
            if (s1 == s2)
                return true;

            if (!s1 || !s2)
                return false;

            if (!*s1 || !*s2)
                return *s1 == *s2;

            while(true)
            {
                if (!*s1 || !*s2)
                    return false;

                if (tolower(*s1) != tolower(*s2))
                    return false;

                ++s1;
                ++s2;
            }
        }

        bool operator() (const wchar_t * s1, const wchar_t * s2) const
        {
            if (s1 == s2)
                return true;

            if (!s1 || !s2)
                return false;

            while (true)
            {
                if (!*s1 || !*s2)
                    return *s1 == *s2;

                if (tolower(*s1) != tolower(*s2))
                    return false;

                ++s1;
                ++s2;
            }
        }
    };

    inline void format_helper(std::ostringstream & stream) {}

    template<typename Head, typename ...Tail>
    void format_helper(std::ostringstream & stream, const Head & head, Tail && ...tail)
    {
        stream << head;
        return format_helper(stream, std::forward<Tail>(tail)...);
    }

    template<typename ...Args>
    std::string format(Args && ...args)
    {
        std::ostringstream stream;
        format_helper(stream, std::forward<Args>(args)...);
        return stream.str();
    }

    inline void format_helperw(std::wostringstream & stream) {}

    template<typename Head, typename ...Tail>
    void format_helperw(std::wostringstream & stream, const Head & head, Tail && ...tail)
    {
        stream << head;
        return format_helperw(stream, std::forward<Tail>(tail)...);
    }

    template<typename ...Args>
    std::wstring formatw(Args && ...args)
    {
        std::wostringstream stream;
        format_helperw(stream, std::forward<Args>(args)...);
        return stream.str();
    }

    std::string from_bytes(std::shared_ptr<uint8_t> bytes, int32_t nbytes);
}
