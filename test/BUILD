cc_test(
    name = "zirconium_client_test",
    srcs = glob(["test_clients.cc", "zirconium_client.hpp"]),
    copts = [
        "-Wl,-rpath,/usr/local/lib",
    ],
    linkopts = [
        "-lmongocxx",
        "-lbsoncxx",
        "-ldl",
        "-std=c++17",
    ],
    deps = [
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "//utils:loguru",
        "//utils:zirconium_utils",
        "//protos:zirconium_cpp_grpc",
        "@bsoncxx",
        "@mongocxx",
        ],
)