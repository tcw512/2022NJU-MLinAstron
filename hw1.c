#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "include/myfilter.h"

double p[255] = {0}; //均衡化,计数器和概率都由这个计算所以用double
int NWidth, NHeight; //为了使fft 2^n
struct _complex* F, * newF; //频域数据

int main(){
    BMPHEADER FileHeader;
    BMPINFOHEADER InfoHeader;
    FILE *fp = fopen("result/test.bmp", "rb");
    if (fp == NULL){
        perror("File open unsuccessfully");
        exit(1);
    }
    fread(&(FileHeader.Type), 2, 1, fp);
    fread(&(FileHeader.Size), 4, 1, fp);
    fread(&(FileHeader.Reservation), 4, 1, fp);
    fread(&(FileHeader.Start), 4, 1, fp);
    fread(&InfoHeader, sizeof(InfoHeader), 1, fp);
    unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * (FileHeader.Size - 54));
    memset(buffer, 0x00, FileHeader.Size - 54);
    fread(buffer, FileHeader.Size - 54, 1, fp);
    if (FileHeader.Type != 0x4D42){
        perror("Not BMP");
        exit(1);
    }
    fclose(fp);
    printf("Mode Choose:(如果是彩色图，务必执行灰度转换后切换读取的文件，在执行滤波器之前务必进行频域转换)\n");
    printf("1 - Crt2Grey         \t2 - Streching\t3 - CrtFrequency\t4 - Idea Highpass\t5 - Butterworth Lowpass\n");
    printf("6 - Gaussian Bandpass\t7 - Sobel\n");
    printf("q - quit\n");
    char argv;
    while(argv = getchar()){
        switch(argv){
            case '1':
                unsigned char* bufferG = (unsigned char*)malloc(sizeof(unsigned char) * (InfoHeader.SIZE -54));
                memset(bufferG, 0x00, FileHeader.Size - 54);
                CrtGrey(&FileHeader, &InfoHeader, buffer, bufferG);
                free(bufferG);
                system("cd result & test.bmp & equalization.bmp");
                break;
            case '2':
                unsigned char* bufferS = (unsigned char*)malloc(sizeof(unsigned char) * (InfoHeader.SIZE -54));
                Streching(&FileHeader, &InfoHeader, buffer, bufferS);
                free(bufferS);
                system("cd result & stretching.bmp");
                break;

            case '3':
                //宽和高不满足2^n
                GetNewSize(&FileHeader, &InfoHeader);
                F = (struct _complex*)malloc(sizeof(struct _complex) * (FileHeader.Size - 54));
                newF = (struct _complex*)malloc(sizeof(struct _complex) * NWidth * NHeight);
                pre_fft(&FileHeader, &InfoHeader, buffer, F, newF);
                free(F);
                fft(newF, newF, 1);
                WriteF(&FileHeader, &InfoHeader, newF, "result/frequency.bmp");

                fft(newF, newF, -1);
                WriteF(&FileHeader, &InfoHeader, newF, "result/-frequency.bmp");
                break;
            
            case '4':
                GetNewSize(&FileHeader, &InfoHeader);
                F = (struct _complex*)malloc(sizeof(struct _complex) * (FileHeader.Size - 54));
                newF = (struct _complex*)malloc(sizeof(struct _complex) * NWidth * NHeight);
                pre_fft(&FileHeader, &InfoHeader, buffer, F, newF);
                free(F);
                fft(newF, newF, 1);
                HighPass(&FileHeader, &InfoHeader, newF);
                WriteF(&FileHeader, &InfoHeader, newF, "result/HighPassF.bmp");

                fft(newF, newF, -1);
                WriteF(&FileHeader, &InfoHeader, newF, "result/HighPass.bmp");
                free(newF);
                system("cd result & HighPassF.bmp & HighPass.bmp");
                break;

            case '5':
                GetNewSize(&FileHeader, &InfoHeader);
                F = (struct _complex*)malloc(sizeof(struct _complex) * (FileHeader.Size - 54));
                newF = (struct _complex*)malloc(sizeof(struct _complex) * NWidth * NHeight);
                pre_fft(&FileHeader, &InfoHeader, buffer, F, newF);
                free(F);
                fft(newF, newF, 1);
                LowPass(&FileHeader, &InfoHeader, newF);
                WriteF(&FileHeader, &InfoHeader, newF, "result/LowPassF.bmp");

                fft(newF, newF, -1);
                WriteF(&FileHeader, &InfoHeader, newF, "result/LowPass.bmp");
                free(newF);
                system("cd result & LowPassF.bmp & LowPass.bmp");
                break;

            case '6':
                GetNewSize(&FileHeader, &InfoHeader);
                F = (struct _complex*)malloc(sizeof(struct _complex) * (FileHeader.Size - 54) / 3);
                newF = (struct _complex*)malloc(sizeof(struct _complex) * NWidth * NHeight);
                pre_fft(&FileHeader, &InfoHeader, buffer, F, newF);
                free(F);
                fft(newF, newF, 1);
                BandPass(&FileHeader, &InfoHeader, newF);
                WriteF(&FileHeader, &InfoHeader, newF, "result/BandPassF.bmp");
                fft(newF, newF, -1);

                WriteF(&FileHeader, &InfoHeader, newF, "result/BandPass.bmp");
                free(newF);
                system("cd result & BandPassF.bmp & BandPass.bmp");
                break;

            case '7':
                GetNewSize(&FileHeader, &InfoHeader);
                unsigned char* news = (unsigned char*)malloc(sizeof(unsigned char) * NWidth * NHeight);
                pre_sobel(&FileHeader, &InfoHeader, buffer, news);
                unsigned char* srcx = (unsigned char*)malloc(sizeof(unsigned char) * NWidth * NHeight);
                unsigned char* srcy = (unsigned char*)malloc(sizeof(unsigned char) * NWidth * NHeight);
                sobel(&FileHeader, &InfoHeader, news, srcx, srcy);
                free(news);
                system("cd result & sobel.bmp");
                break;

            case 'q':
                return 0;

            default:
                break;

        }
    }
    
    free(buffer);


    return 0; 
}

void CrtGrey(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* buffer, unsigned char* bufferG){
    int NewWidth = (InfoHeader->Width * InfoHeader->Bit / 8 + 3) / 4 * 4; //16进制行数，保证行数为4的倍数
    for (int i = 0; i < InfoHeader->Height; i++){
        for (int j = 0; j < NewWidth - 2; j += 3){
            unsigned char g = (buffer[i * NewWidth + j] * 467 + buffer[i * NewWidth + j + 1] * 2405 + buffer[i * NewWidth + j + 2] * 1224) >> 12;
            bufferG[i * NewWidth + j] = g;
            bufferG[i * NewWidth + j + 1] = g;
            bufferG[i * NewWidth + j + 2] = g;
            p[g]++;
        }
    }
    WriteBMP(FileHeader, InfoHeader, bufferG, "result/test.bmp");
    //均衡化
    for (int i = 0; i < 256; i++){
        p[i] /= (InfoHeader->Height * InfoHeader->Width);
        if (i > 0){
            p[i] += p[i - 1];
        }
    }

    unsigned char color[256] = {0};
    for (int i = 0; i < 256; i++){
        color[i] = (unsigned char)(p[i] * 255);
    }
    for (int i = 0; i < InfoHeader->Height; i++){
        for (int j = 0; j < NewWidth - 2; j += 3){
            bufferG[i * NewWidth + j] = color[bufferG[i * NewWidth + j]];
            bufferG[i * NewWidth + j + 1] = color[bufferG[i * NewWidth + j + 1]];
            bufferG[i * NewWidth + j + 2] = color[bufferG[i * NewWidth + j + 2]];
        }
    }
    WriteBMP(FileHeader, InfoHeader, bufferG, "result/equalization.bmp");
}

void Streching(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* buffer, unsigned char* bufferS){
    int NewWidth = (InfoHeader->Width * InfoHeader->Bit / 8 + 3) / 4 * 4; 
    unsigned char Min = 255;
    unsigned char Max = 0;
    for (int i = 0; i < InfoHeader->Height * (NewWidth - 2); i += 3){ //像素信息3格
        Max = Max < buffer[i] ? buffer[i] : Max;
        Min = Min > buffer[i] ? buffer[i] : Min;
    }
    printf("%d %d ", Max, Min);
     for (int i = 0; i < InfoHeader->Height; i++){
        for (int j = 0; j < NewWidth - 2; j += 3){
            unsigned char pix = 255 * (buffer[i * NewWidth + j] - Min) / (Max - Min);
            bufferS[i * NewWidth + j] = pix;
            bufferS[i * NewWidth + j + 1] = pix;
            bufferS[i * NewWidth + j + 2] = pix;
        }
    }
    WriteBMP(FileHeader, InfoHeader, bufferS, "result/Streching.bmp");
}

void WriteBMP(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* buffer, const char* filename){
    FILE *new = fopen(filename, "wb");
    fwrite(&(FileHeader->Type), 2, 1, new);
    fwrite(&(FileHeader->Size), 4, 1, new);
    fwrite(&(FileHeader->Reservation), 4, 1, new);
    fwrite(&(FileHeader->Start), 4, 1, new);
    fwrite(InfoHeader, 40, 1, new);
    fwrite(buffer, FileHeader->Size - 54, 1, new);

    fclose(new);
}

//fft

//fft分奇偶
//0x 1y
void Split(struct _complex *src, struct _complex *dst, int x, int y, int mode){
    struct _complex* tmp = (struct _complex*)malloc(sizeof(struct _complex) * (mode == 0 ? x/2 : y/2));

    if (mode == 0){
        if (x < 2) return;

        for (int i = 0; i < x / 2; i++){
            tmp[i].x = src[i * 2 + 1].x;
            tmp[i].y = src[i * 2 + 1].y;

            dst[i].x = src[i * 2].x;
            dst[i].y = src[i * 2].y;
        }

        for (int i = 0; i < x / 2; i++){
            dst[i + x / 2].x = tmp[i].x;
            dst[i + x / 2].y = tmp[i].y;
        }
    }else{
        if (y < 2) return;
        for (int i = 0; i < y / 2; i++){
            tmp[i].x = src[(i * 2 + 1) * NWidth].x;
            tmp[i].y = src[(i * 2 + 1) * NWidth].y;

            dst[i * NWidth].x = src[(i * 2) * NWidth].x;
            dst[i * NWidth].y = src[(i * 2) * NWidth].y;
        }

        for (int i = 0; i < y / 2; i++){
            dst[(i + y / 2) * NWidth].x = tmp[i].x;
            dst[(i + y / 2) * NWidth].y = tmp[i].y;
        }
    }
}

//mode 确定是傅里叶变换还是逆变换
void fft_x(struct _complex *src, struct _complex *dst, int num, int mode){
    struct _complex w, a0, a1, t;
    double x;

    if (num == 1) return;
    Split(src, dst, num, 0, 0);
    fft_x(dst, dst, num / 2, mode);
    fft_x(dst + num / 2, dst + num / 2, num / 2, mode);

    for (int i = 0; i < num / 2; i++){
        x = -1 * 2 * M_PI * i / num;  //旋转因子
        w.x = cos(x);
        w.y = mode * sin(x);

        a0 = dst[i];
        a1 = dst[i + num / 2];

        t.x = a1.x * w.x - a1.y * w.y;
        t.y = a1.y * w.x + a1.x * w.y;

        dst[i].x = a0.x + t.x;
        dst[i].y = a0.y + t.y;

        dst[i + num / 2].x = a0.x - t.x;
        dst[i + num / 2].y = a0.y - t.y;
    }
}

void fft_y(struct _complex *src, struct _complex *dst, int num, int mode){
    struct _complex w, a0, a1, t;
    double x;

    if (num == 1) return;
    Split(src, dst, 0, num, 1);
    fft_y(dst, dst, num / 2, mode);
    fft_y(dst + NWidth * (num / 2), dst + NWidth * (num / 2), num / 2, mode);

    for (int i = 0; i < num / 2; i++){
        x = -1 * 2 * M_PI * i / num;
        w.x = cos(x);
        w.y = mode * sin(x);

        a0 = dst[i * NWidth];
        a1 = dst[(i + num / 2) * NWidth];

        t.x = a1.x * w.x - a1.y * w.y;
        t.y = a1.y * w.x + a1.x * w.y;

        dst[i * NWidth].x = a0.x + t.x;
        dst[i * NWidth].y = a0.y + t.y;

        dst[(i + num / 2) * NWidth].x = a0.x - t.x;
        dst[(i + num / 2) * NWidth].y = a0.y - t.y;
    }

}

void fft(struct _complex *src, struct _complex *dst, int mode){
    if (mode == -1)
        fft_shift(src);

    for (int i = 0; i < NHeight; i++){
        fft_x(src + i * NWidth, dst + i * NWidth, NWidth, mode);
        if (mode == 1){   //为了控制像素取值
            for (int j = 0; j < NWidth; j++){
                dst[i * NWidth + j].x /= NWidth;
                dst[i * NWidth + j].y /= NWidth;
            }
        }
    }

    for (int i = 0; i < NWidth; i++){
        fft_y(dst + i, dst + i, NHeight, mode);
        if (mode == -1){
            for (int j = 0; j < NHeight; j++){
                dst[j * NWidth + i].x /= NHeight;
                dst[j * NWidth + i].y /= NHeight;
            }
        }
    }
    if (mode == 1) fft_shift(dst);
}

void fft_shift(struct _complex *src){
    struct _complex tmp;
    int a, b;
    for (int i = 0; i < NHeight / 2; i++){
        for (int j = 0; j < NWidth; j++){
            a = i * NWidth + j;
            b = ((i + NHeight / 2) % NHeight) * NWidth + (NWidth / 2 + j) % NWidth;

            tmp = src[a];
            src[a] = src[b];
            src[b] = tmp;

        }
    }
}

void pre_fft(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* buffer, struct _complex* F, struct _complex* newF){
    unsigned char* tmp = (unsigned char*)malloc(sizeof(unsigned char) * (NWidth * NHeight));
    for (int i = 0; i < (FileHeader->Size - 54) / 3; i++){
        tmp[i] = buffer[3 * i];
        F[i].x = (double)tmp[i];
        F[i].y = 0;
    }

    for (int i = 0; i < NHeight; i++){
        for (int j = 0; j < NWidth; j++){
            if (i < InfoHeader->Height && j < InfoHeader->Width){
                newF[i * NWidth + j].x = F[i * NWidth + j].x;                    
                newF[i * NWidth + j].y = 0;
            }
            else{
                newF[i * NWidth + j].x = 0;
                newF[i * NWidth + j].y = 0;
            }
        }
    }
}

void WriteF(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, struct _complex* buffer, const char* filename){
    unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char)* NHeight * NWidth * 3);
    memset(data, 0x00, NWidth * NHeight * 3);
    int NewWidth = (InfoHeader->Width * InfoHeader->Bit / 8 + 3) / 4 * 4;

    FILE *frequency = fopen(filename, "wb+");

    FileHeader->Size = NHeight * NewWidth - 54;
    InfoHeader->Height = NWidth;
    InfoHeader->Height = NHeight;
    InfoHeader->SIZE = NHeight * NewWidth;
    fwrite(&(FileHeader->Type), 2, 1, frequency);
    fwrite(&(FileHeader->Size), 4, 1, frequency);
    fwrite(&(FileHeader->Reservation), 4, 1, frequency);
    fwrite(&(FileHeader->Start), 4, 1, frequency);

    fwrite(InfoHeader, 40, 1, frequency);

    for (int i = 0; i < NHeight; i++){
        for(int j = 0, k = 0; j < NWidth && k < NWidth * 3; j++, k += 3){
            unsigned char pix = (unsigned char)sqrt(buffer[i * NWidth + j].x * buffer[i * NWidth + j].x + buffer[i * NWidth + j].y * buffer[i * NWidth + j].y);
            data[i * NewWidth + k] = pix;
            data[i * NewWidth + k + 1] = pix;
            data[i * NewWidth + k + 2] = pix;
        }
    }
    fwrite(data, FileHeader->Size - 54, 1, frequency);
    free(data);
    fclose(frequency);

}

void GetNewSize(BMPHEADER* Fileheader, BMPINFOHEADER* InfoHeader){
    int m = InfoHeader->Width;
    int n = 0;
    while(m > 1){
        n++;
        m = m >> 1;
    }
    NWidth = pow(2, n);

    m = InfoHeader->Height;
    n = 0;
    while(m > 1){
        n++;
        m = m >> 1;
    }
    NHeight = pow(2, n);
}

void HighPass(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, struct _complex* src){
    double d, d0 = 10;
    int mid_x = NWidth / 2;
    int mid_y = NHeight / 2;
    for (int i = 0; i < InfoHeader->Height; i++){
        for (int j = 0; j < InfoHeader->Width; j++){
            d = sqrt((j - mid_x) * (j - mid_x) + (i - mid_y) * (i - mid_y));
            if (d <= d0){
                src[i * InfoHeader->Width + j].x = 0;
                src[i * InfoHeader->Width + j].y = 0;
            }
        }
    } 
}

void LowPass(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, struct _complex* src){
    double d, d0 = 10;
    double h;
    int mid_x = NWidth / 2;
    int mid_y = NHeight / 2;
    for (int i = 0; i < InfoHeader->Height; i++){
        for (int j = 0; j < InfoHeader->Width; j++){
            d = sqrt((j - mid_x) * (j - mid_x) + (i - mid_y) * (i - mid_y));
            h = 1 / (1 + pow(d/d0, 2));
            src[i * InfoHeader->Width + j].x = src[i * InfoHeader->Width + j].x * h;
            src[i * InfoHeader->Width + j].y = src[i * InfoHeader->Width + j].y * h;
        }
    } 
}

void BandPass(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, struct _complex* src){
    double d, d0 = 10, w = 10;
    double h;
    int mid_x = NWidth / 2;
    int mid_y = NHeight / 2;
    for (int i = 0; i < InfoHeader->Height; i++){
        for (int j = 0; j < InfoHeader->Width; j++){
            d = (j - mid_x) * (j - mid_x) + (i - mid_y) * (i - mid_y);
            h = exp(-0.5 * pow(((d - d0 * d0) / sqrt(d) / w), 2));
            src[i * InfoHeader->Width + j].x = src[i * InfoHeader->Width + j].x * h;
            src[i * InfoHeader->Width + j].y = src[i * InfoHeader->Width + j].y * h;
        }
    } 
}

void pre_sobel(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* buffer, unsigned char* news){
    unsigned char* tmp = (unsigned char*)malloc(sizeof(unsigned char) * (NWidth * NHeight));
    for (int i = 0; i < (FileHeader->Size - 54) / 3; i++){
        tmp[i] = buffer[3 * i];
    }
    for (int i = 0; i < NHeight; i++){
        for (int j = 0; j < NWidth; j++){
            if (i < InfoHeader->Height && j < InfoHeader->Width){
                news[i * NWidth + j] = tmp[i * NWidth + j];                    
            }
            else{
                news[i * NWidth + j] = 0;
            }
        }
    }
}

void sobel(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* news, unsigned char* srcx, unsigned char* srcy){
    unsigned char* s = (unsigned char*)malloc(sizeof(unsigned char) * NHeight * NWidth);
    memset(s, 0, NHeight * NWidth);
    sobelx(FileHeader, InfoHeader, news, srcx);
    sobely(FileHeader, InfoHeader, news, srcy);
    for (int i = 1; i < NHeight - 1; i++){
        for (int j = 1; j < NWidth - 1; j++){
            s[i * NWidth + j] = sqrt(srcx[i * NWidth + j] * srcx[i * NWidth + j] + srcy[i * NWidth + j] * srcy[i * NWidth + j]);
        }
    }
    free(srcx);
    free(srcy);

    //writeBMP
    int NewWidth = (InfoHeader->Width * InfoHeader->Bit / 8 + 3) / 4 * 4;
    unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * NHeight * NewWidth);
    memset(buffer, 0, NHeight * NewWidth);
    for (int i = 0; i < NHeight; i++){
        for(int j = 0, k = 0; j < NWidth && k < NWidth * 3; j++, k += 3){
            buffer[i * NewWidth + k] = s[i * NWidth + j];
            buffer[i * NewWidth + k + 1] = s[i * NWidth + j];
            buffer[i * NewWidth + k + 2] = s[i * NWidth + j];
        }
    }
    WriteBMP(FileHeader, InfoHeader, buffer, "result/sobel.bmp");
}

void sobelx(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* src, unsigned char* srcx){
    for (int i = 1; i < NHeight - 1; i++){
        for (int j = 1; j < NWidth - 1; j++){
            srcx[i * NWidth + j] = abs(src[(i - 1) * NWidth + j + 1] + 2 * src[i * NWidth + j + 1] + src[(i + 1) * NWidth + j + 1] \
                                 - (src[(i - 1) * NWidth + j - 1] + 2 * src[i * NWidth + j - 1] + src[(i + 1) * NWidth + j - 1]));
        }
    }
}

void sobely(BMPHEADER* FileHeader, BMPINFOHEADER* InfoHeader, unsigned char* src, unsigned char* srcy){
    for (int i = 1; i < NHeight - 1; i++){
        for (int j = 1; j < NWidth - 1; j++){
            srcy[i * NWidth + j] = abs(src[(i - 1) * NWidth + j - 1] + 2 * src[(i - 1) * NWidth + j] + src[(i - 1) * NWidth + j + 1] \
                                 - (src[(i + 1) * NWidth + j - 1] + 2 * src[(i + 1) * NWidth + j] + src[(i + 1) * NWidth + j + 1]));
        }
    }
}
