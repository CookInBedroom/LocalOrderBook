#include <boost/asio.hpp>
#include <cpr/cpr.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>
#include <stop_token>

#include "BinanceFeed.h"

namespace asio = boost::asio;
using json = nlohmann::json;

namespace MarketData {
    BinanceFeed::BinanceFeed() = default;

    void BinanceFeed::run() {
        // JThread for the websocket loop
        const std::stop_source s_source {};

        std::jthread websocket_thread ([this, token = s_source.get_token()] {
            websocket_worker(token);
        });

        OrderBookSnapshot snapshot = fetch_order_book_snapshot();
    }

    void BinanceFeed::websocket_worker(std::stop_token token) {
        // 1. When receiving a Ping, returning a pong
        // 2. When receiving a server shutdown message -> reconnect
        // 3. If first websocket's U is less than the snapshot's lastUpdateId, then we need to fetch a new snapshot and restart the process
        while (!token.stop_requested()) {
            std::cout << "[WebSocket] Receiving incoming data..." << std::endl;


        }
    }

    /// Fetches an order book snapshot from Binance's REST endpoint.
    /// The snapshot is used as an initial state for a local order book.
    OrderBookSnapshot BinanceFeed::fetch_order_book_snapshot() {
        std::cout << "[REST] Sending order book snapshot request..." << std::endl;

        OrderBookSnapshot snapshot {};

        cpr::Response response = cpr::Get(
            cpr::Url{"https://api.binance.com/api/v3/depth"},
            cpr::Parameters{
                {"symbol", "BTCUSDT"},
                {"limit", "20"} // top 20 records
            },
            cpr::Timeout{4000} // 4-second timeout threshold
        );

        if (response.status_code != 200) {
            std::cerr << "[Error] Failed REST call with response status code: " << response.status_code << std::endl;
            return snapshot;
        }

        /* Parse the JSON response and store in a snapshot struct */
        try {
            json json_response = json::parse(response.text);

            snapshot.last_update_id = json_response.at("lastUpdateId").get<long long>();

            for (const json &bid_entry: json_response.at("bids")) {
                double price = std::stod(bid_entry[0].get<std::string>());
                double quantity = std::stod(bid_entry[1].get<std::string>());
                snapshot.bids.emplace_back(price, quantity);
            }

            for (const json &ask_entry: json_response.at("asks")) {
                double price = std::stod(ask_entry[0].get<std::string>());
                double quantity = std::stod(ask_entry[1].get<std::string>());
                snapshot.asks.emplace_back(price, quantity);
            }
        } catch (const std::exception &e) {
            std::cerr << "[Error] Failed to parse order book snapshot: " << e.what() << std::endl;
        }

        return snapshot;
    }
}
