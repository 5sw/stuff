//
//  ShaderProgram.m
//  gles2
//
//  Created by Sven Weidauer on 08.07.12.
//  Copyright (c) 2012 Sven Weidauer. All rights reserved.
//

#import "ShaderProgram.h"
#import "Shader.h"

@interface ShaderProgram ()

@property (strong, nonatomic) NSMutableArray *shaderArray;

@end

@implementation ShaderProgram

- (id)init;
{
	self = [super init];
	if (!self) return nil;

	_programObject = glCreateProgram();
	if (_programObject == 0) return nil;

	_shaderArray = [NSMutableArray array];

	return self;
}

- (NSArray *)shaders;
{
	return [self.shaderArray copy];
}

- (void)dealloc;
{
	if (_programObject != 0) {
		glDeleteProgram( _programObject );
	}
}

- (void)attachShader:(Shader *)shader;
{
	[self.shaderArray addObject: shader];
	glAttachShader( self.programObject, shader.shaderObject );
}

- (BOOL)link;
{
	glLinkProgram( self.programObject );

	GLint linkStatus = 0;
	glGetProgramiv( self.programObject, GL_LINK_STATUS, &linkStatus );

	return linkStatus == GL_TRUE;
}

- (NSString *)infoLog;
{
	GLint logLength = 0;
	glGetProgramiv( self.programObject, GL_INFO_LOG_LENGTH, &logLength );

	if (logLength == 0) return nil;

	char buffer[logLength];

	GLsizei length = 0;
	glGetProgramInfoLog( self.programObject, logLength, &length, buffer );

	return [[NSString alloc] initWithBytes: buffer length: length encoding: NSUTF8StringEncoding];
}

+ (ShaderProgram *)programWithShaders:(Shader *)shader, ...;
{
	ShaderProgram *result = [[self alloc] init];

	va_list args;
	va_start( args, shader );
	while (shader) {
		[result attachShader: shader];
		shader = va_arg( args, Shader * );
	}
	va_end( args );

	if (![ result link]) {
		NSLog( @"Error linking shader program: %@", result.infoLog );
		return nil;
	}

	return result;
}

- (GLint)locationForAttribute:(NSString *)attribute;
{
	return glGetAttribLocation( self.programObject, [attribute cStringUsingEncoding: NSASCIIStringEncoding] );
}

- (void)use;
{
	glUseProgram( self.programObject );
}

@end
