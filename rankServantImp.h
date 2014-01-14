#ifndef _rankServantImp_H_
#define _rankServantImp_H_

#include <sys/shm.h>
#include <map.h>

#include "servant/Application.h"
#include "rankServant.h"
#include "rankServer.h"

/*#define MAX_USER 10000


typedef struct UserInfo
{
    int userId;
    int userScore;
    int userRank;
}UserInfo;

typedef struct LinkNode
{
    UserInfo userinfo;
    LinkNode *prev;
    LinkNode *next;
}LinkNode;

typedef struct ShmCheckHead
{
    int ifValidSpace;
    int nodeCout;
}ShmCheckHead;
*/

class rankServantImp : public xxx::rankServant
{
public:
	/**
	 */
	virtual ~rankServantImp() {}

	/**
	 */
	virtual void initialize();

	/**
	 */
    virtual void destroy();

	/**
	 */
	virtual int test(taf::JceCurrentPtr current) { return 0;};
    
    virtual int UploadUserInfo(const xxx::userInfo &userinfo, taf::JceCurrentPtr current);
    virtual int DownloadRankPage(int pageNO, int pageUnit, vector<xxx::userInfo> &output, taf::JceCurrentPtr current);
    virtual int GetUserRank(int userid, int &userrank, taf::JceCurrentPtr current);

private:
    int UpdateLinkRank();
    int UpdatePtrArray();
    int InsertFirstNode(const xxx::userInfo & userinfo);
    pair<int, LinkNode *> IfExistedUser(int userid);                          
    LinkNode * FindInsertPosition(LinkNode * startPositon, int score);
    int InsertLinkNode(LinkNode * insertPosition, LinkNode * toBeInsertNode);

    int OutputEntireLink();
    int OutputEntireMap();
    int OutputEntirePtrarray();
};
#endif
