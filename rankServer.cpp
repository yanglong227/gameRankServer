#include "rankServer.h"
#include "rankServantImp.h"
#include "util/tc_common.h"

using namespace std;

rankServer g_app;

void rankServer::initialize()
{
    if (shmManager.InitializeShareMemory() == 1)
    {
        LinkNode * p = shmManager.GetBiLinkHead();
        while (p != NULL)
        {
            m_userid2LinkNode.insert(pair<int, LinkNode *>(p->userinfo.userId, p));
            p = p->next;
        }
    }
    
 	addServant<rankServantImp>(ServerConfig::Application + "." + ServerConfig::ServerName + ".rankServantObj");
}

void
rankServer::destroyApp()
{
	LOG->debug()<<"##rankServer::destoryApp()\n";
    shmdt(shmManager.GetShmPtr());
    //shmctl(m_shmid, IPC_RMID, 0);
    LOG->debug()<<"##dettach share memory success\n";
}

int
main(int argc, char* argv[])
{
	try
	{
		g_app.main(argc, argv);
		g_app.waitForShutdown();
	}
	catch (std::exception& e)
	{
		cerr << "std::exception:" << e.what() << std::endl;
	}
	catch (...)
	{
		cerr << "unknown exception." << std::endl;
	}
	return -1;
}
