#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma pack(1)

struct BMPHeader {
	short type;           // File type = 0x4D42
	int size;
	short reserved1;
	short reserved2;
	int offset;           // Offset from file start to bitmap data
};

struct BMPInfoHeader {
	int size;             // Size of this structure in bytes
	int width;
	int height;
};

#pragma pack()

unsigned char *LoadBMPFile(const char *path,int *width,int *height) {
//для загрузки файла в формате bmp
	unsigned char *result = NULL;
	FILE *in = fopen(path, "rb");
	if (!in)
		return NULL;
	BMPHeader hdr;
	fread(&hdr, sizeof(hdr), 1, in);
	BMPInfoHeader	infoHdr;
	fread(&infoHdr, sizeof(infoHdr), 1, in);
	result = new unsigned char[infoHdr.width*infoHdr.height * 3];
	//перемещает указатель на указанный символ
	fseek(in, hdr.offset - sizeof(hdr) - sizeof(infoHdr), SEEK_CUR);
	unsigned char *dst = result;
	for (int y = 0; y<infoHdr.height; y++) {
		for (int x = 0; x<infoHdr.width; x++) {
			dst[2] = fgetc(in); //возвращает символ на который ссылается указанный поток
			dst[1] = fgetc(in);
			dst[0] = fgetc(in);
			dst += 3;
		}
		for (int x = 0; x<((4 - (infoHdr.width & 3)) & 3); x++)  // Skip alignment bytes
			fgetc(in);
	}
	fclose(in);
	*width = infoHdr.width;
	*height = infoHdr.height;
	return result;
}
