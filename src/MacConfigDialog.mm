#import "MacConfigDialog.h"
#import <ScreenSaver/ScreenSaver.h>

#include "cinder/app/AppScreenSaver.h"

// NOTE: It is important that your NIB has its Window's "Visible at Launch" set to FALSE

static ConfigWindowController *sController = nil;



void loadConfigMac( cinder::app::AppScreenSaver *app, Configuration *config )
{
	NSBundle *ssaverBundle = app->getBundle();
	ScreenSaverDefaults *defaults = [ScreenSaverDefaults defaultsForModuleWithName:[ssaverBundle bundleIdentifier]];
	
	[defaults registerDefaults:[NSDictionary dictionaryWithObjectsAndKeys:
                                @"", @"AccessToken",
                                @"", @"Username",
                                [NSNumber numberWithBool:NO], @"ShowUsernames",
                                [NSNumber numberWithBool:YES], @"IncludePopularPhotos",
                                [NSNumber numberWithBool:NO], @"IncludeTaggedPhotos",
                                [NSNumber numberWithBool:NO], @"OnlyIncludeLikedPhotos",                                
                                @"", @"PhotoTags",
                
                   nil]];
    
    config->showUsernames               = [defaults boolForKey:@"ShowUsernames"];
    config->includePopularPhotos        = [defaults boolForKey:@"IncludePopularPhotos"];
    config->includeTaggedPhotos         = [defaults boolForKey:@"IncludeTaggedPhotos"];

    config->onlyIncludeLikedPhotos      = [defaults boolForKey:@"OnlyIncludeLikedPhotos"];    

    config->accessToken                 = [[defaults stringForKey:@"AccessToken"] UTF8String];
    config->username                    = [[defaults stringForKey:@"Username"] UTF8String];
    config->photoTags                   = [[defaults stringForKey:@"PhotoTags"] UTF8String];

    
				   
}

NSWindow* getConfigDialogMac( cinder::app::AppScreenSaver *app, Configuration *config )
{	
    
    

	if( ! sController ) {
		sController = [[ConfigWindowController alloc] initWithWindowNibName:@"SettingsSheet"];
        NSLog(@"No sController, loaded new instance");
	}
	sController->config = config;
	sController->app = app;

	loadConfigMac( app, config );
	
	return [sController window];
}

void saveConfigMac( cinder::app::AppScreenSaver *app, const Configuration *config )
{
	NSBundle *ssaverBundle = app->getBundle();
	ScreenSaverDefaults *defaults = [ScreenSaverDefaults defaultsForModuleWithName:[ssaverBundle bundleIdentifier]];
        
    NSLog(config->showUsernames ? @"Yes" : @"No");

    
	[defaults setBool:config->showUsernames 
               forKey:@"ShowUsernames"];        
	[defaults setBool:config->includePopularPhotos 
               forKey:@"IncludePopularPhotos"];            
	[defaults setBool:config->includeTaggedPhotos 
               forKey:@"IncludeTaggedPhotos"];                    

	[defaults setBool:config->onlyIncludeLikedPhotos 
               forKey:@"OnlyIncludeLikedPhotos"];                            
    
    [NSString stringWithCString:config->accessToken.c_str() encoding:[NSString defaultCStringEncoding]];
    
	[defaults setObject:[NSString stringWithCString:config->accessToken.c_str() encoding:[NSString defaultCStringEncoding]]
               forKey:@"AccessToken"];                            
	[defaults setObject:[NSString stringWithCString:config->username.c_str() encoding:[NSString defaultCStringEncoding]] 
                 forKey:@"Username"];   
	[defaults setObject:[NSString stringWithCString:config->photoTags.c_str() encoding:[NSString defaultCStringEncoding]] 
                 forKey:@"PhotoTags"];   

    

	// Save the settings to disk
	[defaults synchronize];
}




@interface ConfigWindowController ()

@end



@implementation ConfigWindowController

@synthesize isAuthenticated;


- (IBAction)showUsernamesClick:(id)sender {
    config->showUsernames = [sender state] == NSOnState;
}


// Outlet for when any of the options in the options matrix are selected.
- (IBAction)optionSelected:(id)sender {
    
    if([[optionsMatrix selectedCell] tag] == 0){ // Show feed if logged in.
        NSLog(@"Show my feed");
        
        [photoTagsTextField setEnabled:NO];
        [photoTagsTextField setStringValue:@""];
        
        config->includePopularPhotos = true;
        config->includeTaggedPhotos = false;
        config->onlyIncludeLikedPhotos = false;
    } else if([[optionsMatrix selectedCell] tag] == 1){ // Only show Liked (if logged in)
        NSLog(@"Show Liked Photos");
        
    
        [photoTagsTextField setEnabled:NO];
        [photoTagsTextField setStringValue:@""];        
        
        config->includePopularPhotos = false;
        config->includeTaggedPhotos = false;
        config->onlyIncludeLikedPhotos = true;        
    } else if ([[optionsMatrix selectedCell] tag] == 2){ // Only show tagged (if logged in)
        NSLog(@"Show Tagged Photos");
        
        [photoTagsTextField setEnabled:YES];
        
        config->onlyIncludeLikedPhotos = false;
        config->includePopularPhotos = false;
        config->includeTaggedPhotos = true;
    } else {
        NSLog(@"Unknow matrix option.");
    }
}


// Just opens up the version number on our S3 account.
-(IBAction)checkForUpgrade:(id)sender {
    
    NSURL * url = [NSURL URLWithString:[NSString stringWithFormat: @"http://screenstagram.s3.amazonaws.com/version%@.html", currentVersion]];
    [[NSWorkspace sharedWorkspace] openURL:url];
    
}

-(IBAction)authenticateClick:(id)sender
{
	
	[authenticateButton setEnabled:NO];
    
    NSURL * url = [NSURL URLWithString:@"https://api.instagram.com/oauth/access_token"];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
	[request setHTTPMethod:@"POST"];	
	NSString *postString = [NSString stringWithFormat:@"username=%@&password=%@&grant_type=password&client_id=c54e61b2f7554ca5bd7ce61bb371cffb&client_secret=d6c736a4fb7b4e4abe37b382ba73d127", 
                            [usernameField stringValue],
                            [passwordField stringValue]
                            ];
	[request setHTTPBody:[postString dataUsingEncoding:NSUTF8StringEncoding]];	
	NSURLConnection *conn = [[NSURLConnection alloc] initWithRequest:request delegate:self];	
	[conn start];		
    
}

-(IBAction)deauthenticateClick:(id)sender
{	
    config->accessToken = [@"" UTF8String];
    config->username = [@"" UTF8String];
    [usernameField setStringValue:@""];    
    [passwordField setStringValue:@""];
	isAuthenticated = false;
	[self authenticationChanged];
}

-(IBAction)cancelClick:(id)sender
{	
    [[NSApplication sharedApplication] endSheet:[sController window]];
}


-(IBAction)doneClick:(id)sender {
    [[NSApplication sharedApplication] endSheet:[sController window]];
    
    config->photoTags = [[photoTagsTextField stringValue] UTF8String];

    saveConfigMac( app, config ); 
}
- (void)saveInfo:(NSString *)token {
    

    config->accessToken = [token UTF8String];
    config->username    = [[usernameField stringValue] UTF8String];
    
	isAuthenticated = true;
	[self authenticationChanged];
    saveConfigMac( app, config );    
}

- (void)authenticationChanged {
	if(isAuthenticated){
        [[optionsMatrix cellWithTag:0] setState:YES];
        [[optionsMatrix cellWithTag:1] setEnabled:YES];
        [[optionsMatrix cellWithTag:2] setEnabled:YES];
		[authenticateButton setEnabled:NO];
		[deauthenticateButton setEnabled:YES];
		[passwordField setEnabled:NO];
		[passwordField setStringValue:@""];
		[usernameField setEnabled:NO];
	} else {
        [[optionsMatrix cellWithTag:0] setState:YES];
        [[optionsMatrix cellWithTag:1] setEnabled:NO];
        [[optionsMatrix cellWithTag:1] setState:NO];
        [[optionsMatrix cellWithTag:2] setEnabled:NO];
        [[optionsMatrix cellWithTag:2] setState:NO];
        [photoTagsTextField setEnabled:NO];
        [photoTagsTextField setStringValue:@""];
        
		[authenticateButton setEnabled:YES];
		[deauthenticateButton setEnabled:NO];		
		[passwordField setEnabled:YES];
		[usernameField setEnabled:YES];
	}
}


- (id)initWithWindow:(NSWindow *)windowP
{
    self = [super initWithWindow:windowP];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

#pragma mark -
#pragma mark TBGOAuth2Client auth callback methods

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
	[xAuthResponseData appendData:data];
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error {
    
	NSLog(@"Connection Error: %@", [NSString stringWithFormat:@"Connection failed: %@", [error description]]);
	[xAuthResponseData release];
	xAuthResponseData = [[NSMutableData alloc] init];	
	isAuthenticated = false;
	[self authenticationChanged];	
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection {
	NSString * response = [[NSString alloc] initWithData:xAuthResponseData encoding:NSUTF8StringEncoding];	
    //	id obj = [response JSONValue];
    
    std::string json = [response UTF8String];
    ci::JsonTree doc( json );
    
    if(doc.hasChild("access_token")){
        NSString * accessToken = [NSString stringWithCString:doc.getChild("access_token").getValue().c_str() encoding:NSUTF8StringEncoding];
        [self saveInfo:accessToken];
        [self showAuthSuccessMessage];
        isAuthenticated = true;
        [self authenticationChanged];	        
    } else {
        [self showAuthFailedMessage];
        NSLog(@"Oauth ok but no token");
        isAuthenticated = false;
        [self authenticationChanged];	
    }
    
	[xAuthResponseData release];
	xAuthResponseData = [[NSMutableData alloc] init];
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response {
    //	NSLog(@"response: %i", [response statusCode]);
}

- (void)showAuthSuccessMessage {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setAlertStyle:NSInformationalAlertStyle];
    [alert setMessageText:@"Authentication Succeeded"];
    [alert setInformativeText:@"You're all set!"];
    [alert runModal];
    [alert release];
}

- (void)showAuthFailedMessage {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setAlertStyle:NSInformationalAlertStyle];
    [alert setMessageText:@"Authentication Failed"];
    [alert setInformativeText:@"This username/password combination didn't work. Try again?"];
    [alert runModal];
    [alert release];
}

- (IBAction)tbgUrlClicked:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://barbariangroup.com/"]];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    NSBundle *ssaverBundle = app->getBundle();
    currentVersion = [[ssaverBundle infoDictionary] objectForKey:@"CFBundleVersion"];
    [versionNumber setStringValue:[NSString stringWithFormat:@"Version %@", currentVersion]];
    
	xAuthResponseData = [[NSMutableData alloc] init];
    
	isAuthenticated = false;
    
    // This'll help us figure out if we're already authenticated.
    if(config->accessToken.length() > 2){
        isAuthenticated = true;
    }           
    
	[self authenticationChanged];    
    
    
    [showUsernamesCheckbox setState:config->showUsernames];



    // Popular
    [[optionsMatrix cellWithTag:0] setState:config->includePopularPhotos];
    // onlyIncludeLikedPhotos
    [[optionsMatrix cellWithTag:1] setState:config->onlyIncludeLikedPhotos];
    // tagged
    [[optionsMatrix cellWithTag:2] setState:config->includeTaggedPhotos];
    if(config->includeTaggedPhotos){
        [photoTagsTextField setEnabled:YES];
    }

    
    [usernameField setStringValue:[NSString stringWithCString:config->username.c_str() encoding:[NSString defaultCStringEncoding]]];
    [photoTagsTextField setStringValue:[NSString stringWithCString:config->photoTags.c_str() encoding:[NSString defaultCStringEncoding]]];
    
}





@end
