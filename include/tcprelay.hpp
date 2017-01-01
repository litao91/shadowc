#ifndef SHADOW_C_TCPRELAY_H__
#define SHADOW_C_TCPRELAY_H__
#include "eventloop.hpp"

namespace tcprelay {
class TCPRelayHandler: public eventloop::EventHandler {
    public:
        TCPRelayHandler();
        virtual ~TCPRelayHandler();
};
}
#endif
