#ifndef BINANCEFEED_H
#define BINANCEFEED_H

#include <vector>
#include <stop_token>

namespace MarketData {
    struct OrderBookSnapshot {
        std::int64_t last_update_id;
        std::vector<std::pair<double, double>> bids;
        std::vector<std::pair<double, double>> asks;
    };

    class BinanceFeed {
    public:
        BinanceFeed();
        void run();

    private:
        void websocket_worker(std::stop_token token);
        OrderBookSnapshot fetch_order_book_snapshot();
    };
}

#endif
