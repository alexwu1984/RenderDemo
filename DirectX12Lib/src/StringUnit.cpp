#include "StringUnit.h"
#include <windows.h>
#include <cassert>
#include <memory>

#define  USE_UTF8

namespace core
{
    std::vector<std::string> split(const std::string & src, const char & delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream sstream(src);
        while(std::getline(sstream, token, delimiter)) { tokens.push_back(token); }
        return tokens;
    }

	std::vector<std::wstring> split(const std::wstring & src, const wchar_t & delimiter)
	{
		std::vector<std::wstring> tokens;
		std::wstring token;
		std::wistringstream sstream(src);
		while (std::getline(sstream, token, delimiter)) { tokens.push_back(token); }
		return tokens;
	}

    std::string & ltrim(std::string & str, const std::string & chars)
    {
        str.erase(0, str.find_first_not_of(chars));
        return str;
    }

    std::string & rtrim(std::string & str, const std::string & chars)
    {
        str.erase(str.find_last_not_of(chars) + 1);
        return str;
    }

    std::string & trim(std::string & str, const std::string & chars) { return ltrim(rtrim(str, chars), chars); }

	std::wstring & ltrim(std::wstring & str, const std::wstring & chars)
	{
		str.erase(0, str.find_first_not_of(chars));
		return str;
	}

	std::wstring & rtrim(std::wstring & str, const std::wstring & chars)
	{
		str.erase(str.find_last_not_of(chars) + 1);
		return str;
	}

	std::wstring & trim(std::wstring & str, const std::wstring & chars) { return ltrim(rtrim(str, chars), chars); }


    std::string ansi_u8(const char * text, int32_t length)
    {
        if (length < 0)
            length = (int32_t)std::strlen(text);
#ifdef USE_UTF8
        int32_t nwchars = MultiByteToWideChar(CP_ACP, 0, text, length, NULL, 0);
        wchar_t * u16 = new wchar_t[nwchars];
        int32_t nwchars2 = MultiByteToWideChar(CP_ACP, 0, text, length, u16, nwchars);
        assert(nwchars2 == nwchars);

        int32_t nchars = WideCharToMultiByte(CP_UTF8, 0, u16, nwchars2, NULL, 0, NULL, FALSE);
        char * u8 = new char[nchars];
        int32_t nchars2 = WideCharToMultiByte(CP_UTF8, 0, u16, nwchars2, u8, nchars, NULL, FALSE);
        delete[] u16;
        assert(nchars2 == nchars);
        std::string str(u8, nchars);
        delete[] u8;
        return str;
#else
        return std::string(text, length);
#endif
    }

    std::string u8_ansi(const char * text, int32_t length)
    {
        if (length < 0)
            length = (int32_t)std::strlen(text);
#ifdef USE_UTF8
        int32_t nwchars = MultiByteToWideChar(CP_UTF8, 0, text, length, NULL, 0);
        wchar_t * u16 = new wchar_t[nwchars];
        int32_t nwchars2 = MultiByteToWideChar(CP_UTF8, 0, text, length, u16, nwchars);
        assert(nwchars2 == nwchars);

        int32_t nchars = WideCharToMultiByte(CP_ACP, 0, u16, nwchars2, NULL, 0, NULL, FALSE);
        char * out = new char[nchars];
        int32_t nchars2 = WideCharToMultiByte(CP_ACP, 0, u16, nwchars2, out, nchars, NULL, FALSE);
        delete[] u16;
        assert(nchars2 == nchars);
        std::string str(out, nchars);
        delete[] out;
        return str;
#else
        return std::string(text, length);
#endif
    }

    std::string ucs2_u8(const wchar_t * text, int32_t length)
    {
        if (length < 0)
            length = (int32_t)std::wcslen(text);
#ifdef USE_UTF8
        int32_t nchars = WideCharToMultiByte(CP_UTF8, 0, text, length, NULL, 0, NULL, FALSE);
#else
        int32_t nchars = WideCharToMultiByte(CP_ACP, 0, text, length, NULL, 0, NULL, FALSE);
#endif
        char * u8 = new char[nchars];
#ifdef USE_UTF8
        int32_t nchars2 = WideCharToMultiByte(CP_UTF8, 0, text, length, u8, nchars, NULL, FALSE);
#else
        int32_t nchars2 = WideCharToMultiByte(CP_ACP, 0, text, length, u8, nchars, NULL, FALSE);
#endif
        assert(nchars2 == nchars);
        std::string str(u8, nchars);
        delete[] u8;
        return str;
    }

	TCHAR NibbleToTChar(uint8_t Num)
	{
		if (Num > 9)
		{
			return TEXT('A') + TCHAR(Num - 10);
		}
		return TEXT('0') + TCHAR(Num);
	}

	const uint8_t TCharToNibble(const TCHAR Char)
	{
		if (Char >= TEXT('0') && Char <= TEXT('9'))
		{
			return Char - TEXT('0');
		}
		else if (Char >= TEXT('A') && Char <= TEXT('F'))
		{
			return (Char - TEXT('A')) + 10;
		}
		return (Char - TEXT('a')) + 10;
	}

	std::string ucs2_ansi(const wchar_t * text, int32_t length)
    {
        if (length < 0)
            length = (int32_t)std::wcslen(text);

        int32_t nchars = WideCharToMultiByte(CP_ACP, 0, text, length, NULL, 0, NULL, FALSE);
        char * u8 = new char[nchars];
        int32_t nchars2 = WideCharToMultiByte(CP_ACP, 0, text, length, u8, nchars, NULL, FALSE);
        assert(nchars2 == nchars);
        std::string str(u8, nchars);
        delete[] u8;
        return str;
    }

    std::wstring u8_ucs2(const char * text, int32_t length)
    {
        if (length < 0)
            length = (int32_t)std::strlen(text);
#ifdef USE_UTF8
        int32_t nchars = MultiByteToWideChar(CP_UTF8, 0, text, length, NULL, 0);
#else
        int32_t nchars = MultiByteToWideChar(CP_ACP, 0, text, length, NULL, 0);
#endif
        wchar_t * u16 = new wchar_t[nchars];
        int32_t nchars2 = MultiByteToWideChar(CP_UTF8, 0, text, length, u16, nchars);
        assert(nchars2 == nchars);
        std::wstring ucs2(u16, nchars);
        delete[] u16;
        return ucs2;
    }

    std::wstring ansi_ucs2(const char * text, int32_t length)
    {
        if (length < 0)
            length = std::strlen(text);
        int32_t nchars = MultiByteToWideChar(CP_ACP, 0, text, length, NULL, 0);
        wchar_t * u16 = new wchar_t[nchars];
        int32_t nchars2 = MultiByteToWideChar(CP_ACP, 0, text, length, u16, nchars);
        assert(nchars2 == nchars);
        std::wstring ucs2(u16, nchars);
        delete[] u16;
        return ucs2;
    }

    std::string ansi_u8(std::string str)
    {
        return ansi_u8(str.c_str(), (int32_t)str.length());
    }

    std::string u8_ansi(std::string str)
    {
        return u8_ansi(str.c_str(), (int32_t)str.length());
    }

    std::string usc2_u8(std::wstring str)
    {
        return ucs2_u8(str.c_str(), (int32_t)str.length());
    }

    std::string usc2_ansi(std::wstring str)
    {
        return ucs2_ansi(str.c_str(), (int32_t)str.length());
    }

    std::wstring u8_ucs2(std::string str)
    {
        return u8_ucs2(str.c_str(), (int32_t)str.length());
    }

    std::string ucs2_u8(std::wstring str)
    {
        return ucs2_u8(str.c_str(), (int32_t)str.length());
    }

    std::string from_bytes(std::shared_ptr<uint8_t> bytes, int32_t nbytes)
    {
        std::string str;
        const char chars[] = { '0' , '1' , '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        for(int32_t cnt = 0; cnt < nbytes; ++cnt)
        {
            uint8_t b = bytes.get()[cnt];
            str.append(1, chars[(b >> 4) & 0xf]);
            str.append(1, chars[b & 0xf]);
        }
        return str;
    }
}
