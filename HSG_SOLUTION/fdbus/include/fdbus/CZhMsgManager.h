#ifndef CZHMSGMANAGER_H
#define CZHMSGMANAGER_H

#include <iostream>
#include <functional>
#include <fdbus/fdbus.h>
#include <fdbus/CFdbProtoMsgBuilder.h>
#include <fdbus/CFdbProtoMsgBuilder.h>

using namespace ipc::fdbus;

class RespItem
{
public:
    RespItem()
    {
        pMsg = nullptr; 
        size = 0;
    }

    bool SetData(const void* msg, const int32_t len)
    {
        try
        {
            pMsg = new uint8_t[len];
            size =len;
        }
        catch (...)
        {
            return false;
        }        
        memcpy(pMsg, msg, size);
        return true;
    }

    ~RespItem()
    {
        if (pMsg)
            delete[] pMsg;
    }
    uint8_t* pMsg;
    int32_t size;
};



typedef std::function<void(const void* pMsg, const int32_t size)> NotifyCb;

typedef std::function<void(google::protobuf::MessageLite*)> ProNotifyCb;

typedef std::function<void(CFdbProtoMsgParser&)> ProtoMsgParserCb;

typedef std::function<void(const int32_t msgId, const void* pMsg, const int32_t size)> MsgNotifyCb;

typedef std::function<int(const void* pMsg, const int32_t size, RespItem* resp)> FunCb;

typedef std::function<void(CBaseMessage *pMsg)> MsgCb;

#define REG_FUNC( FID, FUNC ) \
    RegFuncCall( FID, std::bind( FUNC, this, std::placeholders::_1, std::placeholders::_2 ) )

#define REG_FUNC_RESP( FID, FUNC ) \
    RegFuncCall( FID, std::bind( FUNC, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) )

class CProtoCb
{
public:

    CProtoCb(google::protobuf::MessageLite& msg, NotifyCb cb)
        :mMsg(msg), mMsgResp(msg), mNotifyCb(cb)
    {

    }

    CProtoCb(google::protobuf::MessageLite& msg, google::protobuf::MessageLite& msgResp, NotifyCb cb)
        :mMsg(msg), mMsgResp(msgResp), mNotifyCb(cb)
    {

    }    

    ~CProtoCb(){}

public:    
    google::protobuf::MessageLite& mMsg;
    google::protobuf::MessageLite& mMsgResp;
    NotifyCb mNotifyCb;
};    

class MsgStateNotify
{
public:
    virtual int onOnline(const int32_t id){return 0;};

    virtual int onOffline(const int32_t id){return 0;};

    virtual int onSubscribe(const int32_t msgId){return 0;};

    virtual void onInvoke(CBaseJob::Ptr &msg_ref){};
};

class CZhBaseObj
{

public:

    CZhBaseObj()
    {
        mMsgStateNotify = 0;
        mMsgNotifyCb = 0;
    }
    int RegMsgStateNotify(MsgStateNotify* obj){mMsgStateNotify = obj;return 0;};

    int UnRegMsgStateNotify(){mMsgStateNotify = 0;return 0;};

    int RegMsgNotify(MsgNotifyCb cb){mMsgNotifyCb = cb;return 0;};

    int UnRegMsgNotify(MsgNotifyCb cb){mMsgNotifyCb = 0;return 0;};

    int RegBaseMsgNotify(MsgCb cb){mMsgCb = cb;return 0;};

    int UnBaseRegMsgNotify(MsgCb cb){mMsgCb = 0;return 0;};
    


protected:
    MsgStateNotify* mMsgStateNotify;

    MsgNotifyCb mMsgNotifyCb;

    MsgCb mMsgCb;

};

class CZhMsgManager
{
public:
    ~CZhMsgManager();
    static CZhMsgManager* GetInstance();   

    CBaseWorker* GetWorker(const char* pName);

    int RemoveWorker(const char* pName);

    CBaseWorker* GetServerWorker();

    CBaseWorker* GetClientWorker();

private:
    CZhMsgManager();

    std::map<std::string, CBaseWorker*> mSvrWorkerList;

    std::mutex mLock;


};

#endif // CZHMSGMANAGER_H