# load the rules for building
load("@rules_proto_grpc//cpp:defs.bzl", "cpp_proto_compile")
load("@rules_proto_grpc//cpp:defs.bzl", "cpp_grpc_library")

# native proto_library rule
proto_library(
    name = "helloworld",
    srcs = ["helloworld.proto"],
    deps = ["@com_google_protobuf//:any_proto"],
)

cpp_proto_compile(
    name = "cpp_helloworld_proto",
    deps = [":helloworld"],
)

# compile server and client
cpp_grpc_library(
    name = "greeter_cpp_grpc",
    deps = [":helloworld"],
)

cc_binary(
    name = "hello_server",
    srcs = ["greeter_server.cc"],
    copts = ["-Wl,-rpath,/usr/local/lib"],
    linkopts = [
        "-lmongocxx",
        "-lbsoncxx",
    ],
    deps = [
        ":greeter_cpp_grpc",
        "@bsoncxx",
        "@mongocxx",
    ],
)

# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary
cc_binary(
    name = "hello_client",
    srcs = ["greeter_client.cc"],
    copts = ["-Wl,-rpath,/usr/local/lib"],
    linkopts = [
        "-lmongocxx",
        "-lbsoncxx",
    ],
    deps = [
        ":greeter_cpp_grpc",
        "@bsoncxx",
        "@mongocxx",
    ],
)
