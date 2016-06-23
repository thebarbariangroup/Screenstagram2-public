#pragma once

#include "Configuration.h"

namespace cinder { namespace app {
	class AppScreenSaver;
} }

#if defined( __OBJC__ )

#import <Cocoa/Cocoa.h>

@interface ConfigWindowController : NSWindowController
{
  @public
	IBOutlet NSWindow			*window;
	IBOutlet NSMatrix           *optionsMatrix;
	IBOutlet NSTextField		*usernameField;
	IBOutlet NSSecureTextField	*passwordField;	
	IBOutlet NSButton			*authenticateButton;
	IBOutlet NSButton			*deauthenticateButton;	
	IBOutlet NSButton			*cancelButton;	
	IBOutlet NSButton           *tbgUrlButton;
	IBOutlet NSButton           *checkForUpgrade;    
	IBOutlet NSButton           *showUsernamesCheckbox;
	IBOutlet NSButton           *includeTaggedPhotosCheckbox;
	IBOutlet NSButton           *onlyIncludeLikedPhotosCheckbox;                    
    IBOutlet NSTextField        *photoTagsTextField;
	IBOutlet NSTextField        *versionNumber;
    NSString                    *currentVersion;
	NSMutableData				*xAuthResponseData;
	bool						isAuthenticated;
	
	Configuration					*config;
	cinder::app::AppScreenSaver		*app;
}


@property bool isAuthenticated;
- (void)authenticationChanged;
- (IBAction)optionSelected:(id)sender;
- (bool)isAuthenticated;
- (IBAction)authenticateClick:(id)sender;
- (IBAction)deauthenticateClick:(id)sender;
- (IBAction)cancelClick:(id)sender;
- (IBAction)doneClick:(id)sender;
- (IBAction)checkForUpgrade:(id)sender;
- (IBAction)showUsernamesClick:(id)sender;
- (void)showAuthFailedMessage;
- (void)showAuthSuccessMessage;
- (void)saveInfo:(NSString *)token;
- (IBAction)tbgUrlClicked:(id)sender;
- (IBAction)includeTaggedPhotosClick:(id)sender;
- (void)sheetDidEnd:(NSWindow *)sheet
         returnCode:(int)returnCode
        contextInfo:(void  *)contextInfo;

@end

#endif // defined( __OBJC__ )

extern NSWindow* getConfigDialogMac( cinder::app::AppScreenSaver *app, Configuration *outputConfig );
extern void		loadConfigMac( cinder::app::AppScreenSaver *app, Configuration *config );