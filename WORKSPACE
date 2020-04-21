# add rules_proto_grpc to this WORKSPACE
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_proto_grpc",
    sha256 = "497225bb586e8f587e139c55b0f015e93bdddfd81902985ce24623528dbe31ab",
    strip_prefix = "rules_proto_grpc-1.0.1",
    urls = ["https://github.com/rules-proto-grpc/rules_proto_grpc/archive/1.0.1.tar.gz"],
)

load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_repos", "rules_proto_grpc_toolchains")

rules_proto_grpc_toolchains()

rules_proto_grpc_repos()

# end add rules_proto_grpc

# rules_proto_grpc
load("@rules_proto_grpc//cpp:repositories.bzl", rules_proto_grpc_cpp_repos = "cpp_repos")

rules_proto_grpc_cpp_repos()

# grpc_deps
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

# mongo and bson libraries

# new_local_repository is used to create a local non bazel repository, so you can use the .so
# files generated in that folder and add it as a dependency to your build

new_local_repository(
    name = "bsoncxx",
    build_file = "gsoncxx.BUILD",
    path = "/usr/local",
)

new_local_repository(
    name = "mongocxx",
    build_file = "mongocxx.BUILD",
    path = "/usr/local",
)
