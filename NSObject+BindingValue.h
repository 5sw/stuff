//
//  NSObject+BindingValue.h
//  Tags
//
//  Created by Sven Weidauer on 10.06.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface NSObject (BindingValue)

- (void) setValue: (id) value forBinding: (NSString *) binding;
- (id) valueForBinding: (NSString *) binding;

- (id) mutableArrayValueForBinding: (NSString *) binding;
- (id) mutableSetValueForBinding: (NSString *) binding;

@end
