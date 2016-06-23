#ifndef MSWUREGISTRY_H
#define MSWUREGISTRY_H

#include "MswUString.h"

#include <Windows.h>
#include <string>

namespace mswu {

inline void delRegKey( const HKEY &hKey, const LPCWSTR &sKey ) 
{
	// note: RegDeleteKeyEx is not defined on XP, so using the older version
	//::RegDeleteKeyEx( hKey, sKey, KEY_WOW64_32KEY, 0 );
	//::RegDeleteKeyEx( hKey, sKey, KEY_WOW64_64KEY, 0 );
	::RegDeleteKey( hKey, sKey );
}

inline void getRegString( const HKEY &hKey, const LPCWSTR &sKey, std::string &sv ) 
{
	DWORD sz;

	if ( ::RegQueryValueEx( hKey, sKey, 0, NULL, NULL, &sz ) == ERROR_SUCCESS ) {
		if ( sz == 0 ) {
			sv = "";
			return;
		}

		wchar_t *wc	= new wchar_t[sz];

		if ( RegQueryValueEx( hKey, sKey, 0, NULL, (BYTE *)wc, &sz) == ERROR_SUCCESS ) {		
			sv = ws2s( wc );
		}
	}
};

inline long setRegString( const HKEY &hKey, const LPCWSTR &sKey, const std::string &sv ) 
{	
	std::wstring	ws	= s2ws( sv );	
	const wchar_t	*wc	= ws.c_str();
	
	return ::RegSetValueExW( hKey, sKey, 0, REG_SZ, (BYTE *)wc, wcslen(wc)*sizeof( wchar_t ) ); 
};

inline bool getRegBool( const HKEY &hKey, const LPCWSTR &sKey )
{	
	DWORD dw, dwLen = sizeof(DWORD);

	::RegQueryValueEx( hKey, sKey, 0, NULL, (LPBYTE)&dw, &dwLen );
	
	return dw == 1;
};

inline long setRegBool( const HKEY &hKey, const LPCWSTR &sKey, const bool &bVal ) 
{
	DWORD dw = bVal, dwLen = sizeof(DWORD);
	
	return ::RegSetValueEx( hKey, sKey, 0, REG_DWORD, (PBYTE)&dw, dwLen );
};

inline int getRegInt( const HKEY &hKey, const LPCWSTR &sKey )
{	
	DWORD dw, dwLen = sizeof(DWORD);

	::RegQueryValueEx( hKey, sKey, 0, NULL, (LPBYTE)&dw, &dwLen );
	
	return dw;
};

inline long setRegInt( const HKEY &hKey, const LPCWSTR &sKey, const int &iVal ) 
{
	DWORD dw = iVal, dwLen = sizeof(DWORD);
	
	return ::RegSetValueEx( hKey, sKey, 0, REG_DWORD, (PBYTE)&dw, dwLen );
};

} // mswu

#endif
