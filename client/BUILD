# binary rule to create the clien exeecutable
# depends on the grpc-proto library we made
cc_binary(
    name = "zirconium_client",
    srcs = ["zirconium_client.cc"],
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
        "//utils:cxxopts",
        "//utils:loguru",
        "//utils:zirconium_utils",
        "//protos:zirconium_cpp_grpc",
        "@bsoncxx",
        "@mongocxx",
    ],
)