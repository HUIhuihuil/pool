#### 内存池管理代码流程解析

##### 为什么要有内存池？

在C++并发通信引擎架构中，每秒存在百万次的内存申请和内存释放，所以会产生大量的内存碎片，因此为了使程序长期稳定高效的运行，这些都需要内存池的管理和优化。

##### 内存池申请流程过程详解

1. 我们若需要设计自己的内存池，那么我们就要舍弃C++为我们提供的new、new[]、delete、delete[] 内置方法，调用更底层的 malloc 方法来实现我们的内存池的编写。

   ```c++
   void * operator new(size_t size);
   void operator delete(void *p) ;
   void * operator new[](size_t size);
   void operator delete[](void* p);
   ```

2. 我们现在举一个new的例子，来展示整个调用new的流程。当我们调用new时会进入到我们重载的new中。

   ```c++
   void * operator new(size_t size){
       return MemoryMgr::Instance().allocMem(size);
   }
   ```

3. 此时我们进入了到了MemoryMgr::Instance().allocMem(size)。我们先看MemoryMgr的头文件。

   ```c++
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
   ```

   这里属于内存池的管理类，管理者只能有一个所以采用了单例模式，我们将构造函数私有化，并将创造对象的函数静态化，这样不管调用多少次，我们都是使用的同一个管理者对象。

4. 调用Instance()后我们进入了MemoryMgr管理者类的构造函数。

   ```c++
   MemoryMgr::MemoryMgr()
   {
       init_szAlloc(0,64,&_mem64);
       init_szAlloc(65,128,&_mem128);
       init_szAlloc(129,256,&_mem256);
       init_szAlloc(257,512,&_mem512);
       init_szAlloc(513,1024,&_mem1024);
   }
   ```

   此时构造函数调用了一个类的私有成员函数。

   ```c++
   void MemoryMgr::init_szAlloc(int nBegin,int nEnd,MemoryAlloc* pMemA)
   {
       for(int n=nBegin;n<=nEnd;n++)
       {
           _szAlloc[n] = pMemA;
       }
   }
   ```

   那么这里的含义是：我让我管理者的私有成员的MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE+1]做了一个内存的映射。这个指针数组里分别指向了私有内存池成员MemoryAlloctor<64,10> _mem64到MemoryAlloctor<1024,10> _mem1024 。这样当我们申请内存池时，管理者对象可以很快的找到我们所对应大小的内存池。

5. 现在我们正式进入管理者对象的allocMem方法。

   ```c++
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
   ```

   分为两个部分：1、当我的申请的内存大小超出我的最大内存池时（我们这里的最大内存池为1024，可以根据业务需求来设置）我们则调用系统的malloc方法分配内存，这部分内存我们不做管理。2、当申请的内存小于1024时，我们则调用我们刚才创建好的映射，来进入到MemoryAlloc类来对对象池管（MemoryAlloctor类继承了MemoryAlloc类，只是MemoryAlloc类的种模板实例化的方式）。

6. 以下是我们的MemoryAlloc类的成员函数。

   ```c++
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
   ```

   这里的含义是，如果我们_pBuf为空，也就是我们还没有创建池，那么我们就需要初始化一个内存池出来，初始化内存池的方法为initMemory()。下图为初始化的一整块的内存池图：

   ![](https://github.com/HUIhuihuil/pool/blob/master/%E5%86%85%E5%AD%98%E6%B1%A0%E7%94%B3%E8%AF%B7%E9%87%8A%E6%94%BE%E6%B5%81%E7%A8%8B%E5%9B%BE.jpg)

   接下来就是我们根据phead来决定怎么使用内存池，如果phead不为空，说明我们的池还没有用完，下图展示申请和释放的过程：

   ![](./内存池申请释放流程图.jpg)

   若是phead为空，我们则使用系统自带的malloc直接申请，不经过内存池的管理。

以上，就是我们这个基础内存池的申请内存的基本流程。我们可以根据需要来设计出符合自己的内存池大小。让我们的程序更加稳定。
