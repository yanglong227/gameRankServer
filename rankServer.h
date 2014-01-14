#ifndef _rankServer_H_
#define _rankServer_H_

#include <iostream>
#include <map>
#include <sys/shm.h>

#include "servant/Application.h"
#include "StructDef.h"
#include "shmMemoryManager.h"


using namespace taf;

class rankServer : public Application
{
public:
	virtual ~rankServer() {};
	virtual void initialize();
	virtual void destroyApp();

    map<int, LinkNode *> m_userid2LinkNode;
    ShmMemoryManager shmManager;
};

extern rankServer g_app;
#endif
