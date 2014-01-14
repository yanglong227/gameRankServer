#ifndef R_H
#define SHM_MEMORY_MANAGER_H

#include "StructDef.h"

class ShmMemoryManager
{
public:

    ShmMemoryManager();
    int InitializeShareMemory();
    
    LinkNode * GetFreeListHead();
    LinkNode * GetBiLinkHead();
    LinkNode * GetBiLinkTail();
    ShmCheckHead * GetShmCheckHead();
    int * GetOffsetArray();
    int GetShmId();
    char * GetShmPtr();

    int SetFreeListHead(LinkNode *);
    int SetBiLinkHead(LinkNode *);
    int SetBiLinkTail(LinkNode *);
    int SetShmCheckHead(ShmCheckHead *);
    int SetOffsetArray(int *);
    int SetShmId(int);
    int SetShmPtr(char *);

private:
    LinkNode * m_freeListHead;
    LinkNode * m_biLinkHead;
    LinkNode * m_biLinkTail;
    ShmCheckHead * m_shmCheckHead;
    int * m_offsetArray;
    int m_shmid;
    char * m_shmPtr;

};
#endif
