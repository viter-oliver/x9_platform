#ifndef CZHMSGSERVER_H
#define CZHMSGSERVER_H
#include <fdbus/fdbus.h>
#include <fdbus/cJSON/cJSON.h>
#include <fdbus/CFdbCJsonMsgBuilder.h>
#include <fdbus/fdbus.h>
#include "CZhMsgManager.h"

using namespace ipc::fdbus;

class CFdbServer : public CBaseServer, public CZhBaseObj
{
public:
    CFdbServer(const char *name, CBaseWorker *worker = 0);

  

protected:  

    virtual void onSubscribe(CBaseJob::Ptr &msg_ref);

    virtual void onInvoke(CBaseJob::Ptr &msg_ref);

    virtual void onOffline(const CFdbOnlineInfo &info);

    virtual void onOnline(const CFdbOnlineInfo &info);

private:


};





class CZhMsgServer
{
public:

    typedef std::map<int32_t, FunCb> CallCbList;
    typedef std::map<int32_t, NotifyCb> NotifyCbList;

    typedef std::map<int32_t, CProtoCb*> CProtoCbList;

    
    CZhMsgServer(const char *name, bool singleWork = false);

    virtual ~CZhMsgServer();

    // notify to client
    int Notify(const int32_t msgId, const void* pMsg, const int size);

    int Notify(const int32_t msgId, google::protobuf::MessageLite &message);

    // client->server and response
    int RegFuncCall(const int32_t msgId, FunCb funcb);

    int RegFuncCall(const int32_t msgId, NotifyCb notifyCb, google::protobuf::MessageLite &message);  

    // client->server and not response
    int RegFuncCall(const int32_t msgId, NotifyCb funcb);






  class ServerMsgStateNotify : public MsgStateNotify
    {
    public:
        ServerMsgStateNotify(CZhMsgServer *obj);

        int onSubscribe(const int32_t msgId);

        void onInvoke(CBaseJob::Ptr &msg_ref);

    private:
        CZhMsgServer* mpCZhMsgServer;

    };      

protected:

    virtual int onSubscribe(const int32_t msgId);



private:

    void onInvoke(CBaseJob::Ptr &msg_ref);

    
    CFdbServer* mpFdbServer;

    ServerMsgStateNotify* mpServerMsgStateNotify;

    CallCbList mCallCbList;

    CProtoCbList mCProtoCbList;

    NotifyCbList mNotifyCbList;

    std::string mName;

};


#endif // CZHMSGSERVER_H