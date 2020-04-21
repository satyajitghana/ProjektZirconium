# binary rule to create the server executable
# depends on the grpc-proto library we made above
cc_binary(
    name = "zirconium_server",
    srcs = ["zirconium_server.cc", "transaction_mgr.hpp"],
    copts = ["-Wl,-rpath,/usr/local/lib"],
    linkopts = [
        "-lmongocxx",
        "-lbsoncxx",
        "-ldl",
        "-std=c++17",
        "-shared-libgcc",
    ],
    deps = [
        "//utils:cxxopts",
        "//utils:loguru",
        "//protos:zirconium_cpp_grpc",
        "@bsoncxx",
        "@mongocxx",
    ],
)
