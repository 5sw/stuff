//
//  Shader.m
//  gles2
//
//  Created by Sven Weidauer on 08.07.12.
//  Copyright (c) 2012 Sven Weidauer. All rights reserved.
//

#import "Shader.h"

@implementation Shader

- (id)initWithType:(GLenum)type;
{
	self = [super init];
	if (!self) return nil;

	_shaderType = type;
	_shaderObject = glCreateShader( type );
	if (_shaderObject == 0) return nil;

	return self;
}

- (void)dealloc;
{
	if (_shaderObject != 0) {
		glDeleteShader( _shaderObject );
	}
}

- (BOOL)compileSource:(NSString *)source;
{
	return [self compileSourceData: [source dataUsingEncoding: NSASCIIStringEncoding]];
}

- (BOOL)compileSourceData:(NSData *)data;
{
	const GLchar *string = [data bytes];
	GLint length = [data length];

	glShaderSource( self.shaderObject, 1, &string, &length );

	glCompileShader( self.shaderObject );

	GLint compileStatus = 0;
	glGetShaderiv( self.shaderObject, GL_COMPILE_STATUS, &compileStatus );

	return compileStatus == GL_TRUE;
}

- (NSString *)infoLog;
{
	GLint logLength = 0;
	glGetShaderiv( self.shaderObject, GL_INFO_LOG_LENGTH, &logLength );

	if (logLength == 0) return nil;

	char buffer[logLength];

	GLsizei actualLength = 0;
	glGetShaderInfoLog( self.shaderObject, logLength, &actualLength, buffer );

	return [[NSString alloc] initWithBytes: buffer length: actualLength encoding: NSUTF8StringEncoding];
}

+ (Shader *)shaderNamed:(NSString *)name type: (GLenum) type;
{
	NSURL *url = [[NSBundle mainBundle] URLForResource: name withExtension: @"glsl"];
	NSData *programData = [NSData dataWithContentsOfURL: url];
	if (!programData) return nil;

	Shader *result = [[self alloc] initWithType: type];
	if (![result compileSourceData: programData]) {
		NSLog( @"Error compiling shader: %@", result.infoLog );
		return nil;
	}

	return result;
}

+ (Shader *)vertexShaderNamed:(NSString *)name;
{
	return [self shaderNamed: name type: GL_VERTEX_SHADER];
}

+ (Shader *)fragmentShaderNamed:(NSString *)name;
{
	return [self shaderNamed: name type: GL_FRAGMENT_SHADER];
}

@end
