#ifndef STRUCT_DEF_H
#define STRUCT_DEF_H

#define MAX_USER 10000

typedef struct UserInfo
{
    int userId;
    int userScore;
    int userRank;
}UserInfo;

typedef struct LinkNode
{
    UserInfo userinfo;
    LinkNode *next;
    LinkNode *prev;
}LinkNode;

typedef struct ShmCheckHead
{
    int ifValidShmSpace;
    int nodeCount;
}ShmCheckHead;
#endif
