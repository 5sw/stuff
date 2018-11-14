//
//  Binder.m
//  Grind
//
//  Created by Sven Weidauer on 20.01.15.
//  Copyright (c) 2015 Sven Weidauer. All rights reserved.
//

#import "SWBinder.h"

@import ObjectiveC;

/** 
 *  Helper function that adds a binder to an object so that it gets unbound 
 *  before the object is deallocated.
 *  
 *  @param objct The object whose lifetime should be observed.
 *  @param biner The binder to unbind before @c object is deallocated.
 */
static inline void AddBinder( id object, SWBinder *binder );

/**
 *  Helper function that removes the lifetime observation from an object.
 *
 *  @param object The object whose lifetime is observed
 *  @param binder The binder that is observing the object.
 */
static inline void RemoveBinder( id object, SWBinder *binder );

@implementation SWBinder {
    // Normally I wouldn't use instance variables directly as I do here.
    // It's OK here since this is not supposed to be subclassed and the
    // binder objects should be considered immutable.

    // __unsafe_unretained so that we still have this reference while the
    // object is being deallocated so we can remove the observer.
    __unsafe_unretained id _source;
    NSString *_sourceKeyPath;

    __unsafe_unretained id _target;
    NSString *_targetKeyPath;

    SWBinderTransformationBlock _transform;
}

+ (instancetype)bindFromObject: (id)source keyPath: (NSString *)sourceKeyPath
                      toObject: (id)target keyPath: (NSString *)targetKeyPath
              valueTransformer: (NSValueTransformer *)transformer;
{
    NSParameterAssert( transformer );

    SWBinderTransformationBlock block = ^( id value ) {
        return [transformer transformedValue: value];
    };

    return [self bindFromObject: source keyPath: sourceKeyPath
                       toObject: target keyPath: targetKeyPath
                 transformation: block];
}

+ (instancetype)bindFromObject: (id)source keyPath: (NSString *)sourceKeyPath
                      toObject: (id)target keyPath: (NSString *)targetKeyPath
            reverseTransformer: (NSValueTransformer *)transformer;
{
    NSParameterAssert( transformer );
    NSAssert( [transformer.class allowsReverseTransformation], @"Reverse transformation needed for %@", NSStringFromSelector( _cmd ) );

    SWBinderTransformationBlock block = ^( id value ) {
        return [transformer reverseTransformedValue: value];
    };

    return [self bindFromObject: source keyPath: sourceKeyPath
                       toObject: target keyPath: targetKeyPath
                 transformation: block];
}


+ (instancetype)bindFromObject:(id)source keyPath:(NSString *)sourceKeyPath
                      toObject:(id)target keyPath:(NSString *)targetKeyPath
{
    return [self bindFromObject: source keyPath: sourceKeyPath
                       toObject: target keyPath: targetKeyPath
                 transformation: nil];
}

+ (instancetype)bindFromObject:(id)source keyPath:(NSString *)sourceKeyPath
                      toObject:(id)target keyPath:(NSString *)targetKeyPath
                transformation:(SWBinderTransformationBlock)block
{
    return [[self alloc] initWithSource: source keyPath: sourceKeyPath
                                 target: target keyPath: targetKeyPath
                         transformation: block];
}

- (instancetype)initWithSource:(id)source keyPath:(NSString *)sourceKeyPath
                        target:(id)target keyPath:(NSString *)targetKeyPath
                transformation:(SWBinderTransformationBlock)block;
{
    NSParameterAssert( source && sourceKeyPath && target && targetKeyPath );

    self = [super init];
    if (!self) return nil;

    _source = source;
    _sourceKeyPath = [sourceKeyPath copy];

    _target = target;
    _targetKeyPath = [targetKeyPath copy];

    _transform = [block copy];

    [source addObserver: self forKeyPath: _sourceKeyPath
                options: NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld
                context: NULL];

    AddBinder( _target, self );
    AddBinder( _source, self );

    return self;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object
                        change:(NSDictionary *)change context:(void *)context
{
    id target = _target;

    id newValue = change[NSKeyValueChangeNewKey];
    id previousValue = change[NSKeyValueChangeOldKey];

    if (newValue == previousValue || [newValue isEqual: previousValue]) {
        // The value did not change, so  we won't update the target. This
        // prevents endless cycles when doing bidirectional bindings.
        return;
    }

    if ([newValue isEqual: [NSNull null]]) {
        newValue = nil;
    }

    if (_transform) {
        newValue = _transform( newValue );
    }

    [target setValue: newValue forKeyPath: _targetKeyPath];
}

- (void)unbind
{
    id source = _source;
    if (source) {
        _source = nil;

        [source removeObserver: self forKeyPath: _sourceKeyPath];

        RemoveBinder( source, self );
    }

    _sourceKeyPath = nil;

    id target = _target;
    if (target) {
        _target = nil;

        RemoveBinder( target, self );
    }

    _targetKeyPath = nil;

    _transform = nil;
}

- (void)dealloc
{
    [self unbind];
}

@end


/** 
 *  Helper object. Unbinds a binder in its dealloc method. Used for observing
 *  the lifetime of source and target objects.
 */
@interface SWBinderAutoRemoveHelper_ : NSObject

/**
 *  Designated initializer.
 *  @param binder The binder to unbind in dealloc.
 */
- (instancetype)initWithBinder:(SWBinder *)binder NS_DESIGNATED_INITIALIZER;

/**
 *  Removes the reference to the binder.
 */
- (void)stop;

@end

static inline void AddBinder( id object, SWBinder *binder )
{
    objc_setAssociatedObject( object, (__bridge const void *)binder, [[SWBinderAutoRemoveHelper_ alloc] initWithBinder: binder], OBJC_ASSOCIATION_RETAIN_NONATOMIC );
}

static inline void RemoveBinder( id object, SWBinder *binder )
{
    [(SWBinderAutoRemoveHelper_ *)objc_getAssociatedObject( object, (__bridge const void *)binder ) stop];
    objc_setAssociatedObject( object, (__bridge const void *)binder, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC );
}

@implementation SWBinderAutoRemoveHelper_ {
    SWBinder *_binder;
}

- (instancetype)initWithBinder:(SWBinder *)binder;
{
    NSParameterAssert( binder );

    self = [super init];
    if (!self) return nil;

    _binder = binder;

    return self;
}

- (void)dealloc
{
    [_binder unbind];
}

- (void)stop
{
    _binder = nil;
}

@end
