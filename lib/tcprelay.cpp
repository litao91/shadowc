#include "tcprelay.hpp"

//  as ssserver:
//  stage 0 just jump to stage 1
//  stage 1 addr received from local, query DNS for remote
//  stage 3 DNS resolved, connect to remote
//  stage 4 still connecting, more data from local received
//  stage 5 remote connected, piping local and remote

namespace tcprelay {
}
