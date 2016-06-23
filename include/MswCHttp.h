#ifndef MSWCHTTP_H
#define MSWCHTTP_H

#include <Windows.h>
#include <WinInet.h>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>

namespace mswc {

template< class T=char > 
class MswCHttp
{
private:
	const char *mAppName;
	const char *mServerAddress;
	std::vector<std::string> 
				mHeaders;

	HINTERNET	mSess;
	HINTERNET	mConn;
	HINTERNET	mReq;
	DWORD		mError;
	DWORD		mErrorCode;
		
	void close() 
	{
		::InternetCloseHandle( mReq );
		::InternetCloseHandle( mSess );
		::InternetCloseHandle( mConn );
	};
	
	void connect( bool pUseHttps )
	{
		close();

		INTERNET_PORT dwPort = pUseHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

		mSess = ::InternetOpenA( 
			mAppName, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
		mConn = ::InternetConnectA( 
			mSess, mServerAddress, dwPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );
	};

	void request( const char *pMethod, const char *pRequestPath, bool pUseHttps )
	{
		const char* acceptTypes[] = { "text/*", "application/*", NULL };

		DWORD dwProtocolFlags = pUseHttps ? 
			INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID : 
			0;

		mReq = ::HttpOpenRequestA( mConn, pMethod, pRequestPath, NULL, NULL, acceptTypes, dwProtocolFlags | INTERNET_FLAG_RELOAD | INTERNET_FLAG_KEEP_CONNECTION, 0 );
	};

	std::string getHeaders()
	{
		if ( mHeaders.size() == 0)
			return "";

		std::stringstream ss;
		std::copy( mHeaders.begin(), mHeaders.end(), ostream_iterator< ::string, char >( ss, "\r\n" ) );

		return ss.str();
	};

	long getResponse( T *&pBuffer )
	{		
		if ( mReq ) {			
		// Detect 'hidden' errors in returned header; @see InternetErrorDlg#FLAGS_ERROR_UI_FILTER_FOR_ERRORS:

			mErrorCode = InternetErrorDlg( NULL, mReq, 0, 
				FLAGS_ERROR_UI_FLAGS_NO_UI | FLAGS_ERROR_UI_FILTER_FOR_ERRORS | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS | FLAGS_ERROR_UI_FLAGS_GENERATE_DATA,
				NULL);
		}

		if ( mErrorCode == ERROR_SUCCESS ) {

			DWORD dwRspLen;
			DWORD dwBufLen	= sizeof(dwRspLen);

			if ( HttpQueryInfo( mReq, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (LPVOID)&dwRspLen, &dwBufLen, 0 ) ) {		
				
				DWORD	dwBytesRead = 0;
				char	*rspBuffer	= (char*)::malloc(dwRspLen + 1);
				char	*tmpBuffer	= rspBuffer;
						
				while( ::InternetReadFile( mReq, tmpBuffer, dwRspLen, &dwBytesRead) && dwBytesRead != 0 ) {
					tmpBuffer += dwBytesRead;
				}
				rspBuffer[dwRspLen] = 0;

				dwBufLen = (dwRspLen*sizeof(WCHAR))+1;
				pBuffer	 = new T[dwBufLen];

				setResponse( pBuffer, rspBuffer, dwBufLen );
			
				::free(rspBuffer);
			}
		} else {

			mErrorCode = ::GetLastError();
		}

		close();

		return mErrorCode;
	};

	void setResponse( T *&pTarget, char *&pSource, DWORD pSourceLen )
	{
		memcpy( pTarget, pSource, pSourceLen );
	};

public:
	MswCHttp( const char *pAppName, const char *pServerAddress ) 
	{
		mAppName		= pAppName;
		mServerAddress	= pServerAddress;
	};

	~MswCHttp() 
	{
		close();
	};

	void addHeader( std::string pHeader )
	{
		mHeaders.push_back( pHeader );
	};

	long doGet( T *&pBuffer, const std::string *pRequestPath, bool pUseHttps )
	{
		connect( pUseHttps );
		request( "GET", pRequestPath->c_str(), pUseHttps );

		std::string	headers		= getHeaders();
		const char *szHeaders	= headers.c_str();

		::HttpSendRequestA( mReq, szHeaders, strlen(szHeaders), NULL, 0 );

		return getResponse( pBuffer );
	};

	long doPost( T *&pBuffer, const std::string *pRequestPath, const char *pPostData, bool pUseHttps )
	{
		connect( pUseHttps );
		request( "POST", pRequestPath->c_str(), pUseHttps );

		std::string	headers		= getHeaders();
		const char *szHeaders	= headers.c_str();

		::HttpSendRequestA( mReq, szHeaders, strlen(szHeaders), (LPVOID)pPostData, strlen(pPostData) );

		return getResponse( pBuffer );
	};
};

template<> void MswCHttp<wchar_t>::setResponse( wchar_t *&pTarget, char *&pSource, DWORD pSourceLen )
{
	::MultiByteToWideChar( CP_UTF8, 0, pSource, -1, &pTarget[0], pSourceLen );
};

} // mswc

#endif