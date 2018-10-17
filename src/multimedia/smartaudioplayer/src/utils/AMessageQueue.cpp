#include "AMessageQueue.h"
#include "pthread.h"
#include "Log.h"
#include "assert0.h"

AMessageQueue::AMessageQueue()
      :mHandle(NULL_HANDLE) {

    char s_name[16+1];
    status_t ret = OK;
    snprintf(s_name, sizeof(s_name), "mq-%p", this);
    printf("(%p)create messagequeue name %s\n", this, s_name);

    ret = x_msg_q_create(&mHandle,
                            s_name,
                            sizeof(AMessage),
                            512);
    assert0(ret==OSR_OK);
}

AMessageQueue::~AMessageQueue() {

    status_t ret = OK;

    if (mHandle != NULL_HANDLE) {
        printf("(%p)free messagequeue \n", this);
        ret = x_msg_q_delete(mHandle);
        assert0(ret==OSR_OK);
        mHandle = NULL_HANDLE;
    }
}

status_t AMessageQueue::SendMessage(AMessage *message) {
    int  ret;
    int count = 0;

    if (NULL_HANDLE == mHandle) {
        printf("%p Message queue not exist or thread must exit.", this);
        return UNKNOWN_ERROR;
    }

    while((ret = x_msg_q_send(mHandle, (const VOID *)message, sizeof(AMessage), 1)) != OSR_OK) {
        count++;
        assert0(count<=2);
    }
    ret = (ret == OSR_OK) ? OK : UNKNOWN_ERROR;
    return ret;
}


void AMessageQueue::ReceiveMessage(AMessage *message)
{
    size_t  msg_size = 0;
    unsigned short index = 0;

    msg_size = sizeof(AMessage);

    if(NULL_HANDLE == mHandle) {
        printf("%p Message queue not exist or thread must exit.", this);
        return;
    }

    while(x_msg_q_receive(&index,
                        message,
                        &msg_size,
                        &mHandle, 1,
                        X_MSGQ_OPTION_WAIT) != OSR_OK) {
        printf("%p ReceiveMessage Failed.");
        continue;
    }
}

