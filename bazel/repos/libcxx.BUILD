cc_library(
    name = "libcxx",
    srcs = glob(["lib/llvm-9/lib/libc++.so*"]),
    hdrs = glob([
        "lib/llvm-9/include/c++/v1/**",
    ]),
    includes = ["lib/llvm-9/include/c++/v1/"],
    visibility = ["//visibility:public"],
)
