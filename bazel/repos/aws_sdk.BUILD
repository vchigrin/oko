cc_library(
    name = "common",
    srcs = ["lib/libaws-cpp-sdk-core.so"],
    hdrs = glob(["include/aws/**"]),
    strip_include_prefix = "include",
)

cc_library(
    name = "s3",
    srcs = ["lib/libaws-cpp-sdk-s3.so"],
    visibility = ["//visibility:public"],
    deps = [":common"],
)
