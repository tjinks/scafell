//
//  osdefs.h
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#ifndef osdefs_h
#define osdefs_h

#ifdef WIN32
#include "oswin.h"
#endif

#ifdef __APPLE__
#include "osunix.h"
#endif


#endif /* osdefs_h */
