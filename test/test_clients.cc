// to use c++ style logging
// specifically define this before including loguru.hpp
#define LOGURU_WITH_STREAMS 1

#include <grpcpp/grpcpp.h>

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "protos/zirconiumbank.grpc.pb.h"
#include "test/zirconium_client.hpp"
#include "utils/loguru.hpp"

// gRPC namespaces
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

// zirconium grpc-proto namespaces
using zirconium::bank::AuthUser;
using zirconium::bank::Balance;
using zirconium::bank::RequestAmount;
using zirconium::bank::TransferReq;
using zirconium::bank::WebService;

class zirconium_client_test : public testing::Test {
   public:
    ZirconiumClient zirconium_bank;

    zirconium_client_test() : zirconium_bank(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials())) {
        // turn off the uptime in the output
        loguru::g_preamble_uptime = false;
    }
};

TEST_F(zirconium_client_test, transaction_conflict_test) {
    int NUM_WORKERS = 100;
    LOG_S(INFO) << "TESTING 100 Clients concurrent transaction";
    std::vector<std::thread> workers;

    // create two users

    // run transactions between the two users
    for (int i = 0; i < NUM_WORKERS; i++) {
        workers.push_back(std::thread([&]() {
            LOG_S(INFO) << i;
        }));
    }

    // join the threads
    std::for_each(workers.begin(), workers.end(), [](std::thread &t) {
        t.join();
    });
}
