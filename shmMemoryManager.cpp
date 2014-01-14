#include "StructDef.h"
#include "shmMemoryManager.h"
#include "log/taf_logger.h"

using namespace taf;

ShmMemoryManager::ShmMemoryManager()
{
    m_freeListHead = NULL;
    m_biLinkHead = NULL;
    m_biLinkTail = NULL;
    m_shmCheckHead = NULL;
    m_offsetArray = NULL;
}

LinkNode * ShmMemoryManager::GetFreeListHead()
{
    return m_freeListHead;
}

LinkNode * ShmMemoryManager::GetBiLinkHead()
{
    return m_biLinkHead;
}

LinkNode * ShmMemoryManager::GetBiLinkTail()
{
    return m_biLinkTail;
}

ShmCheckHead * ShmMemoryManager::GetShmCheckHead()
{
    return m_shmCheckHead;
}

int * ShmMemoryManager::GetOffsetArray()
{
    return m_offsetArray;
}

int ShmMemoryManager::GetShmId()
{
    return m_shmid;
}

char * ShmMemoryManager::GetShmPtr()
{
    return m_shmPtr;
}



int ShmMemoryManager::SetFreeListHead(LinkNode * m)
{
    m_freeListHead = m;
    return 0;
}

int ShmMemoryManager::SetBiLinkHead(LinkNode * m)
{
    m_biLinkHead = m;
    return 0;
}

int ShmMemoryManager::SetBiLinkTail(LinkNode * m)
{
    m_biLinkTail = m;
    return 0;
}

int ShmMemoryManager::SetShmCheckHead(ShmCheckHead * m)
{
    m_shmCheckHead = m;
    return 0;
}

int ShmMemoryManager::SetOffsetArray(int * m)
{
    m_offsetArray = m;
    return 0;
}

int ShmMemoryManager::SetShmId(int m)
{
    m_shmid = m;
    return 0;
}

int ShmMemoryManager::SetShmPtr(char * m)
{
    m_shmPtr = m;
    return 0;
}


int ShmMemoryManager::InitializeShareMemory()
{
    LOG->debug()<<"enter ShmMemoryManager::InitializeShareMemory()\n";
    m_shmid = shmget(key_t(0x235), 
            sizeof(ShmCheckHead)+(MAX_USER*sizeof(LinkNode))+(MAX_USER*sizeof(LinkNode *)), 
            0777|IPC_CREAT|IPC_EXCL);
    int flag = 0;
    if (m_shmid == -1 && errno == EEXIST)
    {
        m_shmid = shmget(key_t(0x235), 0, 0);
        flag = 1;
    }
    m_shmPtr = (char *)shmat(m_shmid, 0, 0);
    // flag = 0;
    
    ShmCheckHead * shmCheckHeadPtr = (ShmCheckHead *) m_shmPtr;
    if (flag == 1 && shmCheckHeadPtr->ifValidShmSpace == 1 && shmCheckHeadPtr != 0)
    {
        LOG->debug()<<"valid share memory has existed!"<<endl;
        m_shmCheckHead = shmCheckHeadPtr;
        shmCheckHeadPtr++;
        
        LinkNode *ptr = (LinkNode *)shmCheckHeadPtr;
        m_offsetArray = (int *) (ptr+MAX_USER);
        int offset = *m_offsetArray;
        m_biLinkHead = (LinkNode *)(m_shmPtr+offset);
        m_biLinkHead->prev = NULL;
        
        LinkNode * curNode = m_biLinkHead;
        LinkNode * befNode = m_biLinkHead;
        for (int i=1; i<m_shmCheckHead->nodeCount; ++i) //link the user bi-link list
        {
          int offset = *(m_offsetArray+i);
          curNode = (LinkNode *)(m_shmPtr+offset);
          befNode->next = curNode;
          curNode->prev = befNode;
          befNode = curNode;
          LOG->debug()<<"Nodeinfo,Id:"<<curNode->userinfo.userId<<",Score"<<curNode->userinfo.userScore<<", Rank:"<<curNode->userinfo.userRank<<endl;
        }
        curNode->next = NULL;
        m_biLinkTail = curNode;   //user bi-link list end!!!!! 
        
        LOG->debug()<<"m_biLinkTail info,id:"<<m_biLinkTail->userinfo.userId<<", score:"<<m_biLinkTail->userinfo.userScore<<", rank:"<<m_biLinkTail->userinfo.userRank<<endl;
        // reconstruct the map
        LinkNode * p = m_biLinkHead;
        LinkNode * bef = p;
        int nodeCount = 0;
        while (p != NULL)
        {
            //m_userid2LinkNode.insert(pair<int, LinkNode *>(p->userinfo.userId, p));
            bef = p;
            p = p->next;
            ++nodeCount;
        }
        //LOG->debug()<<"nodeCount:"<<nodeCount<<", m_shmCheckHead->nodeCount:"<<m_shmCheckHead->nodeCount<<endl;
        // deal with the free node list
        if (nodeCount == MAX_USER)
        {
            m_freeListHead = NULL;
            LOG->debug()<<"m_freeListHead is null"<<endl;
        }
        else       //need reconstruct the free node list
        {
            LOG->debug()<<"m_freeListHead is not null, list need reconstruct"<<endl;
            m_freeListHead = (m_biLinkHead + nodeCount);
            m_freeListHead->prev = NULL;
            LinkNode * backNode = m_freeListHead;
            LinkNode * frontNode = m_freeListHead;
            nodeCount++;
            while (nodeCount < MAX_USER)
            {
                frontNode = (m_biLinkHead+nodeCount);
                frontNode->prev = NULL;
                backNode->next = frontNode;
                backNode = frontNode;
                nodeCount++;
            }
            backNode->next = NULL;
        }
        LOG->debug()<<"Load share memory finished"<<endl;
        return 1;
    }
    else
    {
        LOG->debug()<<"no valid share memory existed, construct start"<<endl;
        shmCheckHeadPtr->ifValidShmSpace = 0;
        shmCheckHeadPtr->nodeCount = 0;
        m_shmCheckHead = shmCheckHeadPtr;
        shmCheckHeadPtr++;

        m_freeListHead = (LinkNode *)shmCheckHeadPtr;
        m_freeListHead->prev = NULL;
        m_freeListHead->next = NULL;

        LinkNode *currentPtr = m_freeListHead;
        LinkNode *beforeCurrentPtr = m_freeListHead;

        for (int i=1; i<MAX_USER; ++i)    // link the free list
        {
            currentPtr = m_freeListHead+i;
            (currentPtr->userinfo).userScore = i;
            beforeCurrentPtr->next = currentPtr;
            currentPtr->prev = NULL;
            beforeCurrentPtr = currentPtr;
        }
        (m_freeListHead+MAX_USER-1)->next = NULL;   //terminate the link list

        m_offsetArray = (int *)(m_freeListHead+MAX_USER);
        for(int i=0; i<MAX_USER; ++i)
        {
            *(m_offsetArray+i)= -1; 
        }
        
        m_shmCheckHead->ifValidShmSpace = 1;
        m_shmCheckHead->nodeCount = 0;
        
        LOG->debug()<<"construct share memory finished!!"<<endl;
        return 2;
    }
}
