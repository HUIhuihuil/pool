//
// Created by 69503 on 2020/9/30.
//

#ifndef CMEMORY_ALLOC_H
#define CMEMORY_ALLOC_H

#include <stdio.h>
#include <iostream>
void * operator new(size_t size);
void operator delete(void *p) ;
void * operator new[](size_t size);
void operator delete[](void* p);
void* memAlloc(size_t size);
void memFree(void* p);
#endif //CMEMORY_ALLOC_H
