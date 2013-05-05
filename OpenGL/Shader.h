//
//  Shader.h
//  gles2
//
//  Created by Sven Weidauer on 08.07.12.
//  Copyright (c) 2012 Sven Weidauer. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Shader : NSObject

@property (readonly, nonatomic) GLenum shaderType;
@property (readonly, nonatomic) GLuint shaderObject;
@property (readonly, nonatomic) NSString *infoLog;

+ (Shader *)vertexShaderNamed: (NSString *)name;
+ (Shader *)fragmentShaderNamed: (NSString *)name;
+ (Shader *)shaderNamed:(NSString *)name type: (GLenum) type;

- initWithType: (GLenum)type;

- (BOOL) compileSource: (NSString *)source;
- (BOOL) compileSourceData: (NSData *)data;

@end
