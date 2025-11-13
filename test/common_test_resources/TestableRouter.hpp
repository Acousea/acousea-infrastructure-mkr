#ifndef ACOUSEA_INFRASTRUCTURE_MKR_TESTABLEROUTER_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_TESTABLEROUTER_HPP

#include "Router.h"

/**
 * @brief Minimal friend class for testing Router internals externally.
 *
 * This class exists only to be declared as a friend of Router,
 * so test scripts or tools can access Router’s private methods like
 * encodePacket() and decodePacket() if needed.
 */
class TestableRouter
{
public:
    using Router::Router; // hereda el constructor

    TestableRouter() = default;
};

#endif //ACOUSEA_INFRASTRUCTURE_MKR_TESTABLEROUTER_HPP