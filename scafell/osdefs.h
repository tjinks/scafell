//
//  osdefs.h
//  scafell
//
//  Created by Tony on 16/06/2025.
//

#ifndef osdefs_h
#define osdefs_h

#ifdef WIN32
#include "os/oswin.h"
#endif

// TODO - also need to include this for linux
#ifdef __APPLE__
#include "os/osunix.h"
#endif


#endif /* osdefs_h */
