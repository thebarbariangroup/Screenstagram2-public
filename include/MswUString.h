#ifndef MSWUSTRING_H
#define MSWUSTRING_H

#include <Windows.h>
#include <string>

namespace mswu {

inline std::wstring c2ws( const char *&cs ) 
{
    int mbLen = strlen( cs ) + 1;
    int wcLen = ::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, cs, mbLen, 0, 0);

    std::wstring buf( wcLen, L'\0');
    
	::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, cs, mbLen, &buf[0], wcLen );
    
	return buf;
};

inline std::string wc2s( const wchar_t *&wcs ) 
{
    int wsLen = wcslen( wcs ) + 1;
    int mbLen = ::WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK, wcs, wsLen, 0, 0, NULL, NULL ); 
    
	std::string buf( mbLen, '\0');

    ::WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK, wcs, wsLen, &buf[0], mbLen, NULL, NULL ); 
    
	return buf;
};

inline std::wstring s2ws( const std::string &s )
{
	const char *cs = s.c_str();

	return c2ws( cs );
};

inline std::string ws2s( const std::wstring &s )
{
	const wchar_t *wcs = s.c_str();

	return wc2s( wcs );
};

} // mswu

#endif