//
//  NSObject+BindingValue.m
//  Tags
//
//  Created by Sven Weidauer on 10.06.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "NSObject+BindingValue.h"


@implementation NSObject (BindingValue)

static NSValueTransformer *TransformerFromInfoDict( NSDictionary *dict )
{
    NSDictionary *options = dict[NSOptionsKey];
    if (options == nil) return nil;
    
    NSValueTransformer *transformer = options[NSValueTransformerBindingOption];
    
    if (transformer == nil || (id)transformer == [NSNull null]) {
        transformer = nil;
        NSString *name = options[NSValueTransformerNameBindingOption];
        if (name != nil && (id)name != [NSNull null]) {
            transformer = [NSValueTransformer valueTransformerForName: name];   
        }
    }
    
    return transformer;
}

- (void) setValue: (id) value forBinding: (NSString *) binding;
{
    NSDictionary *info = [self infoForBinding: binding];
    NSValueTransformer *transformer = TransformerFromInfoDict( info );
    if (transformer != nil && [[transformer class] allowsReverseTransformation]) value = [transformer reverseTransformedValue: value];
    [info[NSObservedObjectKey] setValue: value forKeyPath: info[NSObservedKeyPathKey]];
}

- (id) valueForBinding: (NSString *) binding;
{
    NSDictionary *info = [self infoForBinding: binding];
    id value = [info[NSObservedObjectKey] valueForKeyPath: info[NSObservedKeyPathKey]];
    NSValueTransformer *transformer = TransformerFromInfoDict( info );
    if (transformer != nil) value = [transformer transformedValue: value];
    return value;
}

- (id) mutableArrayValueForBinding: (NSString *) binding;
{
    NSDictionary *info = [self infoForBinding: binding];
    return [info[NSObservedObjectKey] mutableArrayValueForKeyPath: info[NSObservedKeyPathKey]];
}

- (id) mutableSetValueForBinding: (NSString *) binding;
{
    NSDictionary *info = [self infoForBinding: binding];
    return [info[NSObservedObjectKey] mutableSetValueForKeyPath: info[NSObservedKeyPathKey]];
}


@end
