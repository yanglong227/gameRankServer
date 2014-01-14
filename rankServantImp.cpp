#include "rankServantImp.h"
#include "servant/Application.h"
#include "log/taf_logger.h"
#include "util/tc_monitor.h"

using namespace std;
using namespace taf;

TC_ThreadLock mutex;


void rankServantImp::initialize()  { }

void rankServantImp::destroy()
{
	LOG->debug()<<"leave rankServantImp::destory()"<<endl;
}

int rankServantImp::OutputEntireLink()
{
    LinkNode * temp = g_app.shmManager.GetBiLinkHead();
    while (temp != NULL)
    {
        LOG->debug()<<"userRank:"<<temp->userinfo.userRank<<", userScore:"<<temp->userinfo.userScore<<", userId:"<<temp->userinfo.userId<<endl;
        temp = temp->next;
    }
    return 0;
}

int rankServantImp::OutputEntireMap()
{
    map<int, LinkNode *>::iterator it = g_app.m_userid2LinkNode.begin();
    while (it != g_app.m_userid2LinkNode.end())
    {
        LOG->debug()<<"it->first:"<<it->first<<" LinkNode *'s userId:"<<(it->second)->userinfo.userId<<endl;
        it++;
    }
    return 0;
}

int rankServantImp::OutputEntirePtrarray()
{
    int i = 0;
    while (*(g_app.shmManager.GetOffsetArray()+i) != -1 && i < (g_app.shmManager.GetShmCheckHead())->nodeCount)
    {
        int offset = *(g_app.shmManager.GetOffsetArray()+i);
        LinkNode *p = (LinkNode *)(g_app.shmManager.GetShmPtr()+offset);
        LOG->debug()<<"userRank:"<<p->userinfo.userRank<<" userScore:"<<p->userinfo.userScore<<" userId:"<<p->userinfo.userId<<endl;
        ++i;
    }
    return 0;
}

int rankServantImp::UpdateLinkRank()
{
    TC_ThreadLock::Lock lock(mutex);                  // add lock
    (g_app.shmManager.GetBiLinkHead())->userinfo.userRank = 1;
    LinkNode * p = (g_app.shmManager.GetBiLinkHead())->next;
    while (p != NULL)
    {
        (p->userinfo).userRank = ((p->prev)->userinfo).userRank+1;
        if (p->next == NULL)
        {
            g_app.shmManager.SetBiLinkTail(p);
        }
        p = p->next;
    }
    return 0;
}

int rankServantImp::UpdatePtrArray()
{
    TC_ThreadLock::Lock lock(mutex);               //add lock
    LinkNode * p = g_app.shmManager.GetBiLinkHead();
    int i = 0;
    while (p != NULL)
    {
        *(g_app.shmManager.GetOffsetArray()+i) = ((char*)p)-g_app.shmManager.GetShmPtr();
        p = p->next;
        ++i;
    }
    return 0;
}

int rankServantImp::InsertFirstNode(const xxx::userInfo & userinfo)
{
    TC_ThreadLock::Lock lock(mutex);               //add lock
    LinkNode * p = g_app.shmManager.GetFreeListHead();
    g_app.shmManager.SetFreeListHead(p->next);
    
    p->next = NULL;
    p->prev = NULL;
    (p->userinfo).userId = userinfo.userId;
    (p->userinfo).userScore = userinfo.userScore;
    (p->userinfo).userRank = 1;
    g_app.shmManager.SetBiLinkHead(p);
    g_app.shmManager.SetBiLinkTail(p);
    UpdatePtrArray();
    
    g_app.m_userid2LinkNode.insert(pair<int, LinkNode*>(userinfo.userId, p));
    (g_app.shmManager.GetShmCheckHead())->nodeCount++;        //node of link list increase one
    //OutputEntireLink();
    return 0;
}

pair<int, LinkNode *> rankServantImp::IfExistedUser(int userid)
{
   map<int, LinkNode *>::iterator it = g_app.m_userid2LinkNode.find(userid); 
   pair<int, LinkNode *> result(-1, NULL);
   if (it == g_app.m_userid2LinkNode.end())
   {
       return  result;
   }
   result.first = it->first;
   result.second = it->second;
   return result;
}

LinkNode * rankServantImp::FindInsertPosition(LinkNode * startPosition, int score) // Find Insert Position
{
    LinkNode * p = startPosition;
    assert(p != NULL);
    while (p != NULL && (p->userinfo).userScore < score)
    {
        p = p->prev;
    }
    return p;
}

int rankServantImp::InsertLinkNode(LinkNode * insertPosition, LinkNode * toBeInsertNode)  //Insert
{
    TC_ThreadLock::Lock lock(mutex);               //add lock
    if (insertPosition == NULL) //the insert position is before the list head
    {
        toBeInsertNode->next = g_app.shmManager.GetBiLinkHead();
        g_app.shmManager.GetBiLinkHead()->prev = toBeInsertNode;
        toBeInsertNode->prev = NULL;
        g_app.shmManager.SetBiLinkHead(toBeInsertNode);
    }
    else
    {
        toBeInsertNode->next = insertPosition->next;
        if (insertPosition->next != NULL)
        {
            insertPosition->next->prev = toBeInsertNode;
        }
        toBeInsertNode->prev = insertPosition;
        insertPosition->next = toBeInsertNode;
    }
    return 0;
}

int rankServantImp::UploadUserInfo(const xxx::userInfo &userinfo, taf::JceCurrentPtr current)
{
    if (g_app.shmManager.GetBiLinkHead() == NULL)   
    {
        InsertFirstNode(userinfo);
        return 0;
    }
    else
    {
        pair<int, LinkNode *> ifExist = IfExistedUser(userinfo.userId);
        if (((g_app.shmManager.GetBiLinkTail())->userinfo).userRank >= MAX_USER)  //Bi-list has already existed MAX_USER nodes
        {
            if (((g_app.shmManager.GetBiLinkTail())->userinfo).userScore >= userinfo.userScore)
            {
                return 0;
            }
            else                         //need insert into the Bi-list
            {
                if (ifExist.first == -1) //current user does not existed in the bi-link list
                {
                    (g_app.m_userid2LinkNode).erase(((g_app.shmManager.GetBiLinkTail())->userinfo).userId);//delete old pair  
                    LinkNode * insertPosition = FindInsertPosition(g_app.shmManager.GetBiLinkTail(), userinfo.userScore);
                    LinkNode * toBeInsertNode = g_app.shmManager.GetBiLinkTail();
                    g_app.shmManager.SetBiLinkTail(g_app.shmManager.GetBiLinkTail()->prev);
                    
                    g_app.shmManager.GetBiLinkTail()->next = NULL;
                    toBeInsertNode->prev = NULL;
                    toBeInsertNode->next = NULL;
                    toBeInsertNode->userinfo.userId = userinfo.userId;
                    toBeInsertNode->userinfo.userScore = userinfo.userScore;

                    InsertLinkNode(insertPosition, toBeInsertNode);
                    (g_app.m_userid2LinkNode).insert(pair<int, LinkNode *>(userinfo.userId, toBeInsertNode));
                }
                else    //current user has already existed in the bi-link list
                {
                    LinkNode * currentPosition = ifExist.second;
                    if (currentPosition->userinfo.userScore >= userinfo.userScore)//old score bigger than current
                    {
                        return 0;
                    }
                    else
                    {
                        LinkNode * insertPosition = FindInsertPosition(g_app.shmManager.GetBiLinkTail(), userinfo.userScore);
                        LinkNode * toBeInsertNode = currentPosition;
                        if (insertPosition != NULL)
                        {
                            LOG->debug()<<(insertPosition->userinfo).userId<<"insertPosition's Id"<<endl;
                        }
                        if (currentPosition == g_app.shmManager.GetBiLinkTail())
                        {
                            LOG->debug()<<"currentPosition is the tail"<<endl;
                            g_app.shmManager.SetBiLinkTail(g_app.shmManager.GetBiLinkTail()->prev);
                            g_app.shmManager.GetBiLinkTail()->next = NULL;
                            toBeInsertNode->prev = NULL;
                            toBeInsertNode->next = NULL;
                            toBeInsertNode->userinfo.userScore = userinfo.userScore;
                            InsertLinkNode(insertPosition, toBeInsertNode);
                        }
                        else if (currentPosition == g_app.shmManager.GetBiLinkHead())
                        {
                            toBeInsertNode->userinfo.userScore = userinfo.userScore;
                        }
                        else
                        {
                            toBeInsertNode->prev->next = toBeInsertNode->next;
                            toBeInsertNode->next->prev = toBeInsertNode->prev;
                            toBeInsertNode->next = NULL;
                            toBeInsertNode->prev = NULL;
                            toBeInsertNode->userinfo.userScore = userinfo.userScore;
                            InsertLinkNode(insertPosition, toBeInsertNode);
                        }
                    } 
                }
                UpdateLinkRank();
                UpdatePtrArray();
            }
        }
        else               //Bi-list has no more than MAX_USER nodes
        {
            if (ifExist.first == -1)   //current user does not existed in bi-link list
            {
                LinkNode * toBeInsertNode = g_app.shmManager.GetFreeListHead();
                g_app.shmManager.SetFreeListHead(toBeInsertNode->next);
                toBeInsertNode->next = NULL;
                toBeInsertNode->userinfo.userScore = userinfo.userScore;
                toBeInsertNode->userinfo.userId = userinfo.userId;

                LinkNode * insertPosition = FindInsertPosition(g_app.shmManager.GetBiLinkTail(), userinfo.userScore);
                InsertLinkNode(insertPosition, toBeInsertNode);
                g_app.shmManager.GetShmCheckHead()->nodeCount++; //increase the node count 
                g_app.m_userid2LinkNode.insert(pair<int, LinkNode *>(userinfo.userId, toBeInsertNode));
                
            }
            else    //current user already existed in bi-link list 
            {
                LinkNode * currentPosition = ifExist.second;
                if (currentPosition->userinfo.userScore >= userinfo.userScore)
                {
                    LOG->debug()<<"old Score is bigger than current"<<endl;
                    return 0;
                }
                else
                {
                    if (g_app.shmManager.GetShmCheckHead()->nodeCount == 1)
                    {
                        currentPosition->userinfo.userScore = userinfo.userScore;
                    }
                    else
                    {
                        LinkNode * insertPosition = FindInsertPosition(g_app.shmManager.GetBiLinkTail(), userinfo.userScore);
                        LinkNode * toBeInsertNode = currentPosition;
                        if (currentPosition == g_app.shmManager.GetBiLinkTail())
                        {
                            g_app.shmManager.SetBiLinkTail(g_app.shmManager.GetBiLinkTail()->prev);
                            g_app.shmManager.GetBiLinkTail()->next = NULL;

                            toBeInsertNode->prev = NULL;
                            toBeInsertNode->next = NULL;
                            toBeInsertNode->userinfo.userScore = userinfo.userScore;
                            InsertLinkNode(insertPosition, toBeInsertNode);

                        }
                        else if (currentPosition == g_app.shmManager.GetBiLinkHead())
                        {
                            g_app.shmManager.GetBiLinkHead()->userinfo.userScore = userinfo.userScore;
                        }
                        else
                        {
                            toBeInsertNode->prev->next = toBeInsertNode->next;
                            toBeInsertNode->next->prev = toBeInsertNode->prev;
                            toBeInsertNode->prev = NULL;
                            toBeInsertNode->next = NULL;
                            toBeInsertNode->userinfo.userScore = userinfo.userScore;
                            InsertLinkNode(insertPosition, toBeInsertNode);
                        }
                    }
                } 
            }
            UpdateLinkRank();
            UpdatePtrArray();
        }
    }
    //OutputEntireLink(); 
    //OutputEntirePtrarray();
    //LOG->debug()<<"leave rankServantImp::UploadUserInfo()"<<endl;
    return 0;
}

int rankServantImp::DownloadRankPage(int pageNO, int pageUnit, vector<xxx::userInfo> &output, taf::JceCurrentPtr current)
{
    int startIndex = (pageNO-1)*pageUnit+1;
    if (pageNO <= 0 || pageUnit <= 0 || startIndex > g_app.shmManager.GetShmCheckHead()->nodeCount)
    {
        return -1;
    }
    else
    {
        int i = 0;
        xxx::userInfo currentNode;
        while (*(g_app.shmManager.GetOffsetArray()+startIndex+i-1) != -1 && i < pageUnit)
        {
            int offset = *(g_app.shmManager.GetOffsetArray()+startIndex+i-1);
            LinkNode *currentPageNode= (LinkNode *)(g_app.shmManager.GetShmPtr()+offset);
            currentNode.userId = currentPageNode->userinfo.userId;
            currentNode.userScore = currentPageNode->userinfo.userScore;
            currentNode.userRank = currentPageNode->userinfo.userRank;
            output.push_back(currentNode);
            ++i;
        }
    }
    //OutputEntireLink();
    //OutputEntirePtrarray();
    return 0;
}

int rankServantImp::GetUserRank(int userid, int &userrank, taf::JceCurrentPtr current)
{
   map<int, LinkNode*>::iterator it = g_app.m_userid2LinkNode.find(userid); 
   if (it == g_app.m_userid2LinkNode.end())
   {
       return -1;
   }
   else
   {
       userrank = (it->second)->userinfo.userRank;
       return 0;
   }
}
