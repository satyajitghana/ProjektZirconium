// to use c++ style logging
// specifically define this before including loguru.hpp
#define LOGURU_WITH_STREAMS 1

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "protos/zirconiumbank.grpc.pb.h"
#include "utils/loguru.hpp"
#include "utils/zirconium_utils.hpp"

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

class ZirconiumClient {
   public:
    ZirconiumClient(std::shared_ptr<Channel> channel) : stub_(WebService::NewStub(channel)) {}

    auto BalanceEnquiry(const std::string& user, const std::string& pin) {
        // Data that we will send to the server
        AuthUser authuser_req;
        authuser_req.set_username(user);
        authuser_req.set_pin(pin);

        // The Response we will get
        Balance balance_res;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC
        Status status = stub_->BalanceEnquiry(&context, authuser_req, &balance_res);

        // return getResponse_(status, user, balance_res);
        return status;
    }

    auto UpdateAmount(const std::string& user, const std::string& pin, int update_amt) {
        // the despoit amount request
        RequestAmount update_amt_req;

        LOG_IF_S(WARNING, update_amt == 0) << "update amount = 0";

        LOG_IF_S(WARNING, update_amt < 0) << "Debit Req of Amount: " << update_amt << " by user: " << user;
        LOG_IF_S(WARNING, update_amt > 0) << "Credit Req of Amount: " << update_amt << " by user: " << user;

        // create the request payload
        update_amt_req.mutable_user()->set_username(user);
        update_amt_req.mutable_user()->set_pin(pin);
        update_amt_req.set_value(update_amt);

        // The Response we will get
        Balance balance_res;

        ClientContext context;

        // The DepositAmount RPC
        Status status = stub_->UpdateAmount(&context, update_amt_req, &balance_res);

        // return getResponse_(status, user, balance_res);
        return status;
    }

    auto TransferAmount(const std::string& user, const std::string& pin, const std::string& to_user, int trans_amt) {
        // the transfer amount request
        TransferReq trans_amt_req;

        if (trans_amt < 0) {
            LOG_S(ERROR) << "Transfer Amount cannot be negative, wtf you trying to do?";
            exit(0);
        }

        LOG_IF_S(WARNING, trans_amt == 0) << "transfer amount = 0";
        LOG_IF_S(WARNING, trans_amt > 0) << "Transfer Req of Amount: " << trans_amt << " by user: " << user;

        trans_amt_req.mutable_from_user()->set_username(user);
        trans_amt_req.mutable_from_user()->set_pin(pin);
        trans_amt_req.set_to_user(to_user);
        trans_amt_req.set_amount(trans_amt);

        // The Response we get
        Balance balance_res;

        ClientContext context;

        // The Transfer Amount RPC
        Status status = stub_->TransferAmount(&context, trans_amt_req, &balance_res);

        // return getResponse_(status, user, balance_res);
        return status;
    }

   private:
    std::unique_ptr<WebService::Stub> stub_;

    ClientContext ctx;

    std::string getResponse_(Status status, std::string user, Balance balance_res) {
        if (status.ok()) {
            std::stringstream response;
            response << "STATUS OK [ user: " << user << ", balance: " << std::to_string(balance_res.value()) << " ]";
            LOG_S(INFO) << response.str();

            return response.str();
        } else {
            std::stringstream response;
            response << "RPC FAILED [ " << status.error_code() << ": " << status.error_message();

            LOG_S(ERROR) << response.str();

            return response.str();
        }
    }
};
