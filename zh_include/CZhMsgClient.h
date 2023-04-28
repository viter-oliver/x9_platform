#ifndef CZHMSGCLIENT_H
#define CZHMSGCLIENT_H

#include <fdbus/fdbus.h>
#include <fdbus/cJSON/cJSON.h>
#include <fdbus/CFdbCJsonMsgBuilder.h>

#include <fdbus/fdbus.h>
#include <mutex>
#include "CZhMsgManager.h"
using namespace ipc::fdbus;

class CFdbClient : public CBaseClient, public CZhBaseObj
{
public:
    CFdbClient(const char *name, CBaseWorker *worker = 0);



protected:  

    virtual void onBroadcast(CBaseJob::Ptr &msg_ref);

    virtual void onReply(CBaseJob::Ptr &msg_ref);

    virtual void onOffline(const CFdbOnlineInfo &info);

    virtual void onOnline(const CFdbOnlineInfo &info);


private:

};



class CZhMsgClient
{
    
public:
    typedef std::map<int32_t, NotifyCb> NotifyCbList;

    typedef std::map<int32_t, CProtoCb*> CProtoCbList;
    
    CZhMsgClient(const char *name, bool singleWork = false);

    ~CZhMsgClient();

    //Register msgId listen
    int RegNotify(const int32_t msgId, NotifyCb notifyCb);

    int RegNotify(const int32_t msgId, NotifyCb notifyCb, google::protobuf::MessageLite &message);

    int SyncCall(const int32_t msgId, const void* pInput, const int32_t inSize, RespItem* resp, int32_t timeout = 2000);

    int SyncCall(const int32_t msgId, const void* pInput, const int32_t inSize, int32_t timeout = 2000);

    int SyncCall(const int32_t msgId, google::protobuf::MessageLite &req, google::protobuf::MessageLite &resp, int32_t timeout = 2000);    

    int AsyncCall(const int32_t msgId, const void* pInput, const int32_t inSize);

    int SyncCall(const int32_t msgId, google::protobuf::MessageLite &message, int32_t timeout = 2000);


    class ClientMsgStateNotify : public MsgStateNotify
    {

    public:
        ClientMsgStateNotify(CZhMsgClient *obj);

        int onOnline(const int32_t id);

        int onOffline(const int32_t id);
    private:
        CZhMsgClient* mpZhMsgClient;


    };
    



private:

    void MsgRecv(const int32_t msgId, const void* pMsg, const int32_t size);

    void BaseMsgRecv(CBaseMessage *pMsg);

    int onOnline(const int32_t id);

    int onOffline(const int32_t id);    

    NotifyCbList mNotifyCbList;

    ProtoMsgParserCb mProtoMsgParserCb;

    CProtoCbList mCProtoCbList;

    CFdbClient* mpFdbClient;

    ClientMsgStateNotify* mpClientMsgStateNotify;


    std::mutex mLock;

    std::string mName;



    friend class CFdbClient;

};

#endif // CZHMSGCLIENT_H