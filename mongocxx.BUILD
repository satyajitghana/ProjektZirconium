# https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library
# basically libmongocxx resides in /usr/local/lib
# and the header files in /usr/local/include
# the top directory is /usr/local which is defined in the WORKSPACE file
# visibility is made public so anyone can use this as a dependency
cc_library(
    name = "mongocxx",
    srcs = ["lib/libmongocxx.so"],
    hdrs = glob(["include/mongocxx/v_noabi/**"]),
    includes = ["include/mongocxx/v_noabi"],
    visibility = ["//visibility:public"],
)
