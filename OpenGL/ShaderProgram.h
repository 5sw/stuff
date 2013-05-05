//
//  ShaderProgram.h
//  gles2
//
//  Created by Sven Weidauer on 08.07.12.
//  Copyright (c) 2012 Sven Weidauer. All rights reserved.
//

#import <Foundation/Foundation.h>
@class Shader;

@interface ShaderProgram : NSObject

@property (readonly, nonatomic) GLuint programObject;
@property (readonly, nonatomic) NSArray *shaders;
@property (readonly, nonatomic) NSString *infoLog;

- (void) attachShader: (Shader *)shader;

- (BOOL) link;

+ (ShaderProgram *)programWithShaders: (Shader *)shader, ... NS_REQUIRES_NIL_TERMINATION;

- (GLint)locationForAttribute: (NSString *)attribute;
- (void) use;

@end
