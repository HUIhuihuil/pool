//
// Created by 69503 on 2020/10/1.
//

#ifndef CMEMORY_MEMORYM_H
#define CMEMORY_MEMORYM_H
//内存管理工具
#include <stdio.h>
#include "Alloc.h"
#include <assert.h>

#define MAX_MEMORY_SIZE 1024
//内存管理工具
class MemoryAlloc;

//内存块，最小单元
class MemoryBlock
{
public:
    //内存块编号
    int nID;
    //引用次数
    int nRef;
    //所属最大内存块（池）
    MemoryAlloc* pAlloc;
    //下一块的位置
    MemoryBlock* pNext;
    //是否在内存池中
    bool bbool;

private:
    //预留
    char c1;
    char c2;
    char c3;

    MemoryBlock();
    ~MemoryBlock();
};
class MemoryAlloc
{
public:
    MemoryAlloc();
    ~MemoryAlloc();
    //申请内
    void* allocMem(size_t nSize);
    //释放内存
    void freeMem(void* p);
    void initMemory();
protected:
    //内存池的地址
    char* _pBuf;
    //头部单元
    MemoryBlock* _pHead;
    //内存单元的大小
    size_t _nSize;
    //内存单元的数量
    size_t _nBlockSize;

};

template <size_t nSize,size_t nBlockSize>
class MemoryAlloctor:public MemoryAlloc{
public:
    MemoryAlloctor()
    {
        const size_t  n= sizeof(void*);
        _nSize = (nSize/n)*n+(nSize%n?n:0);
        _nBlockSize = nBlockSize;
    }

};
class MemoryMgr{
public:

    //申请内
    void* allocMem(size_t nSize);
    //释放内存
    void freeMem(void* p);

    static MemoryMgr& Instance(){
        //单例模式
        static MemoryMgr mgr;
        return mgr;
    }

private:
    MemoryMgr();
    ~MemoryMgr();
    //初始化内存池映射数组
    void init_szAlloc(int nBegin,int nEnd,MemoryAlloc* pMemA);
    MemoryAlloctor<64,10> _mem64;
    MemoryAlloctor<128,10> _mem128;
    MemoryAlloctor<256,10> _mem256;
    MemoryAlloctor<512,10> _mem512;
    MemoryAlloctor<1024,10> _mem1024;
    MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE+1];
};
#endif //CMEMORY_MEMORYM_H
