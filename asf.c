#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

uint8_t *buffer;
uint8_t *cursor;

#define BIT( n ) (1 << n)

typedef enum {
	GUIDUnknown,
	GUIDHeader,
	GUIDData,
	GUIDFileProperties,
	GUIDStreamProperties,
	GUIDHeaderExtension,
	GUIDMediaTypeAudio,
	GUIDMediaTypeVideo,
	GUIDErrorCorrectionNone,
	GUIDErrorCorrectionAudioSpread,
} GUIDType;


typedef struct GUIDEntry {
	GUIDType type;
	uint8_t data[16];
} GUIDEntry;

GUIDEntry guids[] = {
	GUIDHeader, { 0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C },
	GUIDData, { 0x36, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C },
	GUIDFileProperties, { 0xA1, 0xDC, 0xAB, 0x8C, 0x47, 0xA9, 0xCF, 0x11, 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 },
	GUIDStreamProperties, { 0x91, 0x07, 0xDC, 0xB7, 0xB7, 0xA9, 0xCF, 0x11, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 },
	GUIDHeaderExtension, { 0xB5, 0x03, 0xBF, 0x5F, 0x2E, 0xA9, 0xCF, 0x11, 0x8E, 0xE3, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 },
	GUIDMediaTypeAudio, { 0x40, 0x9E, 0x69, 0xF8, 0x4D, 0x5B, 0xCF, 0x11, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B },
	GUIDMediaTypeVideo, { 0xC0, 0xEF, 0x19, 0xBC, 0x4D, 0x5B, 0xCF, 0x11, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B },
	GUIDErrorCorrectionNone, { 0x00, 0x57, 0xFB, 0x20, 0x55, 0x5B, 0xCF, 0x11, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B },
	GUIDErrorCorrectionAudioSpread, { 0x50, 0xCD, 0xC3, 0xBF, 0x8F, 0x61, 0xCF, 0x11, 0x8B, 0xB2, 0x00, 0xAA, 0x00, 0xB4, 0xE2, 0x20 },
};

GUIDType ReadGUID()
{
	uint8_t *start = cursor;
	cursor += 16;
	
	for (size_t i = 0; i < sizeof guids / sizeof guids[0]; i++) {
		if (memcmp( start, guids[i].data, 16) == 0) {
			return guids[i].type;
		}
	}
	return GUIDUnknown;
}

uint8_t ReadByte()
{
	uint8_t result = *cursor;
	cursor++;
	return result;
}

uint16_t ReadWord()
{
	return ReadByte() | (uint16_t)ReadByte() << 8;
}

uint32_t ReadDWord()
{
	return ReadWord() | (uint32_t)ReadWord() << 16;
}

uint64_t ReadQWord()
{
	return ReadDWord()  | (uint64_t)ReadDWord() << 32;
}
uint32_t packetSize = 0;

void PrintGUID()
{
	printf( "%04X-%02X-%02X-", ReadDWord(), ReadWord(), ReadWord() );
	printf( "%02X%02X-", ReadByte(), ReadByte() );
	printf( "%02X%02X%02X%02X%02X%02X\n", ReadByte(), ReadByte(), ReadByte(), ReadByte(), ReadByte(), ReadByte() );
}

void ParseFileProperties()
{
	printf( "\t\tFile ID:       " ); PrintGUID();
	printf( "\t\tFile size:     %llu\n", ReadQWord() );
	printf( "\t\tCreation date: %llu\n", ReadQWord() );
	printf( "\t\tPacket count:  %llu\n", ReadQWord() );
	printf( "\t\tPlay duration: %llu\n", ReadQWord() );
	printf( "\t\tSend duration: %llu\n", ReadQWord() );
	printf( "\t\tPreroll:       %llu\n", ReadQWord() );
	printf( "\t\tFlags:         %u\n", ReadDWord() );
	packetSize = ReadDWord() ;
	printf( "\t\tMin Pack. size %u\n", packetSize );
	uint32_t maxPacketSize = ReadDWord();
	printf( "\t\tMax Pack. size %u\n", maxPacketSize  );
	printf( "\t\tMax bitrate    %u\n", ReadDWord() );
	printf( "\tend file properties\n" );
}

typedef enum StreamType {
	StreamTypeError,
	StreamTypeVideo,
	StreamTypeAudio,
} StreamType;

typedef enum ErrorCorrectionType {
	ErrorCorrectionTypeNone,
	ErrorCorrectionTypeAudioSpread,
} ErrorCorrectionType;

typedef struct AudioMediaInfo {
	uint16_t codecId;
	uint16_t channelCount;
	uint32_t samplesPerSecond;
	uint32_t bytesPerSecond;
	uint16_t blockAlignment;
	uint16_t bitsPerSample;
	uint16_t specificDataSize;
	uint8_t specificData[0];
} __attribute__((packed)) AudioMediaInfo;

typedef struct BitmapInfo {
	uint32_t formatDataSize;
	int32_t width;
	int32_t height;
	uint16_t reserved;
	uint16_t bitsPerPixel;
	union {
		uint8_t compression[4];
		uint32_t compressionId;
	};
	uint32_t imageSize;
	int32_t horizontalPixelsPerMeter;
	int32_t verticalPixelsPerMeter;
	uint32_t colorsUsedCount;
	uint32_t importantColorsCount;
	uint8_t codecSpecificData[0];
} __attribute__((packed)) BitmapInfo;

typedef struct VideoMediaInfo {
	uint32_t imageWidth;
	uint32_t imageHeight;
	uint8_t flags;
	uint16_t formatDataSize;
	union {
		uint8_t formatData[0];
		BitmapInfo bitmapInfo;
	};
} __attribute__((packed)) VideoMediaInfo;

typedef struct Stream {
	StreamType type;
	ErrorCorrectionType ecType;
	size_t specificDataLength;
	size_t ecDataLength;
	uint64_t timeOffset;
	union {
		uint8_t *specificData;
		AudioMediaInfo *audioInfo;
		VideoMediaInfo *videoInfo;
	};
	uint8_t *ecData;
	bool encrypted;
} Stream;

Stream *streams = NULL;
size_t streamCount = 0;

StreamType ReadStreamType()
{
	GUIDType guid = ReadGUID();
	switch (guid) {
		case GUIDMediaTypeAudio:
			return StreamTypeAudio;
			
		case GUIDMediaTypeVideo:
			return StreamTypeVideo;
			
		default:
			printf( "Found invalid stream type\n" );
			return StreamTypeError;
	}
}

ErrorCorrectionType ReadErrorCorrectionType()
{
	GUIDType guid = ReadGUID();
	switch (guid) {
		case GUIDErrorCorrectionNone:
			return ErrorCorrectionTypeNone;
			
		case GUIDErrorCorrectionAudioSpread:
			return ErrorCorrectionTypeAudioSpread;
			
		default:
			printf( "Found invalid error correction type\n" );
			return ErrorCorrectionTypeNone;
	}
}

void DumpAudioInfo( const AudioMediaInfo *info )
{
	printf( "\t\tAudio info:\n" );
	printf( "\t\t\tCodec ID:              0x%04X\n", info->codecId );
	printf( "\t\t\tNumber of channels:    %d\n", info->channelCount );
	printf( "\t\t\tSamples per second:    %d\n", info->samplesPerSecond );
	printf( "\t\t\tAvg. bytes per second: %d\n", info->bytesPerSecond );
	printf( "\t\t\tBlock alignment:       %d\n", info->blockAlignment );
	printf( "\t\t\tBits per sample:       %d\n", info->bitsPerSample );
	printf( "\t\t\tSpecific data size:    %d\n", info->specificDataSize );
}

void DumpVideoInfo( const VideoMediaInfo *info )
{
	printf( "\t\tVideo info:\n" );
	printf( "\t\t\tFrame size:             %dx%d\n", info->imageWidth, info->imageHeight );
	printf( "\t\t\tFlags:                  %d\n", info->flags );
	printf( "\t\t\tFormat data size:       %d\n", info->formatDataSize );
	printf( "\t\t\tSize:                   %dx%d\n", info->bitmapInfo.width, info->bitmapInfo.height );
	printf( "\t\t\tBits per pixel:         %d\n", info->bitmapInfo.bitsPerPixel );
	printf( "\t\t\tCodec:                  %c%c%c%c\n", info->bitmapInfo.compression[0], info->bitmapInfo.compression[1], info->bitmapInfo.compression[2], info->bitmapInfo.compression[3] );
	printf( "\t\t\tImage size:             %d\n", info->bitmapInfo.imageSize );
	printf( "\t\t\tH Pixels per meter:     %d\n", info->bitmapInfo.horizontalPixelsPerMeter );
	printf( "\t\t\tV Pixels per meter:     %d\n", info->bitmapInfo.verticalPixelsPerMeter );
	printf( "\t\t\tColors used count:      %d\n", info->bitmapInfo.colorsUsedCount );
	printf( "\t\t\tImportant colors count: %d\n", info->bitmapInfo.importantColorsCount );
}

void ParseStreamProperties()
{
	StreamType type = ReadStreamType();
	printf( "\t\tStream type:          " ); 
	switch (type) {
		case StreamTypeAudio: printf( "Audio\n" ); break;
		case StreamTypeVideo: printf( "Video\n" ); break;
		default: printf( "Error, invalid\n" ); break;
	}
	
	ErrorCorrectionType ecType = ReadErrorCorrectionType();
	printf( "\t\tEC type:              " ); 
	switch (ecType) {
		case ErrorCorrectionTypeNone: printf( "None\n" ); break;
		case ErrorCorrectionTypeAudioSpread: printf( "Audio spread\n" ); break;
	}
	
	uint64_t timeOffset = ReadQWord();
	printf( "\t\tTime offset:          %llu\n", timeOffset );
	
	size_t specificLength = ReadDWord();
	printf( "\t\tSpecific data length: %zu\n", specificLength );
	
	size_t ecDataLength = ReadDWord();
	printf( "\t\tEC data length:       %zu\n", ecDataLength );
	
	uint16_t flags = ReadWord();

	cursor += 4; // Skip reserved
	
	uint8_t streamNumber = flags & (BIT( 0 ) | BIT( 1 ) | BIT( 2 ) | BIT( 3 ) | BIT( 4 ) | BIT( 5 ) | BIT( 6 ) | BIT( 7 ));
	printf( "\t\tStream number:        %u\n", streamNumber );
	
	bool encrypted = (flags & BIT( 15 )) != 0;
	printf( "\t\tEncrypted:            %s\n", encrypted ? "yes" : "no" );
	
	if (streamNumber > streamCount) {
		printf( "\t\tGrowing stream count from %zu to %d\n", streamCount, streamNumber );
		streamCount = streamNumber;
		streams = realloc( streams, streamCount * sizeof (Stream) );
	}
	
	streams[streamNumber - 1].type = type;
	streams[streamNumber - 1].ecType = ecType;
	streams[streamNumber - 1].specificDataLength = specificLength;
	streams[streamNumber - 1].ecDataLength = ecDataLength;
	streams[streamNumber - 1].specificData = cursor;
	streams[streamNumber - 1].ecData = cursor + specificLength;
	streams[streamNumber - 1].timeOffset = timeOffset;
	streams[streamNumber - 1].encrypted = encrypted;
	
	switch (type) {
		case StreamTypeAudio:
			DumpAudioInfo( streams[streamNumber - 1].audioInfo );
			break;
			
		case StreamTypeVideo:
			DumpVideoInfo( streams[streamNumber - 1].videoInfo );
			break;
			
		default:
			break;
	}
}

void ParseHeaderExtension()
{
	cursor += 16; // Skip reserved field 1
	cursor += 2; // Skip reserved field 2
	uint16_t dataSize = ReadDWord();
	printf( "\tHeader extension data size: %d\n", dataSize );
	uint8_t *end = cursor + dataSize;
	while (cursor < end) {
		uint8_t *objectStart = cursor;
		GUIDType type = ReadGUID();
		uint64_t size = ReadQWord();
		uint8_t *next = objectStart + size;
		switch (type) {
			case GUIDUnknown:
			default:
				printf( "Unknown extended header object\n" );
				break;
		}
		cursor = next;
	}
}

void ParseHeaders() 
{
	printf( "Begin parsing headers\n" );
	uint32_t headerCount = ReadDWord();
	cursor += 2;
	printf( "\t%d headers\n\n", headerCount );
	for (uint32_t i = 0; i < headerCount; i++) {
		uint8_t *objectStart = cursor;
		GUIDType type = ReadGUID();
		uint64_t size = ReadQWord();
		uint8_t *next = objectStart + size;
		printf( "\tOffset %ld, Object size: %llu\n", objectStart - buffer, size );
		switch (type) {
			case GUIDFileProperties:
				printf( "\tFile properties found...\n" );
				ParseFileProperties();
				break;

			case GUIDStreamProperties:
				printf( "\tStream properties found...\n" );
				ParseStreamProperties();
				break;

			case GUIDHeaderExtension:
				printf( "\tHeader extension found...\n" );
				ParseHeaderExtension();
				break;
			
			case GUIDUnknown:
			default:
				printf( "\tUnknown GUID\n" );
				break;
		}
		cursor = next;
		printf( "\n" );
	}
	printf( "end parsing headers\n" );
}

void ParseFile() 
{
	for (;;) {
		uint8_t *objectStart = cursor;
		GUIDType type = ReadGUID();
		uint64_t size = ReadQWord();
		uint8_t *next = objectStart + size;
		printf( "Object size: %llu\n", size );
		switch (type) {
			case GUIDHeader:
				printf( "Header GUID\n" );
				ParseHeaders();
				break;

			case GUIDData:
				printf( "Found Data object\n" );
				return;
				
			case GUIDUnknown:
			default:
				printf( "Unknown GUID\n" );
				break;
				
		}
		cursor = next;
	}
}


uint32_t ReadNumber( int type )
{
	switch (type) {
		case 1: return ReadByte();
		case 2: return ReadWord();
		case 3: return ReadDWord();

		default:
		case 0: return 0;
	}
}

typedef struct ParseInfo {
	int repDataLengthType;
	int offsetLengthType;
	int objectNumberLengthType;
} ParseInfo;

typedef struct MediaObject {
	size_t size;
	uint8_t *data;
	uint32_t presentationTime;
	uint32_t sequenceNumber;
	size_t nextOffset;
	size_t object;
} MediaObject;

MediaObject *currentObjects = NULL;
uint8_t *packetStart = NULL;

void AddDataToObject( MediaObject *o, size_t offset, size_t len, const uint8_t *data )
{
	printf( "Offset: %zu\n", offset );
	printf( "Next offset: %zu\n", o->nextOffset );
	printf( "Len: %zu\n", len );
	
	if (o->nextOffset + len >= o->size) {
		printf( "Error: Packet too big\n" );
		return;
	}
	
	memcpy( o->data + o->nextOffset, data, len );
	o->nextOffset += len;
}

void ParseECData( uint8_t flags )
{
	printf( "\tParsing EC data...\n" );
	int ecDataLength = flags & (BIT(0)|BIT(1)|BIT(2)|BIT(3));
	printf( "\tEC data length:                      %d\n", ecDataLength );
	bool opaque = flags & BIT(4);
	printf( "\tOpaque data present:                 %s\n", opaque ? "yes" : "no" );
	int ecLengthType = (flags & (BIT( 5 ) | BIT( 6 ))) >> 5;
	printf( "\tEC length type:                      %d\n", ecLengthType );
	printf( "\tSkipping EC data...\n");
	cursor += ecDataLength;
}

void ParsePayload( const ParseInfo *info )
{
	printf( "\tParse payload...\n" );
	uint8_t tmp = ReadByte();
	
	uint8_t streamNumber = tmp & (BIT( 0 ) | BIT( 1 ) | BIT( 2 ) | BIT( 3 ) | BIT( 4 ) | BIT( 5 ) | BIT( 6 ));
	bool isKeyFrame = (tmp & BIT( 7 )) != 0;
	
	uint32_t mediaObjectNumber = ReadNumber( info->objectNumberLengthType );
	uint32_t offset = ReadNumber( info->offsetLengthType );
	uint32_t repLength = ReadNumber( info->repDataLengthType );
	
	printf( "\t\tStream number:                   %d\n", streamNumber );
	printf( "\t\tKey frame:                       %s\n", isKeyFrame ? "yes" : "no" );
	printf( "\t\tMedia object number:             %d\n", mediaObjectNumber );
	printf( "\t\tOffset into object:              %d\n", offset );
	printf( "\t\tReplicated data length:          %d\n", repLength );
	
	if (repLength >= 8) {
		printf( "\t\tReplicated data:\n" );
		printf( "\t\t\tMedia object size:           %d\n", ReadDWord() );
		printf( "\t\t\tPresentation time:           %d\n", ReadDWord() );
		printf( "\t\tSkipping rest of replicated data...\n" );
		cursor += repLength - 8;
	} else {
		cursor += repLength;
	}
	
	size_t headerSize = cursor - packetStart;
	size_t dataSize = packetSize - headerSize - 0;
	printf( "\t\tData offset: %ld\n", cursor - buffer );
	AddDataToObject( &currentObjects[streamNumber], offset, dataSize, cursor );
}

void ParseMultiplePayloads( const ParseInfo *info )
{
	printf( "\tParse multiple payloads...\n" );
}

void ParsePayloadHeader( uint8_t flags )
{
	printf( "\tParsing payload parsing information...\n" );
	bool multiplePayloads = (flags & BIT( 0 )) != 0;
	int sequenceType = (flags & (BIT( 1 ) | BIT( 2 ))) >> 1;
	int paddingLengthType = (flags & (BIT( 3 ) | BIT( 4 ))) >> 3;
	int packetLengthType = (flags & (BIT( 5 ) | BIT( 6 ))) >> 5;
	bool ecPresent = (flags & BIT( 7 )) != 0;
	
	printf( "\tMultiple Payloads                    %s\n", multiplePayloads ? "yes" : "no" );
	printf( "\tSequence type:                       %d\n", sequenceType );
	printf( "\tPadding length type:                 %d\n", paddingLengthType );
	printf( "\tPacket length type:                  %d\n", packetLengthType );
	printf( "\tEC present:                          %s\n", ecPresent ? "yes" : "no" );
	
	uint8_t propertyFlags = ReadByte();
	
	ParseInfo info;
	info.repDataLengthType = propertyFlags & (BIT( 0 ) | BIT( 1 ));
	info.offsetLengthType = (propertyFlags & (BIT( 2 ) | BIT( 3 ))) >> 2;
	info.objectNumberLengthType = (propertyFlags & (BIT( 4 ) | BIT( 5 ))) >> 4;
	int streamNumberLengthType = (propertyFlags & (BIT( 6 ) | BIT( 7 ))) >> 6;
	
	printf( "\tRepl. data length type               %d\n", info.repDataLengthType );
	printf( "\tOffset into media object length type %d\n", info.offsetLengthType );
	printf( "\tMedia object number lengtht type     %d\n", info.objectNumberLengthType );
	printf( "\tStream Number Length Type            %d\n", streamNumberLengthType );
	
	uint32_t packetLength = ReadNumber( packetLengthType );
	printf( "\tPacket length:                       %d\n", packetLength );
	
	uint32_t sequence = ReadNumber( sequenceType );
	printf( "\tSequence number:                     %d\n", sequence );
	
	uint32_t paddingLength = ReadNumber( paddingLengthType );
	printf( "\tPadding length:                      %d\n", paddingLength );
	
	uint32_t sendTime = ReadDWord();
	printf( "\tSend time:                           %d\n", sendTime );
	
	uint16_t duration = ReadWord();
	printf( "\tDuration:                            %d\n", duration );
	
	if (multiplePayloads) ParseMultiplePayloads( &info );
	else ParsePayload( &info );
}

void ParsePacket()
{
	packetStart = cursor;
	printf( "Parsing packet...\n" );
	uint8_t flags = ReadByte();
	if (flags & BIT( 7 )) {
		ParseECData( flags );
		flags = ReadByte();
		ParsePayloadHeader( flags );
	} else {
		ParsePayloadHeader( flags );
	}
}

uint8_t *end = 0;

void ParseData()
{
	currentObjects = calloc( sizeof (MediaObject), streamCount );
	printf( "Data begins...\n" );
	printf( "File ID:       " ); PrintGUID();
	printf( "Total packets: %llu\n", ReadQWord() );
	cursor += 2; // Skip reserved field
	while (cursor < end) {
		uint8_t *tmp = cursor;
		ParsePacket();
		cursor = tmp + packetSize;
	}
}

int main( int argc, char **argv )
{
	int handle = open( argv[1], O_RDONLY );
	struct stat buf;
	fstat( handle, &buf );
	cursor = buffer = mmap( NULL, buf.st_size, PROT_READ, MAP_FILE|MAP_PRIVATE, handle, 0 );
	end = buffer + buf.st_size;
	ParseFile();
	ParseData();
}