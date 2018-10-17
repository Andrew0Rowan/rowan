//
// Created by yuyaohui on 17-08-20.
//

#ifndef DUER_LINK_RECEIVED_DATA_OBSERVER_H
#define DUER_LINK_RECEIVED_DATA_OBSERVER_H

#include <string>

class DuerLinkReceivedDataObserver {
public:
    virtual std::string NotifyReceivedData(const std::string& jsonPackageData,int iSessionID = -1) = 0;
};

#endif //DUER_LINK_RECEIVED_DATA_OBSERVER_H
