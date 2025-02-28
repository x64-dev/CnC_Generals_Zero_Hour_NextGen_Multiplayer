#ifndef __J2K__Boolean_HPP__
#define __J2K__Boolean_HPP__

#ifdef DOS
    #define bool           unsigned int      // If bool type doesn't exist
    #define false          0 
    #define true           1 
#endif

#ifndef _WINDEF_

    #define FALSE  false    // Define CAPITAL bool constant
    #define TRUE   true

    #define BOOL   bool
    
#endif

#endif
