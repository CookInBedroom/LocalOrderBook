# Creates a MarketData library target
# that contains logic in the MarketData namespace
add_library(MarketData
        BinanceFeed.h
        BinanceFeed.cpp
        LockFreeRingBuffer.h
)

# Third-party dependencies
find_package(Boost REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(cpr REQUIRED)

target_link_libraries(MarketData
        PUBLIC
            Boost::headers
            nlohmann_json::nlohmann_json
            cpr::cpr
)

# Makes import cleaner
# #include "BinanceFeed.h" over #include "src/MarketData/BinanceFeed.h"
target_include_directories(MarketData
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)