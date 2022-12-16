/* BMP文件解码  */

typedef struct BITHEADER{
    unsigned short Type;
    unsigned int Size;
    unsigned int Reservation;
    unsigned int Start;
}BMPHEADER;

typedef struct BITINFO{
    unsigned int DIB;
    unsigned int Width;
    unsigned int Height;
    unsigned short PLANENUM;
    unsigned short Bit;
    unsigned int COMPRESSION;
    unsigned int SIZE;
    unsigned int HORINZONTAL;
    unsigned int VERTICAL;
    unsigned int TIAOSEPAN; //不知道怎么命名了，看的中文BMP格式讲解，这里写的调色盘颜色;
    unsigned int IMPROTABTCOL;
}BMPINFOHEADER;
 
//streching 
void WriteBMP(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* buffer, const char* filename);

void CrtGrey(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* buffer, unsigned char* bufferG);

void Streching(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* buffer, unsigned char* bufferS);

//fft

void Split(struct _complex *src, struct _complex *dst, int x, int y, int mode);

void fft_x(struct _complex *src, struct _complex *dst, int num, int mode);

void fft_y(struct _complex *src, struct _complex *dst, int num, int mode);

void fft(struct _complex *src, struct _complex *dst, int mode);

void fft_shift(struct _complex *src);

void GetNewSize(BMPHEADER* Fileheader, BMPINFOHEADER* InfoHeader);

void pre_fft(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* buffer, struct _complex* newF, struct _complex* F);

void WriteF(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, struct _complex* buffer, const char* filename);


//filter
void HighPass(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, struct _complex* src);

void LowPass(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, struct _complex* src);

void BandPass(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, struct _complex* src);


//sobel
void pre_sobel(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* buffer, unsigned char* news);

void sobelx(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* src, unsigned char* srcx);

void sobely(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* src, unsigned char* srcy);

void sobel(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* news, unsigned char* srcx, unsigned char* srcy);