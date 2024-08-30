#pragma once

class ClientReadHandler {
public:
    virtual bool clientStateServer();
    virtual bool clientStateRoom();
    virtual void clientStateError() = 0;
};
