#include <string>
#include <stdio.h>
#include <vector>
#include <direct.h>
#include <iostream>
#include <io.h>
#include <thread>
#include <atlimage.h>
#include <assert.h>
#include <string>

using namespace std;

const short soble1[3][3] = { { -1, -2, -1 },
                             { 0, 0, 0 },
                             { 1, 2, 1 } };

const short soble2[3][3] = { { -1, 0, 1 },
                             { -2, 0, 2 },
                             { -1, 0, 1 } };



bool is_in_array(int x1, int y1, int x2, int y2)
{
    if(y1<0||x1<0)
        return false;
    if (x1 >= x2 || y1 >= y2)
        return false;
    return true;
}


//遍历每个文件夹下的图片，并保存为fileList
void getImages(string path, vector<string>& imagesList)
{
    intptr_t hFile = 0;
    struct _finddata_t fileinfo;
    string p;

    hFile = _findfirst(p.assign(path).append("\\*.png").c_str(), &fileinfo);

    if (hFile != -1) {
        do {
            imagesList.push_back(fileinfo.name);//保存类名
        } while (_findnext(hFile, &fileinfo) == 0);
    }
}

vector<string> getImgList(string path)
{
    vector<string> fileLists;
    getImages(path, fileLists);

    return fileLists;

    for (int i = 0; i < fileLists.size(); i++) {
        for (vector<string>::const_iterator p = fileLists.begin(); p != fileLists.end(); p++)
            cout << path+"//"+ *p << endl;
    }
}

void FilterImage(CImage* imgSrc, CImage* imgDst)
{
	if (imgSrc->IsNull())
		return;
	int smoothGauss[9] = { 1,2,1,2,4,2,1,2,1 }; // 高斯模板
	int opTemp[9];
	float aver; // 系数
	aver = (float)(1.0 / 12.0);
	memcpy(opTemp, smoothGauss, 9 * sizeof(int));
	int i, j;
	int nWidth = imgSrc->GetWidth();
	int nHeight = imgSrc->GetHeight();
    byte* pDataSrc = (byte*)imgSrc->GetBits(); //获取指向图像数据的指针
    byte* pDataDst = (byte*)imgDst->GetBits();
    int pitchSrc = imgSrc->GetPitch(); //获取每行图像占用的字节数 +：top-down；-：bottom-up DIB
    int pitchDst = imgDst->GetPitch();
    int bitCountSrc = imgSrc->GetBPP() / 8;  // 获取每个像素占用的字节数
    int bitCountDst = imgDst->GetBPP() / 8;

	for (i = 1; i < nWidth - 1; i++) {
		for (j = 1; j < nHeight - 1; j++) {
			BYTE rr = 0, gg = 0, bb = 0;
            int sum = 0;
			int index = 0;
			for (int col = -1; col <= 1; col++) {
				for (int row = -1; row <= 1; row++) {
                    sum += *(pDataSrc + pitchSrc * (j+col) + (i+row) * bitCountSrc) * opTemp[index];
                    index++;
					
				}
			}
			
			// 处理溢出点
            sum *= aver;
            sum = sum > 255 ? 255 : sum < 0 ? -sum : sum;
            *(pDataDst + pitchDst * j + i * bitCountDst) = sum;
		}
	}
}


bool ImageToGray(CImage* imgSrc, CImage* imgDst)
{
    int maxY = imgSrc->GetHeight();
    int maxX = imgSrc->GetWidth();

    if (!imgDst->IsNull())
    {
        imgDst->Destroy();
    }
    imgDst->Create(maxX, maxY, 8, 0);//图像大小与imgSrc相同，每个像素占1字节

    if (imgDst->IsNull())
        return FALSE;

    //为imgDst构造256阶灰度调色表
    RGBQUAD ColorTab[256];
    for (int i = 0; i<256; i++)
    {
        ColorTab[i].rgbBlue = ColorTab[i].rgbGreen = ColorTab[i].rgbRed = i;
    }
    imgDst->SetColorTable(0, 256, ColorTab);

    byte* pDataSrc = (byte*)imgSrc->GetBits(); //获取指向图像数据的指针
    byte* pDataDst = (byte*)imgDst->GetBits();
    int pitchSrc = imgSrc->GetPitch(); //获取每行图像占用的字节数 +：top-down；-：bottom-up DIB
    int pitchDst = imgDst->GetPitch();
    int bitCountSrc = imgSrc->GetBPP() / 8;  // 获取每个像素占用的字节数
    int bitCountDst = imgDst->GetBPP() / 8;
    /*if ((bitCountSrc != 3) || (bitCountDst != 1))
        return FALSE;*/
    int tmpR, tmpG, tmpB, avg;
    for (int i = 0; i<maxX; i++)
    {
        for (int j = 0; j<maxY; j++)
        {
            tmpR = *(pDataSrc + pitchSrc*j + i*bitCountSrc);
            tmpG = *(pDataSrc + pitchSrc*j + i*bitCountSrc + 1);
            tmpB = *(pDataSrc + pitchSrc*j + i*bitCountSrc + 2);
            avg = (int)(tmpR + tmpG + tmpB) / 3;
            *(pDataDst + pitchDst*j + i*bitCountDst) = avg;
        }
    }
    return TRUE;
}


int sobelEdgeDetect(CImage* imgSrc, CImage* imgDst)
{
    int maxY = imgSrc->GetHeight();
    int maxX = imgSrc->GetWidth();


    if (imgDst->IsNull())
        return FALSE;


    int height = imgSrc->GetHeight();
    int width = imgSrc->GetWidth();
    byte* pDataSrc = (byte*)imgSrc->GetBits(); //获取指向图像数据的指针
    byte* pDataDst = (byte*)imgDst->GetBits();
    int pitchSrc = imgSrc->GetPitch(); //获取每行图像占用的字节数 +：top-down；-：bottom-up DIB
    int pitchDst = imgDst->GetPitch();
    int bitCountSrc = imgSrc->GetBPP() / 8;  // 获取每个像素占用的字节数
    int bitCountDst = imgDst->GetBPP() / 8;

    short value[9];


    for (int i = 0; i < maxX; i++) {
        for (int j = 0; j < maxY; j++) {
            /* sobel */
            value[0] = is_in_array(i - 1, j - 1, width, height) ? imgSrc->GetPixel(i - 1, j - 1) : 0;
            value[1] = is_in_array(i - 1, j, width, height) ? imgSrc->GetPixel(i - 1, j) : 0;
            value[2] = is_in_array(i - 1, j + 1, width, height) ? imgSrc->GetPixel(i - 1, j + 1) : 0;
            value[3] = is_in_array(i, j - 1, width, height) ? imgSrc->GetPixel(i, j - 1) : 0;
            value[4] = imgSrc->GetPixel(i, j);
            value[5] = is_in_array(i, j + 1, width, height) ? imgSrc->GetPixel(i, j + 1) : 0;
            value[6] = is_in_array(i + 1, j - 1, width, height) ? imgSrc->GetPixel(i + 1, j - 1) : 0;
            value[7] = is_in_array(i + 1, j, width, height) ? imgSrc->GetPixel(i + 1, j) : 0;
            value[8] = is_in_array(i + 1, j + 1, width, height) ? imgSrc->GetPixel(i + 1, j + 1) : 0;

            short a =abs( value[0] * soble1[0][0] + value[1] * soble1[0][1] + value[2] * soble1[0][2] +
                value[3] * soble1[1][0] + value[4] * soble1[1][1] + value[5] * soble1[1][2] +
                value[6] * soble1[2][0] + value[7] * soble1[2][1] + value[8] * soble1[2][2]);
            a = a > 255 ? 255 : 0;

            short b = abs(value[0] * soble2[0][0] + value[1] * soble2[0][1] + value[2] * soble2[0][2] +
                value[3] * soble2[1][0] + value[4] * soble2[1][1] + value[5] * soble2[1][2] +
                value[6] * soble2[2][0] + value[7] * soble2[2][1] + value[8] * soble2[2][2]);
            b = b > 255 ? 255 : 0;
            short temp = a + b > 255 ? 255 : 0;

            *(pDataDst + pitchDst * j + i * bitCountDst) = temp;

        }
    }

    return TRUE;
}


bool InitalImage(CImage* img, int width, int height)
{
    if (img->IsNull())
        img->Create(width, height, 8);
    else
    {
        if (width <= 0 || height <= 0)
            return false;
        else if (img->GetHeight() == width && img->GetWidth() == height)
            return true;
        else
        {
            img->Destroy();
            img->Create(width, height, 8);
        }
    }
    //写入调色板
    RGBQUAD ColorTable[256];
    img->GetColorTable(0, 256, ColorTable);
    for (int i = 0; i < 256; i++)
    {
        ColorTable[i].rgbBlue = (BYTE)i;
        ColorTable[i].rgbGreen = (BYTE)i;
        ColorTable[i].rgbRed = (BYTE)i;
    }
    img->SetColorTable(0, 256, ColorTable);
    return true;
}


CImage* SplitImage(CImage* imgSrc, int splitCount)
{
    int width = imgSrc->GetWidth();
    int height = imgSrc->GetHeight();
    int blockHeight = height / splitCount;
    CImage* roiImg = new CImage[splitCount];
    byte* pDataSrc = (byte*)imgSrc->GetBits(); //获取指向图像数据的指针
    
    int pitchSrc = imgSrc->GetPitch(); //获取每行图像占用的字节数 +：top-down；-：bottom-up DIB
    
    int bitCountSrc = imgSrc->GetBPP() / 8;  // 获取每个像素占用的字节数
    
    for (int i = 0; i < splitCount; i++)
    {
        InitalImage(&roiImg[i], width, blockHeight);
        byte* pDataDst = (byte*)roiImg[i].GetBits();
        int pitchDst = roiImg[i].GetPitch();
        int bitCountDst = roiImg[i].GetBPP() / 8;
        for (int x = 0; x < width; x++)
            for (int y = 0; y < blockHeight; y++)
            {
                *(pDataDst + pitchDst * y + x * bitCountDst) = *(pDataSrc + pitchSrc * (y + i * blockHeight) + x * bitCountSrc);
            }
                //roiImg[i].SetPixel(x,y,imgSrc->GetPixel(x,y+i*blockHeight));
    }
    
    return roiImg;

}

void join(CImage* imgSrc,CImage* imgDst,int len)
{

    int width = imgSrc[0].GetWidth(), height = imgSrc[0].GetHeight();
    int bpp = imgSrc->GetBPP();
    
    byte* pDataDst = (byte*)imgDst->GetBits();
    int pitchDst = imgDst->GetPitch();
    int bitCountDst = imgDst->GetBPP() / 8;
    
    for (int i = 0; i < len; i++)
    {
        byte* pDataSrc = (byte*)imgSrc[i].GetBits(); //获取指向图像数据的指针

        int pitchSrc = imgSrc[i].GetPitch(); //获取每行图像占用的字节数 +：top-down；-：bottom-up DIB

        int bitCountSrc = imgSrc[i].GetBPP() / 8;  // 获取每个像素占用的字节数
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height ; y++)
            {    
                //imgDst->SetPixel(x, y + i * height, imgSrc[i].GetPixel(x, y));
                *(pDataDst + pitchDst * (y+i*height) + x * bitCountDst) = *(pDataSrc + pitchSrc * y + x * bitCountSrc);
            }
        }
        
    }


}


void ThreadTest(CImage* imgSrc, CImage* imgDst)
{
    int threadCount = 2;

    thread* th = new thread[threadCount];
    thread* th1 = new thread[threadCount];
    ImageToGray(imgSrc, imgDst);
    FilterImage(imgDst,imgDst);
    CImage* newImage = SplitImage(imgDst, threadCount);


    for (int i = 0; i < threadCount; i++)
    {
        th[i] = thread(sobelEdgeDetect, &newImage[i], &newImage[i]);

    }
    for (int i = 0; i < threadCount; i++)
    {
        th[i].join();
    }

    cout << "OK";

    join(newImage, imgDst, threadCount);

}


wstring string2wstring(string str)
{
    wstring result;
    //获取缓冲区大小，并申请空间，缓冲区大小按字符计算  
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    TCHAR* buffer = new TCHAR[len + 1];
    //多字节编码转换成宽字节编码  
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
    buffer[len] = '\0';             //添加字符串结尾  
    //删除缓冲区并返回值  
    result.append(buffer);
    delete[] buffer;
    return result;
}


CImage Bit32To24(CImage* imgSrc)
{
    int width = imgSrc->GetWidth(), height = imgSrc->GetHeight();
    int bpp = imgSrc->GetBPP();

    byte* pDataSrc = (byte*)imgSrc->GetBits(); //获取指向图像数据的指针

    int pitchSrc = imgSrc->GetPitch(); //获取每行图像占用的字节数 +：top-down；-：bottom-up DIB

    int bitCountSrc = imgSrc->GetBPP() / 8;  // 获取每个像素占用的字节数
    CImage imgDst;
    imgDst.Create(width, height, 24);
    byte* pDataDst = (byte*)imgDst.GetBits();
    int pitchDst = imgDst.GetPitch();
    int bitCountDst = imgDst.GetBPP() / 8;
    int step = imgSrc->GetPitch();

    int BytesPerLine24 = 3 * imgSrc->GetWidth();
    int BytesPerLine32 = 4 * imgSrc->GetWidth();
    while (BytesPerLine24 % 4 != 0)
        BytesPerLine24++;
    while (BytesPerLine32 % 4 != 0)
        BytesPerLine32++;

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            *pDataDst++ = *pDataSrc++;
            *pDataDst++ = *pDataSrc++;
            *pDataDst++ = *pDataSrc++;
            pDataSrc++;
        }
        pDataDst += BytesPerLine24-imgDst.GetWidth()*3;
        pDataSrc += BytesPerLine32 - imgSrc->GetWidth() * 4;
    }
    return imgDst;
}

int main()
{
    int n;

    CImage imgSrc, imgDst;
    string srcPath = "G://Pic//pixiv";
    string dstPath = "G://Pic//Test";
    

slct:    cout << endl << "按1使用默认输入/输出目录，按2自行输入目录" << endl;
    cin >> n;
    if (n == 2)
    {
        cout << "输入目录：";
        cin >> srcPath;
        cout << endl << "输出目录：";
        cin >> dstPath;
    }
    else if (n == 1) {}
    else goto slct;

    vector<string> Images = getImgList(srcPath);
    int num = Images.size();
    CImage* imgSet = new CImage[num];
    int idx = 0;
    for (vector<string>::const_iterator p = Images.begin(); p != Images.end(); p++)
    {
        string pth = srcPath + "//" + *p;
        wstring tmp = string2wstring(pth);
        LPCTSTR tPath = tmp.c_str();
        imgSet[idx].Load(tPath);
        if(imgSet[idx].IsNull())
        {
            continue;
        }
        if (imgSet[idx].GetBPP() == 32)
        {
            for (int i = 0; i < imgSet[idx].GetWidth(); i++)
            {
                for (int j = 0; j < imgSet[idx].GetHeight(); j++)
                {
                    UCHAR* cr = (UCHAR*)imgSet[idx].GetPixelAddress(i, j);
                    cr[0] = cr[0] * cr[3] / 255;
                    cr[1] = cr[1] * cr[3] / 255;
                    cr[2] = cr[2] * cr[3] / 255;
                }
            }
        }
        idx++;
    }
    
    for (int i = 0; i < idx; i++)
    {
        CImage dstImage;
        string pth = dstPath + "//" + to_string(i)+".png";
        wstring tmp = string2wstring(pth);
        LPCTSTR tPath = tmp.c_str();
        ThreadTest(&imgSet[i], &dstImage);
        dstImage.Save(tPath);
    }
    
	
	return 0;

}

