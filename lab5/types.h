#pragma once

#pragma pack(push, 1)
struct employee {
    int num;
    char name[10];
    double hours;
};
#pragma pack(pop)

enum RequestType {
    REQ_READ          = 0,
    REQ_MODIFY        = 1,
    REQ_WRITE_DATA    = 2,
    REQ_RELEASE_READ  = 3,
    REQ_RELEASE_WRITE = 4
};

enum ResponseStatus {
    STATUS_OK        = 0,
    STATUS_LOCKED    = 1,
    STATUS_NOT_FOUND = 2
};

struct Request {
    int type;
    int key;
};

struct Response {
    int status;
    employee record;
};
