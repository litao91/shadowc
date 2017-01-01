#include "tcprelay.hpp"


namespace tcprelay {
    //  as ssserver:
    //  stage 0 just jump to stage 1
    //  stage 1 addr received from local, query DNS for remote
    //  stage 3 DNS resolved, connect to remote
    //  stage 4 still connecting, more data from local received
    //  stage 5 remote connected, piping local and remote
    const static char STAGE_INIT = 0;
    const static char STAGE_ADDR = 1;
    const static char STAGE_UDP_ASSOC = 2;
    const static char STAGE_DNS = 3;
    const static char STAGE_CONNECTING = 4;
    const static char STAGE_STREAM = 5;
    const static char STAGE_DESTROYED = -1;

//  for each handler, we have 2 stream directions:
//     upstream:    from client to server direction
//                  read local and write to remote
//     downstream:  from server to client direction
//                  read remote and write to local

    const static char STREAM_UP = 0;
    const static char STREAM_DOWN = 1;

    // for each stream, it's waiting for reading, or writing, or both
    const static char WAIT_STATUS_INIT = 0;
    const static char WAIT_STATUS_READING = 1;
    const static char WAIT_STATUS_WRITING = 2;
    const static char WAIT_STATUS_READWRITING = WAIT_STATUS_READING | WAIT_STATUS_WRITING;

    const static int BUF_SIZE = 32 * 1024;
}
