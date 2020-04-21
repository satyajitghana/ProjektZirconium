#pragma once

// to use c++ style logging
// specifically define this before including loguru.hpp
#define LOGURU_WITH_STREAMS 1

#include <bsoncxx/json.hpp>
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/exception/operation_exception.hpp>

#include "utils/loguru.hpp"

namespace zirconium {

namespace transaction_mgr {

using namespace mongocxx;

using on_success_func = std::function<void(void)>;

inline auto commit_with_retry(client_session& session, on_success_func succ_fun) {
    while (true) {
        try {
            session.commit_transaction();  // Uses write concern set at transaction start.
            LOG_S(INFO) << "Transaction commited : " << bsoncxx::to_json(session.id());
            succ_fun();
            break;
        } catch (const operation_exception& oe) {
            // Can retry commit
            if (oe.has_error_label("UnknownTransactionCommitResult")) {
                LOG_S(WARNING) << "UnknownTransactionCommitResult, retrying commit operation for session: " << bsoncxx::to_json(session.id());
                continue;
            } else {
                throw oe;
            }
        }
    }
};

using transaction_func = std::function<void(client_session& session)>;

inline auto run_transaction_with_retry(transaction_func txn_func, client_session& session) {
    while (true) {
        try {
            txn_func(session);  // performs transaction.
            break;
        } catch (const operation_exception& oe) {
            LOG_S(ERROR) << "Transaction aborted. Caught exception during transaction: " << oe.what() << ", for session: " << bsoncxx::to_json(session.id());
            // If transient error, retry the whole transaction.
            if (oe.has_error_label("TransientTransactionError")) {
                LOG_S(WARNING) << "TransientTransactionError, retrying transaction for session: " << bsoncxx::to_json(session.id());
                continue;
            } else {
                throw oe;
            }
        }
    }
};

inline auto get_transaction_opts() {
    mongocxx::options::transaction txn_opts;

    // read and write concerns and read preference
    mongocxx::write_concern wc_majority{};
    wc_majority.acknowledge_level(mongocxx::write_concern::level::k_majority);

    mongocxx::read_concern rc_local{};
    rc_local.acknowledge_level(mongocxx::read_concern::level::k_local);

    mongocxx::read_preference rp_primary{};
    rp_primary.mode(mongocxx::read_preference::read_mode::k_primary);

    txn_opts.write_concern(wc_majority);
    txn_opts.read_concern(rc_local);
    txn_opts.read_preference(rp_primary);

    return txn_opts;
};

}  // namespace transaction_mgr

class exception : public std::exception {
   public:
    exception(const std::string& message) : msg_(message) {}

    virtual const char* what() const throw() { return msg_.c_str(); }

   private:
    std::string msg_;
};

}  // namespace zirconium