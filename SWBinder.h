//
//  SWBinder.h
//
//  Created by Sven Weidauer on 20.01.15.
//  Copyright (c) 2015 Sven Weidauer. All rights reserved.
//

@import Foundation;

/**
 *  Block typed used to transform values.
 *
 *  @param value The value to transform
 *  @return The transformed value.
 */
typedef id (^SWBinderTransformationBlock)(id value);

/**
 *  Establishes a binding from the source object to the target object using KVO.
 *  Each time the value for a given key path of the source object is changed
 *  the corresponding property of the target object is updated (again via a
 *  key path).
 *  Changed means that a different value is assigned, reassigning the current 
 *  value would not trigger an update of the target object.
 *  This automatically handels all memory management. While both the source and
 *  target objects are living it observes the source and updates the target. As
 *  soon as either one gets deallocated the binder stops observing the source
 *  and gets cleaned up. That means in most cases it is not necessary to keep
 *  a reference to the Binder around.
 *  You only need to keep a reference to the binder around if you want to stop
 *  observing the source object before it or the target object gets deallocated.
 *  This class is not meant to be subclassed.
 */
@interface SWBinder : NSObject

/**
 *  Established a binding from a source to an target object.
 *
 *  @param source The source object to observe
 *  @param sourceKeyPath The key path of the source object to observe.
 *  @param target The target object to update
 *  @param targetKeyPath The keypath for the property of the target object
 *      to update.
 *  @return The new binder object.
 */
+ (instancetype)bindFromObject: (id)source keyPath: (NSString *)sourceKeyPath
                      toObject: (id)target keyPath: (NSString *)targetKeyPath;

/**
 *  Established a binding from a source to an target object with an optional
 *  transformation.
 *
 *  @param source The source object to observe
 *  @param sourceKeyPath The key path of the source object to observe.
 *  @param target The target object to update
 *  @param targetKeyPath The keypath for the property of the target object
 *      to update.
 *  @param block A block that can translate the source values to target values.
 *      This may be @c nil if no translation is necessary.
 *  @return The new binder object.
 */
+ (instancetype)bindFromObject: (id)source keyPath: (NSString *)sourceKeyPath
                      toObject: (id)target keyPath: (NSString *)targetKeyPath
                transformation: (SWBinderTransformationBlock)block;

/**
 *  Established a binding from a source to an target object with a value
 *  transformer.
 *
 *  @param source The source object to observe
 *  @param sourceKeyPath The key path of the source object to observe.
 *  @param target The target object to update
 *  @param targetKeyPath The keypath for the property of the target object
 *      to update.
 *  @param transformer A value transformer used to translate the values
 *      from the source to target representation.
 *  @return The new binder object.
 */
+ (instancetype)bindFromObject: (id)source keyPath: (NSString *)sourceKeyPath
                      toObject: (id)target keyPath: (NSString *)targetKeyPath
              valueTransformer: (NSValueTransformer *)transformer;

/**
 *  Established a binding from a source to an target object with a value
 *  transformer used in the reverse direction.
 *
 *  @param source The source object to observe
 *  @param sourceKeyPath The key path of the source object to observe.
 *  @param target The target object to update
 *  @param targetKeyPath The keypath for the property of the target object
 *      to update.
 *  @param transformer A value transformer used to translate the values
 *      from the source to target representation. The transformer needs to
 *      allow the reverse transformation.
 *  @return The new binder object.
 */
+ (instancetype)bindFromObject: (id)source keyPath: (NSString *)sourceKeyPath
                      toObject: (id)target keyPath: (NSString *)targetKeyPath
            reverseTransformer: (NSValueTransformer *)transformer;

/**
 *  Breaks the binding. This removes the observer from the source object and
 *  releases all resources. After this has been sent this Binder will never do
 *  anything again and should be released.
 */
- (void)unbind;

@end
