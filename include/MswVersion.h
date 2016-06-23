#ifndef MSWVERSION_H
#define MSWVERSION_H

#define EXP_VERSION(ver)			#ver
#define MSW_VERSION(ver)			EXP_VERSION(ver)
#define GET_MSW_VERSION				MSW_VERSION(VER_UPDATECHECK_STR)

// Version and app details for the update check and Win OS property pages:

// Required:

#define VER_UPDATECHECK_STR			"2.0"

#define VER_FILEVERSION				2,0,0,0
#define VER_PRODUCTVERSION			2,0,0,0
#define VER_FILEVERSION_STR			"1, 0, 0, 0\0"
#define VER_PRODUCTVERSION_STR		"2, 0, 0, 0\0"

#define VER_PRODUCTNAME_STR			"Screenstagram 2\0"
#define VER_INTERNALNAME_STR		"Screenstagram2\0"
#define VER_FILEDESCRIPTION_STR		"Screenstagram 2\0"
#define VER_COMPANYNAME_STR			"The Barbarian Group\0"
#define VER_ORIGINALFILENAME_STR	"Screenstagram2.scr\0"

// Optional:

#define VER_LEGALCOPYRIGHT_STR		"© 2012 The Barbarian Group, LLC.\0"
#define VER_LEGALTRADEMARKS1_STR	"\0"
#define VER_LEGALTRADEMARKS2_STR	"\0"

#endif
