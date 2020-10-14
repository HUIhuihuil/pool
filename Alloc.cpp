//
// Created by 69503 on 2020/9/30.
//
#include "Alloc.h"
#include "MemoryM.h"
void * operator new(size_t size){
    return MemoryMgr::Instance().allocMem(size);
}
void operator delete(void* p){
    MemoryMgr::Instance().freeMem(p);
}
void * operator new[](size_t size){
    return MemoryMgr::Instance().allocMem(size);
}
void operator delete[](void* p){
    MemoryMgr::Instance().freeMem(p);
}
void* memAlloc(size_t size){
    return malloc(size);
}
void memFree(void* p){
    free(p);
}