//
//  SmallIntSet.h
//  JSParser
//
//  Created by Sven Weidauer on 06.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <unistd.h>

typedef uint64_t SmallIntSet_t;
#define SMALLINTSET_MAX_VALUE ((sizeof(SmallIntSet_t) * CHAR_BIT) - 1)
#define SMALLINTSET_EMPTY ((SmallIntSet_t)0)

#define PURE __attribute__ (( pure ))

PURE static inline SmallIntSet_t SmallIntSetWithValue( uint8_t value )
{
    assert( 0 <= value && value <= SMALLINTSET_MAX_VALUE );
    return ((SmallIntSet_t)1) << value;
}

PURE static inline SmallIntSet_t SmallIntSetByAddingValue( SmallIntSet_t set, uint8_t value )
{
    return set | SmallIntSetWithValue( value );
}

PURE static inline SmallIntSet_t SmallIntSetByRemovingValue( SmallIntSet_t set, uint8_t value )
{
    return set & ~SmallIntSetWithValue( value );
}

PURE static inline bool SmallIntSetContainsValue( SmallIntSet_t set, uint8_t value )
{
    return (set & SmallIntSetWithValue( value )) != 0;
}

PURE SmallIntSet_t SmallIntSetWithValues( size_t count, ... );