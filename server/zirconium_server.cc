// to use c++ style logging
// specifically define this before including loguru.hpp
#define LOGURU_WITH_STREAMS 1

#include <grpcpp/grpcpp.h>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>
#include <memory>
#include <mongocxx/client.hpp>
#include <mongocxx/exception/operation_exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <stdexcept>
#include <string>
#include <thread>

#include "protos/zirconiumbank.grpc.pb.h"
#include "server/transaction_mgr.hpp"
#include "utils/loguru.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

// gRPC namespaces
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

// zirconium grpc-proto namespaces
using zirconium::bank::AuthUser;
using zirconium::bank::Balance;
using zirconium::bank::RequestAmount;
using zirconium::bank::TransferReq;
using zirconium::bank::WebService;

class WebServiceImpl final : public WebService::Service {
    /**
     * BalanceEnquiry: Gets the balance of a AuthUser
    */
    Status BalanceEnquiry(ServerContext* context, const AuthUser* auth_user, Balance* balance) override {
        // get a client from the pool
        auto client = conn_pool_->acquire();

        // log the auth data from the request
        logAuthData_(auth_user->username(), auth_user->pin());

        // check if the user is authenticated and also fetch the document
        auto [is_authenticated, user] = getAuthData_(auth_user->username(), auth_user->pin(), *(client));

        if (not is_authenticated) {
            LOG_S(WARNING) << "UNAUTHENTICATED User: " << auth_user->username();
            return Status(StatusCode::UNAUTHENTICATED, "username or pin wrong!");
        }

        // the user is authenticated

        balance->set_value(user->view()["balance"].get_int32());

        return Status::OK;
    }

    /**
     * UpdateAmount : updates the balance of the user with the given amount
     * Tested Works ✅
    */
    Status UpdateAmount(ServerContext* context, const RequestAmount* update_req, Balance* balance) override {
        LOG_S(INFO) << "UpdateAmount called by peer: " << context->peer();

        // get a client from the pool
        auto client = conn_pool_->acquire();

        // get the "zirconium" db
        auto db = (*client)["zirconium"];

        const auto auth_user = update_req->user();
        logAuthData_(auth_user.username(), auth_user.pin());

        try {
            auto [is_authenticated, user] = getAuthData_(auth_user.username(), auth_user.pin(), (*client));

            if (not is_authenticated) {
                LOG_S(WARNING) << "UNAUTHENTICATED User: " << auth_user.username();
                return Status(StatusCode::UNAUTHENTICATED, "username or pin wrong!");
            }

            // the user is authenticated

            auto update_user_amount = [&](mongocxx::client_session& session) {
                // get the transaction options
                auto txn_opts = zirconium::transaction_mgr::get_transaction_opts();

                // initiate the transaction
                session.start_transaction(txn_opts);

                // container for the updated user
                bsoncxx::stdx::optional<bsoncxx::document::value> updated_user;

                // try the transaction
                try {
                    // read fuser value from db
                    auto curr_user = db["bank"].find_one(session, make_document(kvp("username", auth_user.username())));
                    auto curr_user_balance = curr_user->view()["balance"].get_int32().value;

                    // guard to check if from_user has necessary balance
                    if (curr_user_balance + update_req->value() < 0) {
                        // insufficient balance
                        throw zirconium::exception("insufficient balance");
                    }

                    // increment/decrement the balance of user
                    updated_user = db["bank"].find_one_and_update(
                        session,
                        curr_user->view(),
                        make_document(
                            kvp("$inc", make_document(kvp("balance", update_req->value())))),
                        mongocxx::options::find_one_and_update().return_document(mongocxx::options::return_document::k_after));

                    // set the new balance as responce
                    balance->set_value(updated_user->view()["balance"].get_int32());

                } catch (const mongocxx::operation_exception& oe) {
                    session.abort_transaction();
                    throw oe;
                }

                // run this if the transaction was successful
                auto on_success = [&](void) {
                    LOG_S(INFO) << "UPDATED: " << bsoncxx::to_json(*updated_user) << "\n";
                    auto update_type = update_req->value() >= 0 ? "credited" : "debited";

                    LOG_S(INFO) << "UpdateAmount [ Account " << updated_user->view()["username"].get_utf8().value.to_string() << " " << update_type << " with ₹ " << std::abs(update_req->value()) << " new balance: " << updated_user->view()["balance"].get_int32() << " ]";
                };

                // commit the transaction and retry if failure
                zirconium::transaction_mgr::commit_with_retry(session, on_success);
            };

            // create a session and start a transaction
            auto session = (*client).start_session();

            try {
                zirconium::transaction_mgr::run_transaction_with_retry(update_user_amount, session);
            } catch (const mongocxx::operation_exception& oe) {
                LOG_S(ERROR) << "Error during commit: " << oe.what() << ", for session: " << bsoncxx::to_json(session.id());
            }

        } catch (zirconium::exception& ex) {
            LOG_S(ERROR) << "Precondition failed: " << ex.what();
            return Status(StatusCode::FAILED_PRECONDITION, ex.what());
        } catch (std::exception& ex) {
            LOG_S(ERROR) << "Internal Server Error: " << ex.what();
            return Status(StatusCode::INTERNAL, "Internal Server Error, Cannot Update Amount");
        }

        return Status::OK;
    }

    /**
     * Transfer Amount: Transfer amount from current user to to_user
     * Tested Works ✅
    */
    Status TransferAmount(ServerContext* context, const TransferReq* transfer_req, Balance* balance) override {
        LOG_S(INFO) << "TransferAmount called by peer: " << context->peer();

        // get a client from the pool
        auto client = conn_pool_->acquire();

        // get the "zirconium" db
        auto db = (*client)["zirconium"];

        // curr_user
        const auto auth_user = transfer_req->from_user();
        const auto to_user = transfer_req->to_user();

        // guard to check if the to_user is same as curr_user
        if (to_user == auth_user.username()) {
            return Status(StatusCode::FAILED_PRECONDITION, "to_user is same as curr_user");
        }

        logAuthData_(auth_user.username(), auth_user.pin());

        try {
            auto [is_authenticated, user] = getAuthData_(auth_user.username(), auth_user.pin(), (*client));

            // check if the user is authenticated
            if (not is_authenticated) {
                LOG_S(WARNING) << "UNAUTHENTICATED User: " << auth_user.username();
                return Status(StatusCode::UNAUTHENTICATED, "username or pin wrong!");
            }

            // the user is authenticated

            // transaction function
            auto transfer_user_amount = [&](mongocxx::client_session& session) {
                // get the transaction options
                auto txn_opts = zirconium::transaction_mgr::get_transaction_opts();

                // initiate the transaction
                session.start_transaction(txn_opts);

                // container for the updated user
                bsoncxx::stdx::optional<bsoncxx::document::value> updated_from_user;

                // try the transaction
                try {
                    // read to_user value from db
                    auto curr_to_user = db["bank"].find_one(session, make_document(kvp("username", to_user)));

                    // this is unnecessary, can be ommited
                    if (not curr_to_user) {
                        // to_user does not exist, abort transaction
                        throw zirconium::exception("to_user does not exist");
                    }

                    // read from_user value from db
                    auto curr_from_user = db["bank"].find_one(session, make_document(kvp("username", auth_user.username())));
                    auto curr_from_user_balance = curr_from_user->view()["balance"].get_int32().value;

                    // guard to check if from_user has necessary balance
                    if (curr_from_user_balance - transfer_req->amount() < 0) {
                        // insufficient balance
                        throw zirconium::exception("insufficient balance");
                    }

                    // decrement the balance of from_user
                    updated_from_user = db["bank"].find_one_and_update(
                        session,
                        curr_from_user->view(),
                        make_document(
                            kvp("$inc", make_document(
                                            kvp("balance", -transfer_req->amount())))),
                        mongocxx::options::find_one_and_update().return_document(mongocxx::options::return_document::k_after));

                    // increment the balance of to_user
                    db["bank"].find_one_and_update(
                        session,
                        curr_to_user->view(),
                        make_document(
                            kvp("$inc", make_document(
                                            kvp("balance", transfer_req->amount())))),
                        mongocxx::options::find_one_and_update());

                    // set the new balance as responce
                    balance->set_value(updated_from_user->view()["balance"].get_int32());

                } catch (const mongocxx::operation_exception& oe) {
                    session.abort_transaction();
                    throw oe;
                } catch (const zirconium::exception& ex) {
                    session.abort_transaction();
                    throw ex;
                }

                // run this if the transaction was successful
                auto on_success = [&](void) {
                    LOG_S(INFO) << "UPDATED: " << bsoncxx::to_json(*updated_from_user) << "\n";
                    auto update_type = transfer_req->amount() >= 0 ? "credited" : "debited";

                    LOG_S(INFO) << "UpdateAmount [ Account " << updated_from_user->view()["username"].get_utf8().value.to_string() << " " << update_type << " with ₹ " << std::abs(transfer_req->amount()) << " new balance: " << updated_from_user->view()["balance"].get_int32() << " ]";
                };

                // commit the transaction and retry if failure
                zirconium::transaction_mgr::commit_with_retry(session, on_success);
            };

            // create a session and start a transaction
            auto session = client->start_session();

            try {
                zirconium::transaction_mgr::run_transaction_with_retry(transfer_user_amount, session);
            } catch (const mongocxx::operation_exception& oe) {
                // some expection occured during commit
                LOG_S(ERROR) << "Error during commit: " << oe.what() << ", for session: " << bsoncxx::to_json(session.id());
            }
        } catch (zirconium::exception& ex) {

            LOG_S(ERROR) << "Precondition failed: " << ex.what();
            return Status(StatusCode::FAILED_PRECONDITION, ex.what());

        } catch (const std::exception& ex) {

            LOG_S(ERROR) << "Internal Server Error: " << ex.what();
            return Status(StatusCode::INTERNAL, "Internal Server Error, Cannot Transfer Amount");

        } catch (...) {
            // something notorious has happened if you reach here
            // stop the server
            LOG_S(FATAL) << "shutting down";
        }

        return Status::OK;
    }

   public:
    void initiaize(const std::string& mongo_uri) {
        conn_pool_ = std::make_unique<mongocxx::pool>(mongocxx::uri{mongo_uri});
    };

   private:
    std::unique_ptr<mongocxx::pool> conn_pool_;

    void logAuthData_(std::string user, std::string pin) {
        LOG_S(INFO) << "Received AuthUser: [ username: " << user << ", pin: " << pin << " ]";
    }

    std::tuple<bool, bsoncxx::stdx::optional<bsoncxx::document::value>> getAuthData_(std::string username, std::string pin, mongocxx::client& client) {
        auto db = client["zirconium"];

        bool is_authenticated = false;
        bsoncxx::stdx::optional<bsoncxx::document::value> user = db["bank"].find_one(document{} << "username" << username << finalize);

        if (user) {
            LOG_S(INFO) << "Found User from DB: " << bsoncxx::to_json(*user);
            if (pin == user->view()["pin"].get_utf8().value.to_string()) {
                is_authenticated = true;
            }
        } else {
            LOG_S(ERROR) << username << " not found !";
        }

        return std::make_tuple(is_authenticated, user);
    }
};

void RunServer(const std::string& server_addr, const std::string& mongo_uri) {
    WebServiceImpl service;

    // initialize the service with mongo db name "zirconium"
    service.initiaize(mongo_uri);

    ServerBuilder builder;

    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());

    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);

    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    LOG_S(INFO) << "Server listening on " << server_addr;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

int main(int argc, char* argv[]) {
    // turn off the printing thread info in the output
    // loguru::g_preamble_thread = false;

    // turn off the uptime in the output
    loguru::g_preamble_uptime = false;

    // instantiate the mongo connection
    mongocxx::instance instance{};  // This should be done only once.
    std::string mongo_addr("mongodb://localhost:27017/?replicaSet=rs0");

    // create a pool
    // mongocxx::pool pool{uri};

    std::string server_addr("0.0.0.0:50051");

    // run the server with the pool
    RunServer(server_addr, mongo_addr);

    return 0;
}