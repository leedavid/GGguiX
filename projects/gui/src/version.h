#pragma once



#define VERSION_MAJOR   1
#define VERSION_MINOR   5
#define REVISION        4

#define XGLUE2(a,b) #a "," #b
#define GLUE2(a,b)  XGLUE2(a,b)

#define XGLUE4(a,b,c,d) #a "," #b "," #c "," #d
#define GLUE4(a,b,c,d)  XGLUE4(a,b,c,d)

#define XGLUEDOT3(a,b,c) #a "." #b "." #c
#define GLUEDOT3(a,b,c)  XGLUEDOT3(a,b,c)

#define STR_VERSION     GLUE2(VERSION_MAJOR,VERSION_MINOR)
#define STR_REVISION    GLUE4(VERSION_MAJOR,VERSION_MINOR,REVISION,0)

#define STR_VERSION_NET GLUEDOT3(VERSION_MAJOR,VERSION_MINOR,REVISION)

#define COPYRIGHT_DATE "(C) 2016"
#define COPYRIGHT "Copyright(C) 2016"
