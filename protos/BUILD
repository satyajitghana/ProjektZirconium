# load the rules for building
load("@rules_proto_grpc//cpp:defs.bzl", "cpp_proto_compile")
load("@rules_proto_grpc//cpp:defs.bzl", "cpp_grpc_library")

# native proto_library rule
proto_library(
    name = "zirconiumbank",
    srcs = ["zirconiumbank.proto"],
    deps = ["@com_google_protobuf//:any_proto"],
)

# create the grpc library that our server and client can depend on
cpp_grpc_library(
    name = "zirconium_cpp_grpc",
    deps = [":zirconiumbank"],
    visibility = ["//visibility:public"],
)