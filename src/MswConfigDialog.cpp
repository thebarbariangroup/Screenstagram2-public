#include "MswConfigDialog.h"
#include "MswVersion.h"
#include "MswUString.h"
#include "MswURegistry.h"
#include "MswCHttp.h"
using namespace mswu;
using namespace mswc;

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include <minmax.h>
#include <GdiPlus.h>
using namespace Gdiplus;

#include <cinder/Json.h>
using namespace ci;

using namespace std;

// Screenstagram download URL
static const ::string DOWNLOAD_URL = "http:\\\\barbariangroup.com\\software\\screenstagram";
/*
// Sample Foursquare venue URL (link)
static const ::string FOURSQ_VENUE_URL = "https:\\\\foursquare.com\\v\\barbarian-group--nyc\\49d164f1f964a5207c5b1fe3";
*/
// Registry group for options save
static const wchar_t *REGISTRY_PATH = L"Software\\The Barbarian Group\\Screenstagram 2";
/*
// Max characters for Foursquare venue ID
static const int FOURSQUARE_VENUEID_MAXLEN = 40;
*/
// Instagram username max is currently 30
static const int INSTAGRAM_USERNAME_MAXLEN = 35;
static const int INSTAGRAM_PASSWORD_MAXLEN = 50;

// Max characters for tag field, arbitrary
static const int INSTAGRAM_TAGSLIST_MAXLEN = 200;

// Update check endpoint details
static const ::string UPDATEPOLL_SERVER	= "screenstagram.s3.amazonaws.com";
static const ::string UPDATEPOLL_PATH	= "/win-currentVersion.txt";

// XAUTH endpoint details
static const ::string INSTAGRAM_AUTH_SERVER		= "api.instagram.com";
static const ::string INSTAGRAM_AUTH_PATH		= "/oauth/access_token";
static const ::string INSTAGRAM_CLIENT_ID		= "c54e61b2f7554ca5bd7ce61bb371cffb";
static const ::string INSTAGRAM_CLIENT_SECRET	= "d6c736a4fb7b4e4abe37b382ba73d127";
static const ::string INSTAGRAM_AUTH_DATA_TPL	= "username=%s&password=%s&grant_type=password&client_id=%s&client_secret=%s";

ULONG_PTR	gdipToken;
BOOL		bDownloadUpdate		= false;
BOOL		bAccessToken		= false;
char		msgBoxMessage[300];
char		msgBoxCaption[100];
/*
HWND		hwFourSqOptionsDlg;
*/

BOOL bDefaultMode = false; // (interim UI helper)

int getErrorMsgBox( HWND hDlg, DWORD dwErrorCode=NULL )
{
	if ( dwErrorCode ) {
		std::stringstream ss; 
		ss << "\n\n" << dwErrorCode;
		strcat_s( msgBoxMessage, sizeof(msgBoxMessage), ss.str().c_str() );
	}

	::MessageBeep( MB_ICONERROR );

	return ::MessageBoxExA( hDlg, msgBoxMessage, msgBoxCaption, MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND, 0 );
}

void toggleDefaultModeControls( HWND hDlg )
{
	BOOL toggle = ::IsDlgButtonChecked( hDlg, IDC_DEFAULTBEHAVIOR );
}

void toggleTaggedExclusiveModeControls( HWND hDlg )
{	
	BOOL toggle = ::IsDlgButtonChecked( hDlg, IDC_ONLYINCLUDETAGGED );

	::EnableWindow( ::GetDlgItem( hDlg, IDC_INCLUDETAGGEDINFO ), toggle && bAccessToken );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_TAGS ), toggle && bAccessToken );
}

void toggleAuthStateControls( HWND hDlg ) 
{	
	int toggleNoAuth[] = { 
		IDC_USERNAMEINFO, IDC_USERNAME, IDC_PASSWORDINFO, IDC_PASSWORD, IDC_LOGIN 
		};
	int toggleOnAuth[] = {
		IDC_DEAUTHENTICATEINFO, IDC_DEAUTHENTICATE, IDC_ONLYSHOWLIKED, IDC_ONLYINCLUDETAGGED 
		};

	for each( int id in toggleNoAuth ) 
		::EnableWindow( ::GetDlgItem( hDlg, id ), !bAccessToken );
	for each( int id in toggleOnAuth ) 
		::EnableWindow( ::GetDlgItem( hDlg, id ), bAccessToken );

	toggleDefaultModeControls( hDlg );
	toggleTaggedExclusiveModeControls( hDlg );
}

void loadNonBmpImgResource( int resource, ::Image *&pImg ) 
{	
	::GdiplusShutdown(gdipToken);
	::GdiplusStartupInput gdipStartupInput;
	::GdiplusStartup(&gdipToken, &gdipStartupInput, NULL);

	HINSTANCE hInst	= ::GetModuleHandle( NULL );

	HRSRC hRes = ::FindResource( hInst, MAKEINTRESOURCE(resource), RT_RCDATA );
	if (!hRes)
		return;
	
	LPVOID lpRes = ::LockResource(  
		::LoadResource( hInst, hRes ) 
		);
	if (!lpRes) 
		return;

	DWORD	dwSize	= ::SizeofResource( hInst, hRes );
	HGLOBAL	hBuff	= ::GlobalAlloc( GMEM_FIXED, dwSize );
	LPVOID	lpResX	= ::LockResource( lpRes );	

	memcpy( hBuff, lpResX, dwSize );
	::FreeResource( hBuff );

	LPSTREAM st = NULL;
	::CreateStreamOnHGlobal( hBuff, TRUE, &st );	
	pImg = ::Image::FromStream( st, FALSE );

	st->Release();
}

void drawNonBmpImgTo( ::Gdiplus::Image *pImg, HWND hDlg, int nDlgItem, ::Gdiplus::Rect rct ) 
{
	HWND hBox = ::GetDlgItem( hDlg, nDlgItem );
	
	::Gdiplus::Graphics g( hBox );
	g.DrawImage( pImg, rct ); 

	ValidateRect( hBox, NULL );
}

// TODO, runonce:
void updateConfigMsw()
{
	HKEY key;
	LONG result = ::RegOpenKeyEx( HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_READ, &key );
	if( result != ERROR_SUCCESS )
		return;	

	// Ensure no old keys?
	::delRegKey( key, L"TaggedInclusive" );
	::delRegKey( key, L"TaggedExclusive" );
	::delRegKey( key, L"LikedExclusive" );

	::RegCloseKey( key );
}

void loadConfigMsw( Configuration *config )
{	
	// This really should be moved to the Configuration struct.
	config->accessToken = "";
	config->username = "";
	config->showUsernames = false;
	config->includeTaggedPhotos = false;
	config->onlyIncludeLikedPhotos = false;
	config->photoTags = "";
/*
	config->fourquareVenueId = "";
*/

	HKEY key;
	LONG result = ::RegOpenKeyEx( HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_READ, &key );
	if( result != ERROR_SUCCESS )
		return;	

	::getRegString( key, L"Username", config->username );
	::getRegString( key, L"AccessToken", config->accessToken );
	::getRegString( key, L"PhotoTags", config->photoTags );
/*
	::getRegString( key, L"FoursquareVenue", config->foursquareVenueId );
*/

	config->showUsernames = ::getRegBool( key, L"ShowUsernames" );

	// New config flow, but same old Configuration class; needs OS parity, so, TODO.
	// Config class should probably be refactored to use a mode enum for exclusives.
	// This is hella awkward, setting up Config object per exclusive mode:
	//
	switch( ::getRegInt( key, L"ExclusionMode" ) ) 
	{
	case 1: 
	// Only liked; if not authenticated, force default mode

		if ( config->accessToken.empty() ) {
			bDefaultMode = true;
			break;
		}
		config->includeTaggedPhotos = false;
		config->onlyIncludeLikedPhotos = true;
	break;
	case 2: 
	// Only tagged; if not authenticated or no tags, force default mode

		if ( config->accessToken.empty() || config->photoTags.empty() ) {
			bDefaultMode = true; 
			break;
		}
		config->includeTaggedPhotos = true;
		config->onlyIncludeLikedPhotos = false;
	break;
	default:
		bDefaultMode = true;
	};
	
	if ( bDefaultMode ) {
		config->includeTaggedPhotos = false;
		config->onlyIncludeLikedPhotos = false;
	}

	::RegCloseKey( key );
}

void saveConfigMsw( const Configuration &config )
{
	HKEY key;
	LONG result = ::RegCreateKeyEx( HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, NULL );
	if( result != ERROR_SUCCESS )
		return;

	::setRegString( key, L"Username", config.username );
	::setRegString( key, L"PhotoTags", config.photoTags );	
	::setRegBool( key, L"ShowUsernames", config.showUsernames );

	// @see loadConfigMsw; more hella awkwardness:

	if ( config.onlyIncludeLikedPhotos ) {
		::setRegInt( key, L"ExclusionMode", 1 );
	}
	else if ( config.includeTaggedPhotos ) {
		::setRegInt( key, L"ExclusionMode", 2 );
	}
	else {
		::setRegInt( key, L"ExclusionMode", 0 );
	}

	::RegCloseKey( key );
}

BOOL saveAccessToken( const ::string &token ) 
{
	HKEY key;
	LONG result = ::RegCreateKeyEx( HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, NULL );
	if( result != ERROR_SUCCESS )
		return FALSE;
	
	bAccessToken = !token.empty();

	::setRegString( key, L"AccessToken", token );

	::RegCloseKey( key );

	return TRUE;
}
/*
BOOL saveFoursquareVenue( const ::string &venueID )
{
	if ( !hwFourSqOptionsDlg )
		return FALSE;

	HKEY key;
	LONG result = ::RegCreateKeyEx( HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, NULL );
	if( result != ERROR_SUCCESS )
		return FALSE;

	::setRegString( key, L"FoursquareVenue", venueID );

	::RegCloseKey( key );

	return TRUE;
}

BOOL CALLBACK getFourSqDialog( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) 
{
	long lwp = LOWORD( wParam );
	long hwp = HIWORD( wParam );

	switch( message ) {
		case WM_INITDIALOG: {
		// Reload config with defaults and set control states

			Configuration cfg;
			loadConfigMsw( &cfg );

			// Foursq dialog text is too long for static control, so we're using a read-only edit box;
			// Load default text:
			wchar_t msgFsDialogTxt[1000];
			::LoadStringW( GetModuleHandle(NULL), IDS_FOURSQDIALOGTXT, msgFsDialogTxt, sizeof(msgFsDialogTxt) );
			::SetDlgItemTextW( hDlg, IDC_FOURSQDIALOGTXT,  msgFsDialogTxt );

			// Update editable controls with stored values:

			// If a Fsq id is currently configured, disabled control and change OK to Clear:
			if ( !cfg.foursquareVenueId.empty() ) {
				::EnableWindow( ::GetDlgItem( hDlg, IDC_FOURSQVENUE ), false );

				wchar_t msgClear[15];
				::LoadStringW( GetModuleHandle(NULL), IDS_FOURSQIDD_CLEARSTATE, msgClear, sizeof(msgClear) );
				::SetDlgItemTextW( hDlg, IDOK, msgClear );
			}				
			::SetDlgItemTextW( hDlg, IDC_FOURSQVENUE, ::s2ws( cfg.foursquareVenueId ).c_str() );			

			return TRUE;
		}
		break;
		case WM_CTLCOLORSTATIC: {
			HWND control = (HWND)lParam;

			// Color action links ( sample venue ID )
			if ( control == ::GetDlgItem( hDlg, IDC_SAMPLEVENUE )) {
				
				::SetBkMode( (HDC)wParam, TRANSPARENT );
				::SetTextColor( (HDC)wParam, ::GetSysColor( COLOR_HIGHLIGHT ));

				return (BOOL)::GetSysColorBrush( COLOR_BTNFACE );
			}
		}
		break;
		case WM_CLOSE: {	

			::EndDialog( hDlg, lwp );
			return TRUE;
		}
		break;
		case WM_COMMAND: {

			switch( lwp ) {
				case IDC_SAMPLEVENUE : {
					// Only honor clicks here
					if ( hwp != STN_CLICKED )
						return FALSE;
					
					// Open sample Foursquare venue page
					::ShellExecuteA( NULL, "open", FOURSQ_VENUE_URL.c_str(), NULL, NULL, SW_SHOWNORMAL );
					
					return TRUE;
				}
				break;
				case IDOK : {
				// Dual-purpose, save or clear:

					// Enable window; if previously disabled, process clear action:
					if ( ::EnableWindow( ::GetDlgItem( hDlg, IDC_FOURSQVENUE ), true ) != 0 ) {
						::SetDlgItemTextW( hDlg, IDOK, L"" );
						saveFoursquareVenue( "" );

						wchar_t msgOK[15];
						::LoadStringW( GetModuleHandle(NULL), IDS_FOURSQIDD_OKSTATE, msgOK, sizeof(msgOK) );
						::SetDlgItemTextW( hDlg, IDOK, msgOK );

						::SetDlgItemTextW( hDlg, IDC_FOURSQVENUE, L"" );

						return TRUE;
					}
					// Else normal save/dialog close:

					char venue[FOURSQUARE_VENUEID_MAXLEN];
					::GetDlgItemTextA( hwFourSqOptionsDlg, IDC_FOURSQVENUE, venue, FOURSQUARE_VENUEID_MAXLEN );

					saveFoursquareVenue( venue ); 
				}
				// #nobreak
				case IDCANCEL: {
				// Post to destroy

					return ::PostMessageA( hDlg, WM_CLOSE, 0, 0 );
				}
				break;
				default: {
					return TRUE;
				}
			};
		}
		break;
		default: {
			return FALSE;
		}
	};

	return FALSE;
}
*/

BOOL getConfigDialogMsw( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	updateConfigMsw();

	long lwp = LOWORD( wParam );
	long hwp = HIWORD( wParam );

	switch( message ) {
		case WM_INITDIALOG: {
		// Reload config with defaults and set control states

			Configuration cfg;
			loadConfigMsw( &cfg );

			// Global session state
			bAccessToken = !cfg.accessToken.empty();

			// Update editable controls with stored values
			::SetDlgItemTextW( hDlg, IDC_USERNAME, ::s2ws( cfg.username ).c_str() );
			::SetDlgItemTextW( hDlg, IDC_TAGS, ::s2ws( cfg.photoTags ).c_str() );
			::CheckDlgButton( hDlg, IDC_SHOWUSERNAMES, cfg.showUsernames );
			
			if ( cfg.onlyIncludeLikedPhotos ) {
				::CheckDlgButton( hDlg, IDC_ONLYSHOWLIKED, true );
			} 
			else if ( cfg.includeTaggedPhotos ) {
				::CheckDlgButton( hDlg, IDC_ONLYINCLUDETAGGED, true );
			} 
			else {
				::CheckDlgButton( hDlg, IDC_DEFAULTBEHAVIOR, true );
			}

			// Toggle related controls from default and honor auth state
			toggleAuthStateControls( hDlg );

			// Set current version (@see MswVersion.h for defs)
			wchar_t curVersion[10];
			wchar_t tplVersion[50];
			wchar_t msgVersion[62];
			::LoadStringW( GetModuleHandle(NULL), IDS_UPDATECHECKVERSION, curVersion, sizeof(curVersion) );
			::LoadStringW( GetModuleHandle(NULL), IDS_UPDATECHECKDEFAULT, tplVersion, sizeof(msgVersion) );
			swprintf_s( msgVersion, 62, tplVersion, curVersion );
			::SetDlgItemText( hDlg, IDC_CURRENTVERSION, msgVersion );
			
			return TRUE;
		}
		break;
		case WM_CLOSE: {	
		// Shutdown Gdi+, if applicable & close

			::GdiplusShutdown(gdipToken);
/*
			if ( hwFourSqOptionsDlg ) 
				::PostMessageA( hwFourSqOptionsDlg, WM_CLOSE, 0, 0 );
*/
			::EndDialog( hDlg, lwp );

			return TRUE;
		}
		break;
		case WM_CTLCOLORSTATIC: {
		// Give a few controls some contrasting text
			
			HWND control = (HWND)lParam;			

			if (( control == ::GetDlgItem( hDlg, IDC_INSTADISCLAIMER )) ||
				( control == ::GetDlgItem( hDlg, IDC_COPYRIGHT ))) {
				
				::SetBkMode( (HDC)wParam, TRANSPARENT );
				::SetTextColor( (HDC)wParam, ::GetSysColor( COLOR_GRAYTEXT ));
				
				return (BOOL)::GetSysColorBrush( COLOR_BTNFACE );
			}

			// Color action links ( check for update and download new )
			if ( control == ::GetDlgItem( hDlg, IDC_CHECKFORUPDATE )) {	
				
				::SetBkMode( (HDC)wParam, TRANSPARENT );
				::SetTextColor( (HDC)wParam, ::GetSysColor( COLOR_HIGHLIGHT ));

				return (BOOL)::GetSysColorBrush( COLOR_BTNFACE );
			}
		}
		break;
		case WM_DRAWITEM: {

			switch( lwp ) {
				case IDC_LOGO: {
				// Draw Screenstagram logo with GDI+

					::Gdiplus::Image *pImg = NULL;
					loadNonBmpImgResource( RES_LOGO_PNG, pImg );
					if ( !pImg ) 
						return FALSE;

					RECT rDis = ((LPDRAWITEMSTRUCT)lParam)->rcItem;

					// Left align and center vertically:
					int xOff = rDis.right - pImg->GetWidth();
					int yOff = rDis.bottom - pImg->GetHeight();
					::Rect rct( rDis.left, rDis.top + yOff/2, pImg->GetWidth(), pImg->GetHeight());

					drawNonBmpImgTo( pImg, hDlg, IDC_LOGO, rct );

					delete pImg; pImg = 0;
				}
				break;
/*
				case IDC_FOURSQLOGO: {
				// Draw FSq logo with GDI+

					::Gdiplus::Image *pImg = NULL;
					loadNonBmpImgResource( RES_FOURSQ_PNG, pImg );
					if ( !pImg ) 
						return FALSE;

					RECT rDis = ((LPDRAWITEMSTRUCT)lParam)->rcItem;

					// Left align+1 and bottom:
					int xOff = rDis.right - pImg->GetWidth();
					int yOff = rDis.bottom - pImg->GetHeight();
					::Rect rct( rDis.left + 1, yOff, pImg->GetWidth(), pImg->GetHeight());

					drawNonBmpImgTo( pImg, hDlg, IDC_FOURSQLOGO, rct );

					delete pImg; pImg = 0;
				}
				break;
*/
			}
			return TRUE;
		}
		break;
		case WM_COMMAND: {

			switch( lwp ) {
/*
				case IDC_FOURSQLOGO: {
				// Show Foursquare options dialog

					hwFourSqOptionsDlg = ::CreateDialog( ::GetModuleHandle( NULL ), MAKEINTRESOURCE( IDD_FOURSQCONFIGURE ), hDlg, getFourSqDialog );
					::SetFocus( hwFourSqOptionsDlg );

					return TRUE;
				}
				break;
*/
				case IDC_DEAUTHENTICATE: {
				// Clear auth token locally and in storage

					saveAccessToken( "" );
					toggleAuthStateControls( hDlg );

					// If deauthorizing, reset dependent controls and stored values for 
					// compatibility with the way config values are used when determining 
					// how to update feeds; in theory this shoul be handled by the control flow 
					// then, but it is otherwise fail-safe and mostly harmless. User will 
					// just have to reapply settings of dependent controls if authorized again:
					::CheckDlgButton( hDlg, IDC_ONLYSHOWLIKED, false );
					::CheckDlgButton( hDlg, IDC_ONLYINCLUDETAGGED, false );
					::CheckDlgButton( hDlg, IDC_DEFAULTBEHAVIOR, true );

					HKEY key;
					LONG result = ::RegCreateKeyEx( HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &key, NULL );
					if( result == ERROR_SUCCESS ) {
						::setRegInt( key, L"ExclusionMode", 0 );
					}
					::RegCloseKey( key );

					::LoadStringA( GetModuleHandle(NULL), IDS_INSTAUTHREMOVE_MESSAGE, msgBoxMessage, sizeof(msgBoxMessage) );
					::LoadStringA( GetModuleHandle(NULL), IDS_INSTAUTHREMOVE_CAPTION, msgBoxCaption, sizeof(msgBoxCaption) );

					::MessageBeep( MB_ICONASTERISK );
					::MessageBoxExA( hDlg, msgBoxMessage, msgBoxCaption, MB_OK | MB_ICONASTERISK | MB_APPLMODAL | MB_SETFOREGROUND, 0 );

					return TRUE;
				}
				break;
				case IDC_LOGIN: {
				// Request auth token and store on success, post message on failure

					// Cursor change
					::SetCursor( ::LoadCursor(NULL, IDC_WAIT) );

					// Fetch unap and build post data from string template:
					char username[INSTAGRAM_USERNAME_MAXLEN];
					::GetDlgItemTextA( hDlg, IDC_USERNAME, username, INSTAGRAM_USERNAME_MAXLEN );

					char password[INSTAGRAM_PASSWORD_MAXLEN];
					::GetDlgItemTextA( hDlg, IDC_PASSWORD, password, INSTAGRAM_PASSWORD_MAXLEN );
					::SetDlgItemTextA( hDlg, IDC_PASSWORD, "" );

					DWORD dwDataLen = (INSTAGRAM_AUTH_DATA_TPL.length()-(4*2)) + 
						strlen(username) + strlen(password) + INSTAGRAM_CLIENT_ID.length()  + INSTAGRAM_CLIENT_SECRET.length() + 1;
					char *data		= new char[dwDataLen];	

					sprintf_s( data, dwDataLen, INSTAGRAM_AUTH_DATA_TPL.c_str(), 
						username, password, INSTAGRAM_CLIENT_ID.c_str(), INSTAGRAM_CLIENT_SECRET.c_str() );
					
					// Send:

					DWORD dwError;
					char *response = NULL;

					::MswCHttp<char> *http = new ::MswCHttp<char>( VER_INTERNALNAME_STR, INSTAGRAM_AUTH_SERVER.c_str() );
					http->addHeader( "Content-type: application/x-www-form-urlencoded; charset=utf-8" );
					dwError = http->doPost( response, &INSTAGRAM_AUTH_PATH, data, true );
					delete http;
					delete []data;

					if ( response && dwError == ERROR_SUCCESS ) {
#ifdef _DEBUG
	OutputDebugStringA( response );
#endif
						::JsonTree jt( response );

						if ( jt.hasChild( "access_token" ) ) {
						// Save and toggle to deauth controls

							saveAccessToken( jt.getChild( "access_token").getValue<string>() );
							toggleAuthStateControls( hDlg );

							::LoadStringA( GetModuleHandle(NULL), IDS_INSTAUTHGOOD_MESSAGE, msgBoxMessage, sizeof(msgBoxMessage) );
							::LoadStringA( GetModuleHandle(NULL), IDS_INSTAUTHGOOD_CAPTION, msgBoxCaption, sizeof(msgBoxCaption) );

							::MessageBeep( MB_ICONINFORMATION );
							::MessageBoxExA( hDlg, msgBoxMessage, msgBoxCaption, MB_OK | MB_ICONINFORMATION | MB_APPLMODAL | MB_SETFOREGROUND, 0 );

						} else {
						// Show Instagram exception if available, else generic auth failure message

							::LoadStringA( GetModuleHandle(NULL), IDS_INSTAUTHFAIL_CAPTION, msgBoxCaption, sizeof(msgBoxCaption) );

							if ( jt.hasChild( "error_message" ) ) {
								strcpy_s( msgBoxMessage, sizeof(msgBoxMessage), jt.getChild( "error_message").getValue<string>().c_str() );
							} else {
								::LoadStringA( GetModuleHandle(NULL), IDS_INSTAUTHFAIL_DEFAULT, msgBoxMessage, sizeof(msgBoxMessage) );
							}

							getErrorMsgBox( hDlg );
						}

						delete []response;

					} else {
					// Something went wrong with the request altogether; show dialog, append error code

						::LoadStringA( GetModuleHandle(NULL), IDS_REQUESTFAIL_MESSAGE, msgBoxMessage, sizeof(msgBoxMessage) );
						::LoadStringA( GetModuleHandle(NULL), IDS_REQUESTFAIL_CAPTION, msgBoxCaption, sizeof(msgBoxCaption) );

						getErrorMsgBox( hDlg, dwError );
					}

					// Revert cursor back to an arrow
					::SetCursor( ::LoadCursor(NULL, IDC_ARROW) );

					return TRUE;
				}
				break;
				case IDC_ONLYINCLUDETAGGED :
				// Able related radio options on toggle; pass
				case IDC_ONLYSHOWLIKED : 
				// Able related radio options on toggle; pass
				case IDC_DEFAULTBEHAVIOR : {
				// Able related radio options on toggle:
					toggleDefaultModeControls( hDlg );
					toggleTaggedExclusiveModeControls( hDlg );

					return TRUE;
				}
				break;
				case IDC_CHECKFORUPDATE : {
				// Update display of control during update check

					using namespace boost::posix_time;
					
					// Only honor clicks here
					if ( hwp != STN_CLICKED )
						return FALSE;
					
					// If control repurposed to download new version link, open browser and exit
					if ( bDownloadUpdate ) {
						::ShellExecuteA( NULL, "open", DOWNLOAD_URL.c_str(), NULL, NULL, SW_SHOWNORMAL );
						return TRUE;
					}

					static const ptime EPOCH( boost::gregorian::date(1970, 1, 1) );

					// Cursor change
					::SetCursor( ::LoadCursor(NULL, IDC_WAIT) );

					HWND updateControl = ::GetDlgItem( hDlg, IDC_CHECKFORUPDATE );
					::EnableWindow( updateControl, false );
					bool reenableControl = false;

					// Get current version, store default action text for possible revert, update to checking state
					wchar_t curVersion[10];
					::LoadStringW( GetModuleHandle(NULL), IDS_UPDATECHECKVERSION, curVersion, sizeof(curVersion) );
					wchar_t msgAction[50];
					::GetDlgItemText( hDlg, IDC_CHECKFORUPDATE, msgAction, 50 );
					wchar_t msgPolling[50];
					::LoadStringW( GetModuleHandle(NULL), IDS_UPDATECHECKACTION, msgPolling, sizeof(msgPolling) );
					::SetDlgItemText( hDlg, IDC_CHECKFORUPDATE, msgPolling );
					
					// Send:

					DWORD dwError;
					char *response	= NULL;
					
					// Cache bust path
					char cNoCachePath[200];
					sprintf_s( cNoCachePath, 200, "%s?i=%u\0", UPDATEPOLL_PATH.c_str(), (microsec_clock::universal_time() - EPOCH).total_milliseconds() );
					::string sNoCachePath = cNoCachePath;

					::MswCHttp<char> *http = new ::MswCHttp<char>( VER_INTERNALNAME_STR, UPDATEPOLL_SERVER.c_str() );
					dwError = http->doGet( response, &sNoCachePath, false );
					delete http;

					if ( response && dwError == ERROR_SUCCESS ) {
#ifdef _DEBUG
	OutputDebugStringA( response );
#endif						
						wchar_t *stopwcs;
						char	*stopcs;
						if ( strtod( response, &stopcs ) > wcstod( curVersion, &stopwcs ) ) {
						// New version available, replace with download link

							wchar_t msgUpdateAvailable[300];
							::LoadStringW( GetModuleHandle(NULL), IDS_UPDATEAVAILABLE, msgUpdateAvailable, sizeof(msgUpdateAvailable) );
							::SetDlgItemText( hDlg, IDC_CHECKFORUPDATE, msgUpdateAvailable );

							bDownloadUpdate = reenableControl = true;
						} else {
						// User has current version, express this state

							wchar_t msgUpdateUnnecessary[300];
							::LoadStringW( GetModuleHandle(NULL), IDS_UPDATEUNNECESSARY, msgUpdateUnnecessary, sizeof(msgUpdateUnnecessary) );
							::SetDlgItemText( hDlg, IDC_CHECKFORUPDATE, msgUpdateUnnecessary );
						}

						delete []response;

					} else {

						::LoadStringA( GetModuleHandle(NULL), IDS_UPDATECHECKFAIL_MESSAGE, msgBoxMessage, sizeof(msgBoxMessage) );
						::LoadStringA( GetModuleHandle(NULL), IDS_UPDATECHECKFAIL_CAPTION, msgBoxCaption, sizeof(msgBoxCaption) );

						getErrorMsgBox( hDlg, dwError );
						
						// Revert to check for update action link
						::SetDlgItemText( hDlg, IDC_CHECKFORUPDATE, msgAction );

						reenableControl = true;
					}

					if ( reenableControl )
						::EnableWindow( updateControl, true );

					// Revert cursor back to an arrow
					::SetCursor( ::LoadCursor(NULL, IDC_ARROW) );

					return TRUE;
				}
				break;
				case IDOK: {
									
					HWND hwLastControl = ::GetFocus();

					// Process ENTER on password box, trigger auth and break
					if ( hwLastControl == ::GetDlgItem( hDlg, IDC_PASSWORD ) ) {
						return ::PostMessageA( hDlg, WM_COMMAND, IDC_LOGIN, 0 );
					} 

					// Save then fall through to cancel

					Configuration cfg;

					char username[INSTAGRAM_USERNAME_MAXLEN];
					::GetDlgItemTextA( hDlg, IDC_USERNAME, username, INSTAGRAM_USERNAME_MAXLEN );

					char tags[INSTAGRAM_TAGSLIST_MAXLEN];
					::GetDlgItemTextA( hDlg, IDC_TAGS, tags, INSTAGRAM_TAGSLIST_MAXLEN );

					cfg.username					= std::string( username ); 
					cfg.showUsernames				= ::IsDlgButtonChecked( hDlg, IDC_SHOWUSERNAMES ) == 1;
					cfg.includeTaggedPhotos			= ::IsDlgButtonChecked( hDlg, IDC_ONLYINCLUDETAGGED ) == 1;
					cfg.onlyIncludeLikedPhotos		= ::IsDlgButtonChecked( hDlg, IDC_ONLYSHOWLIKED ) == 1;
					cfg.photoTags					= std::string( tags );
					
					saveConfigMsw( cfg );
				} 
				// #nobreak
				case IDCANCEL: {
				// Post to destroy
/*
					if ( hwFourSqOptionsDlg )
						::PostMessageA( hwFourSqOptionsDlg, WM_CLOSE, 0, 0 );
*/
					return ::PostMessageA( hDlg, WM_CLOSE, 0, 0 );
				}
				break;
				default: {
					return TRUE;
				}
			};
		}		
		break;
		default: {
			return FALSE;
		}
	};

	return FALSE;
}

