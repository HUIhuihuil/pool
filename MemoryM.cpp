//
// Created by 69503 on 2020/10/1.
//
#include "MemoryM.h"
MemoryMgr::MemoryMgr()
{
    init_szAlloc(0,64,&_mem64);
    init_szAlloc(65,128,&_mem128);
    init_szAlloc(129,256,&_mem256);
    init_szAlloc(257,512,&_mem512);
    init_szAlloc(513,1024,&_mem1024);

}
MemoryMgr::~MemoryMgr()
{

}
void* MemoryMgr::allocMem(size_t nSize){
    if(nSize<=MAX_MEMORY_SIZE)
    {
        return _szAlloc[nSize]->allocMem(nSize);
    } else
    {
        MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize+ sizeof(MemoryBlock));
        pReturn->bbool= false;
        pReturn->nID = -1;
        pReturn->nRef=1;
        pReturn->pAlloc= nullptr;
        pReturn->pNext= nullptr;
        std::printf("MemoryM.cpp,allocMem:%llx,id=%d,size=%d\n",
                pReturn,pReturn->nID,nSize);
        return (char*)pReturn+sizeof(MemoryBlock);
    }
}
void MemoryMgr::freeMem(void* pMem)
{
    MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem- sizeof(MemoryBlock));
    std::printf("freeMem:%llx,id=%d\n",
                pBlock,pBlock->nID);
    if(pBlock->bbool)
    {
        pBlock->pAlloc->freeMem(pMem);
    } else
    {
        if(--pBlock->nRef==0)
        {
            free(pBlock);
        }
    }
}
void MemoryMgr::init_szAlloc(int nBegin,int nEnd,MemoryAlloc* pMemA)
{
    for(int n=nBegin;n<=nEnd;n++)
    {
        _szAlloc[n] = pMemA;
    }
}








MemoryAlloc::MemoryAlloc(){
    _pBuf= nullptr;
    _pHead= nullptr;
    _nSize=0;
    _nBlockSize=0;
}
MemoryAlloc::~MemoryAlloc(){
    if(_pBuf)
    {
        free(_pBuf);
    }
}
void* MemoryAlloc::allocMem(size_t nSize){
    if(!_pBuf)
    {
        initMemory();
    }
    MemoryBlock* pReturn = nullptr;
    if(!_pHead)
    {
        pReturn = (MemoryBlock*)malloc(nSize+ sizeof(MemoryBlock));
        pReturn->bbool= false;
        pReturn->nID = -1;
        pReturn->nRef=0;
        pReturn->pAlloc=nullptr;
        pReturn->pNext= nullptr;
    }
    else
    {
        pReturn=_pHead;
        _pHead = _pHead->pNext;
        assert(0==pReturn->nRef);
        pReturn->nRef=1;
    }
    std::printf("In pool,MemoryM.cpp,allocMem:%llx,id=%d,size=%d\n",
                pReturn,pReturn->nID,nSize);
    return ((char*)pReturn+sizeof(MemoryBlock));
}
void MemoryAlloc::freeMem(void* pMem){
    char* pDat=(char*)pMem;
    MemoryBlock* pBlock=(MemoryBlock*)(pDat- sizeof(MemoryBlock));
    assert(1==pBlock->nRef);
    if(--pBlock->nRef!=0)
    {
        return;
    }
    if(pBlock->bbool)
    {
        pBlock->pNext=_pHead;
        _pHead=pBlock;
    } else
    {
        free(pBlock);
    }
}
void MemoryAlloc::initMemory(){
    //断言
    assert(nullptr==_pBuf);
    if(_pBuf)
    {
        return;
    }
    //向系统申请内存池
    size_t bufSize = (_nSize+ sizeof(MemoryBlock))*_nBlockSize;
    _pBuf = (char*)malloc(bufSize);
    //初始化内存池
    _pHead = (MemoryBlock*)_pBuf;
    _pHead->bbool= true;
    _pHead->nID = 0;
    _pHead->nRef=0;
    _pHead->pAlloc=this;
    _pHead->pNext= nullptr;
    MemoryBlock* _pTemp1 = _pHead;
    //遍历初始内存块
    for(size_t n=1;n<_nBlockSize;n++){
        MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf+(n*(_nSize+ sizeof(MemoryBlock))));

        pTemp2->bbool= true;
        pTemp2->nID = n;
        pTemp2->nRef=0;
        pTemp2->pAlloc=this;
        _pTemp1->pNext= nullptr;
        _pTemp1->pNext= pTemp2;
        _pTemp1 = pTemp2;
    }
}
