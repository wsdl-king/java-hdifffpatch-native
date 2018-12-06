//
//  diff_demo.cpp
//  HDiff
//
/*
 This is the HDiffPatch copyright.

 Copyright (c) 2012-2013 HouSisong All Rights Reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "assert.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <jni.h>
#include "HDiff/diff.h"
#include "hdiff_Hdiff.h"

typedef unsigned char   TByte;
typedef size_t          TUInt;
typedef ptrdiff_t       TInt;

using namespace std;

void readFile(std::vector<TByte>& data,const char* fileName){
    std::ifstream file(fileName,std::ios::in | std::ios::binary | std::ios::ate);
    assert(file);
    file.seekg(0,std::ios::end);
     std::streampos file_length=file.tellg();
    printf("fileopen = %d \n",file.is_open());
    file.seekg(0,std::ios::beg);
    size_t needRead=(size_t)file_length;
    if ((file_length<0)||((std::streamsize)needRead!=(std::streamsize)file_length)) {
        file.close();
        exit(1);
    }
    data.resize(needRead);
    file.read((char*)&data[0], needRead);
    std::streamsize readed=file.gcount();
    file.close();
    if ((std::streamsize)needRead!=readed)  exit(1);
}

void writeFile(const std::vector<TByte>& data,const char* fileName){
    std::ofstream file(fileName);
    file.write((const char*)&data[0], data.size());
    file.close();
}


int genpatch(int argc, char * argv[]){
    clock_t time0=clock();
    if (argc!=4) {
        std::cout<<"diff command line parameter:\n oldFileName newFileName outDiffFileName\n";
        return 0;
    }
    const char* oldFileName=argv[1];
    const char* newFileName=argv[2];
    const char* outDiffFileName=argv[3];
    std::cout<<"old:\"" <<oldFileName<< "\"\nnew:\""<<newFileName<<"\"\nout:\""<<outDiffFileName<<"\"\n";

    std::vector<TByte> oldData; readFile(oldData,oldFileName);
    std::vector<TByte> newData; readFile(newData,newFileName);
    const TUInt oldDataSize=oldData.size();
    const TUInt newDataSize=newData.size();
    std::vector<TByte> diffData;
    diffData.push_back((TByte)newDataSize);
    diffData.push_back((TByte)(newDataSize>>8));
    diffData.push_back((TByte)(newDataSize>>16));
    TUInt kNewDataSize=-1;
    if ((newDataSize>>31)==0){
        kNewDataSize=4;
        diffData.push_back((TByte)(newDataSize>>24));
    }else{
        kNewDataSize=9;
        diffData.push_back(0xFF);
        diffData.push_back((TByte)(newDataSize>>24));

        const TUInt highSize=((newDataSize>>16)>>16);
        diffData.push_back((TByte)highSize);
        diffData.push_back((TByte)(highSize>>8));
        diffData.push_back((TByte)(highSize>>16));
        diffData.push_back((TByte)(highSize>>24));
    }

    TByte* newData_begin=0; if (!newData.empty()) newData_begin=&newData[0];
    const TByte* oldData_begin=0; if (!oldData.empty()) oldData_begin=&oldData[0];
    clock_t time1=clock();
     std::cout<<"filedata1:"<<std::endl<<newData_begin<<std::endl;

    create_diff(newData_begin,newData_begin+newDataSize,oldData_begin, oldData_begin+oldDataSize, diffData);
    clock_t time2=clock();
    if (!check_diff(newData_begin,newData_begin+newDataSize,oldData_begin,oldData_begin+oldDataSize, &diffData[0]+kNewDataSize, &diffData[0]+diffData.size())){
        std::cout<<"  patch check diff data error!!!\n";
        exit(2);
    }else{
        std::cout<<"  patch check diff data ok!\n";
    }
    writeFile(diffData,outDiffFileName);
    clock_t time3=clock();
    std::cout<<"  out diff file ok!\n";
    std::cout<<"oldDataSize : "<<oldDataSize<<"\nnewDataSize : "<<newDataSize<<"\ndiffDataSize: "<<diffData.size()<<"\n";
    std::cout<<"\ndiff    time:"<<(time2-time1)*(1000.0/CLOCKS_PER_SEC)<<" ms\n";
    std::cout<<"all run time:"<<(time3-time0)*(1000.0/CLOCKS_PER_SEC)<<" ms\n";

    return 0;
}

JNIEXPORT jint JNICALL Java_hdiff_Hdiff_hdiff(JNIEnv *env, jobject thiz, jstring oldFilePath, jstring newFilePath,
                             jstring diffFilePath){
    int argc = 4;
    char * argv[argc];
    argv[0] = "hdiff";
    argv[1] = (char*) (env->GetStringUTFChars( oldFilePath, 0));
    argv[2] = (char*) (env->GetStringUTFChars( newFilePath, 0));
    argv[3] = (char*) (env->GetStringUTFChars( diffFilePath, 0));

    printf("old apk = %s \n", argv[1]);
    printf("new apk = %s \n", argv[2]);
    printf("patch = %s \n", argv[3]);

    int ret = genpatch(argc, argv);

    printf("genDiff result = %d ", ret);

    env->ReleaseStringUTFChars( oldFilePath, argv[1]);
    env->ReleaseStringUTFChars( newFilePath, argv[2]);
    env->ReleaseStringUTFChars( diffFilePath, argv[3]);
	//env->DeleteLocalRef(argv[1]);
	//env->DeleteLocalRef(argv[2]);
	//env->DeleteLocalRef(argv[3]);
    return ret;
}

int main()
{
    ifstream in("v1");
    assert(in);

    in.seekg(0,ios::end);       //基地址为文件结束处，偏移地址为0，于是指针定位在文件结束处
    streampos sp=in.tellg(); //sp为定位指针，因为它在文件结束处，所以也就是文件的大小
    cout<<"file size:"<<endl<<sp<<endl;

    in.seekg(-sp/3,ios::end); //基地址为文件末，偏移地址为负，于是向前移动sp/3个字节
    streampos sp2=in.tellg();
    cout<<"from file to point:"<<endl<<sp2<<endl;

    in.seekg(0,ios::beg);        //基地址为文件头，偏移量为0，于是定位在文件头
    cout<<in.rdbuf();             //从头读出文件内容
    in.seekg(sp2);

    cout<<in.rdbuf()<<endl; //从sp2开始读出文件内容

    return 0;
}