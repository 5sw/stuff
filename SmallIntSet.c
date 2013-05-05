//
//  SmallIntSet.c
//  JSParser
//
//  Created by Sven Weidauer on 06.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "SmallIntSet.h"

#include <stdarg.h>

SmallIntSet_t SmallIntSetWithValues( size_t count, ... )
{
    va_list args;
    va_start( args, count );
    
    SmallIntSet_t result = SMALLINTSET_EMPTY;
    for (size_t i = 0; i < count; ++i) {
        uint8_t value = va_arg( args, uint8_t );
        result |= SmallIntSetWithValue( value );
    }
    
    va_end( args );
    
    return result;
}