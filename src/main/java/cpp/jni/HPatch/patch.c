//
//  patch.cpp
//  HPatch
//
/*
 This is the HPatch copyright.

 Copyright (c) 2012-2014 HouSisong All Rights Reserved.

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
#include "patch.h"
#include "string.h" //memcpy memset size_t
#include "assert.h" //assert

typedef unsigned char TByte;
#define TUInt size_t
#define _hpatch_FALSE hpatch_FALSE
//int __debug_check_false_x=0; //for debug
//#define _hpatch_FALSE (1/__debug_check_false_x)

//__RUN_MEM_SAFE_CHECK用来启动内存访问越界检查.
#define __RUN_MEM_SAFE_CHECK

static hpatch_BOOL _bytesRle_load(TByte* out_data,TByte* out_dataEnd,const TByte* rle_code,const TByte* rle_code_end);
static void addData(TByte* dst,const TByte* src,TUInt length);
static hpatch_StreamPos_t unpackUInt64WithTag(const TByte** src_code,const TByte* src_code_end,const int kTagBit);
#define unpackUIntWithTag(src_code,src_code_end,kTagBit) (TUInt)unpackUInt64WithTag(src_code,src_code_end,kTagBit)
#define unpackUInt(src_code,src_code_end) unpackUIntWithTag(src_code,src_code_end,0)

hpatch_BOOL patch(TByte* out_newData,TByte* out_newData_end,
            const TByte* oldData,const TByte* oldData_end,
            const TByte* serializedDiff,const TByte* serializedDiff_end){
    const TByte *code_lengths, *code_lengths_end,
                *code_inc_newPos, *code_inc_newPos_end,
                *code_inc_oldPos, *code_inc_oldPos_end,
                *code_newDataDiff, *code_newDataDiff_end;
    TUInt       ctrlCount;

    assert(out_newData<=out_newData_end);
    assert(oldData<=oldData_end);
    assert(serializedDiff<=serializedDiff_end);
    ctrlCount=unpackUInt(&serializedDiff, serializedDiff_end);
    {   //head
        TUInt lengthSize=unpackUInt(&serializedDiff, serializedDiff_end);
        TUInt inc_newPosSize=unpackUInt(&serializedDiff, serializedDiff_end);
        TUInt inc_oldPosSize=unpackUInt(&serializedDiff, serializedDiff_end);
        TUInt newDataDiffSize=unpackUInt(&serializedDiff, serializedDiff_end);
#ifdef __RUN_MEM_SAFE_CHECK
        if (lengthSize>(TUInt)(serializedDiff_end-serializedDiff)) return _hpatch_FALSE;
#endif
        code_lengths=serializedDiff;     serializedDiff+=lengthSize;
        code_lengths_end=serializedDiff;
#ifdef __RUN_MEM_SAFE_CHECK
        if (inc_newPosSize>(TUInt)(serializedDiff_end-serializedDiff)) return _hpatch_FALSE;
#endif
        code_inc_newPos=serializedDiff; serializedDiff+=inc_newPosSize;
        code_inc_newPos_end=serializedDiff;
#ifdef __RUN_MEM_SAFE_CHECK
        if (inc_oldPosSize>(TUInt)(serializedDiff_end-serializedDiff)) return _hpatch_FALSE;
#endif
        code_inc_oldPos=serializedDiff; serializedDiff+=inc_oldPosSize;
        code_inc_oldPos_end=serializedDiff;
#ifdef __RUN_MEM_SAFE_CHECK
        if (newDataDiffSize>(TUInt)(serializedDiff_end-serializedDiff)) return _hpatch_FALSE;
#endif
        code_newDataDiff=serializedDiff; serializedDiff+=newDataDiffSize;
        code_newDataDiff_end=serializedDiff;
    }

    //decode rle ; rle data begin==serializedDiff;
    if (!_bytesRle_load(out_newData, out_newData_end, serializedDiff, serializedDiff_end))
        return _hpatch_FALSE;

    {   //patch
        const TUInt newDataSize=(TUInt)(out_newData_end-out_newData);
        TUInt oldPosBack=0;
        TUInt newPosBack=0;
        TUInt i;
        for (i=0; i<ctrlCount; ++i){
            TUInt oldPos,inc_oldPos,inc_oldPos_sign;

            TUInt copyLength=unpackUInt(&code_inc_newPos, code_inc_newPos_end);
            TUInt addLength=unpackUInt(&code_lengths, code_lengths_end);
#ifdef __RUN_MEM_SAFE_CHECK
            if (code_inc_oldPos>=code_inc_oldPos_end) return _hpatch_FALSE;
#endif
            inc_oldPos_sign=(*code_inc_oldPos)>>(8-1);
            inc_oldPos=unpackUIntWithTag(&code_inc_oldPos, code_inc_oldPos_end, 1);
            if (inc_oldPos_sign==0)
                oldPos=oldPosBack+inc_oldPos;
            else
                oldPos=oldPosBack-inc_oldPos;
            if (copyLength>0){
#ifdef __RUN_MEM_SAFE_CHECK
                if (copyLength>(TUInt)(newDataSize-newPosBack)) return _hpatch_FALSE;
                if (copyLength>(TUInt)(code_newDataDiff_end-code_newDataDiff)) return _hpatch_FALSE;
#endif
                memcpy(out_newData+newPosBack,code_newDataDiff,copyLength);
                code_newDataDiff+=copyLength;
                newPosBack+=copyLength;
            }
#ifdef __RUN_MEM_SAFE_CHECK
            if ((addLength>(TUInt)(newDataSize-newPosBack))) return _hpatch_FALSE;
            if ((oldPos>(TUInt)(oldData_end-oldData))||(addLength>(TUInt)(oldData_end-oldData-oldPos))) return _hpatch_FALSE;
#endif
            addData(out_newData+newPosBack,oldData+oldPos,addLength);
            oldPosBack=oldPos;
            newPosBack+=addLength;
        }

        if (newPosBack<newDataSize){
            TUInt copyLength=newDataSize-newPosBack;
#ifdef __RUN_MEM_SAFE_CHECK
            if (copyLength>(TUInt)(code_newDataDiff_end-code_newDataDiff)) return _hpatch_FALSE;
#endif
            memcpy(out_newData+newPosBack,code_newDataDiff,copyLength);
            code_newDataDiff+=copyLength;
            newPosBack=newDataSize;
        }
    }

    if (  (code_lengths==code_lengths_end)
        &&(code_inc_newPos==code_inc_newPos_end)
        &&(code_inc_oldPos==code_inc_oldPos_end)
        &&(code_newDataDiff==code_newDataDiff_end))
        return !hpatch_FALSE;
    else
        return _hpatch_FALSE;
}

//变长正整数编码方案(x bit额外类型标志位,x<=7),从高位开始输出1--n byte:
// x0*  7-x bit
// x1* 0*  7+7-x bit
// x1* 1* 0*  7+7+7-x bit
// x1* 1* 1* 0*  7+7+7+7-x bit
// x1* 1* 1* 1* 0*  7+7+7+7+7-x bit
// ......
static hpatch_StreamPos_t unpackUInt64WithTag(const TByte** src_code,const TByte* src_code_end,const int kTagBit){//读出整数并前进指针.
#ifdef __RUN_MEM_SAFE_CHECK
    const int kPackMaxTagBit=7;
#endif
    hpatch_StreamPos_t  value;
    TByte               code;
    const TByte*        pcode=*src_code;

#ifdef __RUN_MEM_SAFE_CHECK
    assert((0<=kTagBit)&&(kTagBit<=kPackMaxTagBit));
    if (src_code_end-pcode<=0) return 0;
#endif
    code=*pcode; ++pcode;
    value=code&((1<<(7-kTagBit))-1);
    if ((code&(1<<(7-kTagBit)))!=0){
        do {
#ifdef __RUN_MEM_SAFE_CHECK
            assert((value>>(sizeof(value)*8-7))==0);
            if (src_code_end==pcode) break;
#endif
            code=*pcode; ++pcode;
            value=(value<<7) | (code&((1<<7)-1));
        } while ((code&(1<<7))!=0);
    }
    (*src_code)=pcode;
    return value;
}

static void addData(TByte* dst,const TByte* src,TUInt length){
    TUInt length_fast,i;

    length_fast=length&(~(TUInt)7);
    for (i=0;i<length_fast;i+=8){
        dst[i  ]+=src[i  ];
        dst[i+1]+=src[i+1];
        dst[i+2]+=src[i+2];
        dst[i+3]+=src[i+3];
        dst[i+4]+=src[i+4];
        dst[i+5]+=src[i+5];
        dst[i+6]+=src[i+6];
        dst[i+7]+=src[i+7];
    }
    for (;i<length;++i)
        dst[i]+=src[i];
}


//数据用rle压缩后的包类型2bit
typedef enum TByteRleType{
    kByteRleType_rle0  = 0,    //00表示后面存的压缩0;    (包中不需要字节数据)
    kByteRleType_rle255= 1,    //01表示后面存的压缩255;  (包中不需要字节数据)
    kByteRleType_rle   = 2,    //10表示后面存的压缩数据;  (包中字节数据只需储存一个字节数据)
    kByteRleType_unrle = 3     //11表示后面存的未压缩数据;(包中连续储存多个字节数据)
} TByteRleType;

static const int kByteRleType_bit=2;


static hpatch_BOOL _bytesRle_load(TByte* out_data,TByte* out_dataEnd,const TByte* rle_code,const TByte* rle_code_end){
    const TByte*    ctrlBuf,*ctrlBuf_end;

    TUInt ctrlSize= unpackUInt(&rle_code,rle_code_end);
#ifdef __RUN_MEM_SAFE_CHECK
    if (ctrlSize>(TUInt)(rle_code_end-rle_code)) return _hpatch_FALSE;
#endif
    ctrlBuf=rle_code;
    rle_code+=ctrlSize;
    ctrlBuf_end=rle_code;
    while (ctrlBuf_end-ctrlBuf>0){
        enum TByteRleType type=(enum TByteRleType)((*ctrlBuf)>>(8-kByteRleType_bit));
        TUInt length= 1 + unpackUIntWithTag(&ctrlBuf,ctrlBuf_end,kByteRleType_bit);
#ifdef __RUN_MEM_SAFE_CHECK
        if (length>(TUInt)(out_dataEnd-out_data)) return _hpatch_FALSE;
#endif
        switch (type){
            case kByteRleType_rle0:{
                memset(out_data,0,length);
                out_data+=length;
            }break;
            case kByteRleType_rle255:{
                memset(out_data,255,length);
                out_data+=length;
            }break;
            case kByteRleType_rle:{
#ifdef __RUN_MEM_SAFE_CHECK
                if (1>(TUInt)(rle_code_end-rle_code)) return _hpatch_FALSE;
#endif
                memset(out_data,*rle_code,length);
                ++rle_code;
                out_data+=length;
            }break;
            case kByteRleType_unrle:{
#ifdef __RUN_MEM_SAFE_CHECK
                if (length>(TUInt)(rle_code_end-rle_code)) return _hpatch_FALSE;
#endif
                memcpy(out_data,rle_code,length);
                rle_code+=length;
                out_data+=length;
            }break;
        }
    }
    
    if (  (ctrlBuf==ctrlBuf_end)
        &&(rle_code==rle_code_end)
        &&(out_data==out_dataEnd))
        return !hpatch_FALSE;
    else
        return _hpatch_FALSE;
}

//----------------------
//patch by stream
#undef  TUInt
#define TUInt hpatch_StreamPos_t
#define kStreamCacheSize  (1024)


typedef struct TStreamClip{
    TUInt       streamPos;
    TUInt       streamPos_end;
    const struct hpatch_TStreamInput*  srcStream;
    size_t      cacheBegin;//cacheEnd==kStreamCacheSize
    TByte       cacheBuf[kStreamCacheSize];
} TStreamClip;

static void _TStreamClip_init(struct TStreamClip* sclip, const struct hpatch_TStreamInput* srcStream,
                            TUInt streamPos,TUInt streamPos_end){
    sclip->srcStream=srcStream;
    sclip->streamPos=streamPos;
    sclip->streamPos_end=streamPos_end;
    sclip->cacheBegin=kStreamCacheSize;
}
#define _TStreamClip_isFinish(sclip) (((sclip)->cacheBegin==kStreamCacheSize) && ((sclip)->streamPos==(sclip)->streamPos_end))
#define _TStreamClip_getStreamDataSize(sclip) ((TUInt)((TUInt)((sclip)->streamPos_end-(sclip)->streamPos)+(size_t)(kStreamCacheSize-(sclip)->cacheBegin)))
#define _TStreamClip_getCachedSize(sclip) ((size_t)(kStreamCacheSize-(sclip)->cacheBegin))
#define _TStreamClip_getCacheIsEmpty(sclip) ((sclip)->cacheBegin==kStreamCacheSize)

static void _TStreamClip_updateCache(struct TStreamClip* sclip){
    TByte* buf0=&sclip->cacheBuf[0];
    const TUInt stremSize=(TUInt)(sclip->streamPos_end-sclip->streamPos);
    size_t readSize=sclip->cacheBegin;
    if (readSize>stremSize)
        readSize=(size_t)stremSize;
    if (readSize==0) return;
    if (!_TStreamClip_getCacheIsEmpty(sclip)){
        memmove(buf0+(size_t)(sclip->cacheBegin-readSize),buf0+sclip->cacheBegin,_TStreamClip_getCachedSize(sclip));
    }
    sclip->srcStream->read(sclip->srcStream->streamHandle,sclip->streamPos,
                           buf0+(size_t)(kStreamCacheSize-readSize),buf0+kStreamCacheSize);
    sclip->cacheBegin-=readSize;
    sclip->streamPos+=readSize;
}

static TByte _TStreamClip_getByte(struct TStreamClip* sclip){
    //assert(1<=kStreamCacheSize);
    if (_TStreamClip_getCacheIsEmpty(sclip))
        _TStreamClip_updateCache(sclip);
    assert(!_TStreamClip_getCacheIsEmpty(sclip));
    return sclip->cacheBuf[sclip->cacheBegin];
}
static TByte* _TStreamClip_getData(struct TStreamClip* sclip,size_t readSize){
    assert(readSize<=kStreamCacheSize);
    if (readSize>_TStreamClip_getCachedSize(sclip))
        _TStreamClip_updateCache(sclip);
    assert(readSize<=_TStreamClip_getCachedSize(sclip));
    return &sclip->cacheBuf[sclip->cacheBegin];
}
#define _TStreamClip_getDataSkip(sclip,skipDataLength) { assert((skipDataLength)<=_TStreamClip_getCachedSize(sclip)); (sclip)->cacheBegin+=(skipDataLength); }

static void _TStreamClip_resetPosEnd(struct TStreamClip* sclip,TUInt new_posEnd){
    sclip->streamPos_end=new_posEnd;
    if (sclip->streamPos>new_posEnd){
        TByte* buf0=&sclip->cacheBuf[0];
        size_t popSize=(size_t)(sclip->streamPos-new_posEnd);
        assert((TUInt)(sclip->streamPos-new_posEnd)<=_TStreamClip_getCachedSize(sclip));
        memmove(buf0+sclip->cacheBegin+popSize,buf0+sclip->cacheBegin,(size_t)(_TStreamClip_getCachedSize(sclip)-popSize));
        sclip->cacheBegin+=popSize;
        sclip->streamPos-=popSize;
    }
}

static TUInt _TStreamClip_unpackUIntWithTag(struct TStreamClip* sclip,const int kTagBit){
#define  kMaxPackedByte ((sizeof(TUInt)*8+6)/7+1)
    TByte* code,*code0;
    TUInt result;
    size_t readSize=kMaxPackedByte;
    const TUInt dataSize=_TStreamClip_getStreamDataSize(sclip);
    if (readSize>dataSize)
        readSize=(size_t)dataSize;
    assert(readSize>0);
    code0=_TStreamClip_getData(sclip,readSize);
    code=code0;
    result=unpackUInt64WithTag((const TByte**)&code,code0+readSize,kTagBit);
    _TStreamClip_getDataSkip(sclip,(size_t)(code-code0));
    return result;
}
#define _TStreamClip_unpackUInt(sclip) _TStreamClip_unpackUIntWithTag(sclip,0)

typedef struct _TBytesRle_load_stream{
    TUInt                               memCopyLength;
    TUInt                               memSetLength;
    const struct hpatch_TStreamInput*   rle_stream;
    TByte                               memSetValue;
    struct TStreamClip                  ctrlClip;
    struct TStreamClip                  rleCodeClip;
} _TBytesRle_load_stream;

static hpatch_BOOL _TBytesRle_load_stream_init(_TBytesRle_load_stream* loader,const struct hpatch_TStreamInput* rle_stream,
                                               TUInt rle_code,TUInt rle_code_end){
    TUInt ctrlSize;
    _TStreamClip_init(&loader->ctrlClip,rle_stream,rle_code,rle_code_end);
    ctrlSize=_TStreamClip_unpackUInt(&loader->ctrlClip);
    rle_code=(TUInt)(rle_code_end-_TStreamClip_getStreamDataSize(&loader->ctrlClip));
#ifdef __RUN_MEM_SAFE_CHECK
    if (ctrlSize>(TUInt)(rle_code_end-rle_code)) return _hpatch_FALSE;
#endif
    loader->rle_stream=rle_stream;
    //_TStreamClip_init(&loader->ctrlClip,rle_stream,rle_code,rle_code+ctrlSize);
    _TStreamClip_resetPosEnd(&loader->ctrlClip, rle_code+ctrlSize);
    _TStreamClip_init(&loader->rleCodeClip,rle_stream,rle_code+ctrlSize,rle_code_end);
    loader->memSetLength=0;
    loader->memSetValue=0;//nil;
    loader->memCopyLength=0;
    return !hpatch_FALSE;
}

static void memSet_add(TByte* dst,const TByte src,size_t length){
    size_t length_fast,i;

    length_fast=length&(~(size_t)7);
    for (i=0;i<length_fast;i+=8){
        dst[i  ]+=src;
        dst[i+1]+=src;
        dst[i+2]+=src;
        dst[i+3]+=src;
        dst[i+4]+=src;
        dst[i+5]+=src;
        dst[i+6]+=src;
        dst[i+7]+=src;
    }
    for (;i<length;++i)
        dst[i]+=src;
}

static void _TBytesRle_load_stream_mem_add(_TBytesRle_load_stream* loader,size_t* _decodeSize,TByte** _out_data){
    size_t  decodeSize=*_decodeSize;
    TByte* out_data=*_out_data;
    struct TStreamClip* rleCodeClip=&loader->rleCodeClip;

    while ((loader->memSetLength>0)&&(decodeSize>0)) {
        size_t memSetStep=decodeSize;
        if (memSetStep>loader->memSetLength) memSetStep=(size_t)loader->memSetLength;
        memSet_add(out_data,loader->memSetValue,memSetStep);
        out_data+=memSetStep;
        decodeSize-=memSetStep;
        loader->memSetLength-=memSetStep;
    }
    while ((loader->memCopyLength>0)&&(decodeSize>0)) {
        TByte* rleData;
        size_t decodeStep=kStreamCacheSize;
        if (decodeStep>decodeSize)
            decodeStep=decodeSize;
        if (decodeStep>loader->memCopyLength)
            decodeStep=(size_t)loader->memCopyLength;
        if (decodeStep>_TStreamClip_getCachedSize(rleCodeClip))
            _TStreamClip_updateCache(rleCodeClip);
        rleData=_TStreamClip_getData(rleCodeClip,decodeStep);
        addData(out_data,rleData,decodeStep);
        _TStreamClip_getDataSkip(rleCodeClip, decodeStep);
        out_data+=decodeStep;
        decodeSize-=decodeStep;
        loader->memCopyLength-=decodeStep;
    }
    *_decodeSize=decodeSize;
    *_out_data=out_data;
}

static hpatch_BOOL _TBytesRle_load_stream_isFinish(const _TBytesRle_load_stream* loader){
    return(loader->memSetLength==0)
        &&(loader->memCopyLength==0)
        &&(_TStreamClip_isFinish(&loader->rleCodeClip))
        &&(_TStreamClip_isFinish(&loader->ctrlClip));
}

static hpatch_BOOL _TBytesRle_load_stream_decode_add(_TBytesRle_load_stream* loader,size_t decodeSize,TByte* out_data){
    _TBytesRle_load_stream_mem_add(loader,&decodeSize,&out_data);

    while ((decodeSize>0)&&(!_TStreamClip_isFinish(&loader->ctrlClip))){
        enum TByteRleType type=(enum TByteRleType)(_TStreamClip_getByte(&loader->ctrlClip)>>(8-kByteRleType_bit));
        TUInt length= 1 + _TStreamClip_unpackUIntWithTag(&loader->ctrlClip,kByteRleType_bit);
        switch (type){
            case kByteRleType_rle0:{
                loader->memSetLength=length;
                loader->memSetValue=0;
            }break;
            case kByteRleType_rle255:{
                loader->memSetLength=length;
                loader->memSetValue=255;
            }break;
            case kByteRleType_rle:{
#ifdef __RUN_MEM_SAFE_CHECK
                if (1>_TStreamClip_getStreamDataSize(&loader->rleCodeClip)) return _hpatch_FALSE;
#endif
                loader->memSetLength=length;
                loader->memSetValue=_TStreamClip_getByte(&loader->rleCodeClip);
                _TStreamClip_getDataSkip(&loader->rleCodeClip, 1);
            }break;
            case kByteRleType_unrle:{
#ifdef __RUN_MEM_SAFE_CHECK
                if (length>_TStreamClip_getStreamDataSize(&loader->rleCodeClip)) return _hpatch_FALSE;
#endif
                loader->memCopyLength=length;
            }break;
        }
        _TBytesRle_load_stream_mem_add(loader,&decodeSize,&out_data);
    }

    if (decodeSize==0)
        return !hpatch_FALSE;
    else
        return _hpatch_FALSE;
}

static  hpatch_BOOL _patch_decode_from_clipOrStream(const struct hpatch_TStreamOutput* out_newData,TUInt writeToPos,
                                                    _TBytesRle_load_stream* rle_loader,TUInt decodeLength,
                                                    struct TStreamClip* srcClip,
                                                    const struct hpatch_TStreamInput* srcStream,TUInt readPos){
    TByte  _tempMemBuf[kStreamCacheSize];
    TByte* data=&_tempMemBuf[0];
    while (decodeLength>0){
        size_t decodeStep=kStreamCacheSize;
        if (decodeStep>decodeLength)
            decodeStep=(size_t)decodeLength;
        if (srcClip!=0) {
            assert(srcStream==0);
            if (decodeStep>_TStreamClip_getCachedSize(srcClip))
                _TStreamClip_updateCache(srcClip);
            memcpy(data, _TStreamClip_getData(srcClip,decodeStep),decodeStep);
            _TStreamClip_getDataSkip(srcClip,(size_t)decodeStep);
        }else{
            assert(srcStream!=0);
            srcStream->read(srcStream->streamHandle,readPos,data,data+decodeStep);
        }
        if (!_TBytesRle_load_stream_decode_add(rle_loader,decodeStep,data)) return _hpatch_FALSE;
        out_newData->write(out_newData->streamHandle,writeToPos,data,data+decodeStep);
        readPos+=decodeStep;
        writeToPos+=decodeStep;
        decodeLength-=decodeStep;
    }
    return !hpatch_FALSE;
}

hpatch_BOOL patch_stream(const struct hpatch_TStreamOutput* out_newData,
                         const struct hpatch_TStreamInput*  oldData,
                         const struct hpatch_TStreamInput*  serializedDiff){
    struct TStreamClip              code_lengthsClip;
    struct TStreamClip              code_inc_oldPosClip;
    struct TStreamClip              code_inc_newPosClip;
    struct TStreamClip              code_newDataDiffClip;
    struct _TBytesRle_load_stream   rle_loader;
    TUInt                           ctrlCount;

    assert(out_newData!=0);
    assert(out_newData->write!=0);
    assert(oldData!=0);
    assert(oldData->read!=0);
    assert(serializedDiff!=0);
    assert(serializedDiff->read!=0);
    assert(serializedDiff->streamSize>0);

    {   //head
        TUInt lengthSize,inc_newPosSize,inc_oldPosSize,newDataDiffSize;
        const TUInt diffPos_end=serializedDiff->streamSize;
        TUInt diffPos0;
        struct TStreamClip*  diffClip=&code_lengthsClip;
        _TStreamClip_init(diffClip,serializedDiff,0,diffPos_end);
        ctrlCount=_TStreamClip_unpackUInt(diffClip);
        lengthSize=_TStreamClip_unpackUInt(diffClip);
        inc_newPosSize=_TStreamClip_unpackUInt(diffClip);
        inc_oldPosSize=_TStreamClip_unpackUInt(diffClip);
        newDataDiffSize=_TStreamClip_unpackUInt(diffClip);
        diffPos0=(TUInt)(diffPos_end-_TStreamClip_getStreamDataSize(diffClip));
#ifdef __RUN_MEM_SAFE_CHECK
        if (lengthSize>(TUInt)(serializedDiff->streamSize-diffPos0)) return _hpatch_FALSE;
#endif
        //_TStreamClip_init(&code_lengthsClip,serializedDiff,diffPos0,diffPos0+lengthSize);
        _TStreamClip_resetPosEnd(&code_lengthsClip,diffPos0+lengthSize);
        diffPos0+=lengthSize;
#ifdef __RUN_MEM_SAFE_CHECK
        if (inc_newPosSize>(TUInt)(serializedDiff->streamSize-diffPos0)) return _hpatch_FALSE;
#endif
        _TStreamClip_init(&code_inc_newPosClip,serializedDiff,diffPos0,diffPos0+inc_newPosSize);
        diffPos0+=inc_newPosSize;
#ifdef __RUN_MEM_SAFE_CHECK
        if (inc_oldPosSize>(TUInt)(serializedDiff->streamSize-diffPos0)) return _hpatch_FALSE;
#endif
        _TStreamClip_init(&code_inc_oldPosClip,serializedDiff,diffPos0,diffPos0+inc_oldPosSize);
        diffPos0+=inc_oldPosSize;
#ifdef __RUN_MEM_SAFE_CHECK
        if (newDataDiffSize>(TUInt)(serializedDiff->streamSize-diffPos0)) return _hpatch_FALSE;
#endif
        _TStreamClip_init(&code_newDataDiffClip,serializedDiff,diffPos0,diffPos0+newDataDiffSize);
        diffPos0+=newDataDiffSize;

        if (!_TBytesRle_load_stream_init(&rle_loader,serializedDiff,diffPos0,diffPos_end)) return _hpatch_FALSE;
    }

    {   //patch
        const TUInt newDataSize=out_newData->streamSize;
        TUInt oldPosBack=0;
        TUInt newPosBack=0;
        TUInt i;
        for (i=0; i<ctrlCount; ++i){
            TUInt oldPos,inc_oldPos;
            TByte inc_oldPos_sign;

            TUInt copyLength=_TStreamClip_unpackUInt(&code_inc_newPosClip);
            TUInt addLength=_TStreamClip_unpackUInt(&code_lengthsClip);
#ifdef __RUN_MEM_SAFE_CHECK
            if (_TStreamClip_isFinish(&code_inc_oldPosClip)) return _hpatch_FALSE;
#endif
            inc_oldPos_sign=_TStreamClip_getByte(&code_inc_oldPosClip)>>(8-1);
            inc_oldPos=_TStreamClip_unpackUIntWithTag(&code_inc_oldPosClip,1);
            if (inc_oldPos_sign==0)
                oldPos=oldPosBack+inc_oldPos;
            else
                oldPos=oldPosBack-inc_oldPos;
            if (copyLength>0){
#ifdef __RUN_MEM_SAFE_CHECK
                if (copyLength>(TUInt)(newDataSize-newPosBack)) return _hpatch_FALSE;
                if (copyLength>_TStreamClip_getStreamDataSize(&code_newDataDiffClip)) return _hpatch_FALSE;
#endif
                if (!_patch_decode_from_clipOrStream(out_newData,newPosBack,&rle_loader,copyLength,
                                                     &code_newDataDiffClip,0,0)) return _hpatch_FALSE;
                newPosBack+=copyLength;
            }
#ifdef __RUN_MEM_SAFE_CHECK
            if ((addLength>(TUInt)(newDataSize-newPosBack))) return _hpatch_FALSE;
            if ((oldPos>(oldData->streamSize))||(addLength>(TUInt)(oldData->streamSize-oldPos))) return _hpatch_FALSE;
#endif
            if (!_patch_decode_from_clipOrStream(out_newData,newPosBack,&rle_loader,addLength,
                                                 0,oldData,oldPos)) return _hpatch_FALSE;
            oldPosBack=oldPos;
            newPosBack+=addLength;
        }

        if (newPosBack<newDataSize){
            TUInt copyLength=newDataSize-newPosBack;
#ifdef __RUN_MEM_SAFE_CHECK
            if (copyLength>_TStreamClip_getStreamDataSize(&code_newDataDiffClip)) return _hpatch_FALSE;
#endif
            if (!_patch_decode_from_clipOrStream(out_newData,newPosBack,&rle_loader,copyLength,
                                      &code_newDataDiffClip,0,0)) return _hpatch_FALSE;
            newPosBack=newDataSize;
        }
    }

    if (   _TBytesRle_load_stream_isFinish(&rle_loader)
        && _TStreamClip_isFinish(&code_lengthsClip)
        && _TStreamClip_isFinish(&code_inc_newPosClip)
        && _TStreamClip_isFinish(&code_inc_oldPosClip)
        && _TStreamClip_isFinish(&code_newDataDiffClip) )
        return !hpatch_FALSE;
    else
        return _hpatch_FALSE;
}



