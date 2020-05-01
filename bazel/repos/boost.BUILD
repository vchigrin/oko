cc_library(
    name = "headers",
    hdrs = glob(["include/boost/**"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "program_options",
    srcs = glob([
        "lib/libboost_program_options.so*",
    ]),
    visibility = ["//visibility:public"],
    deps = [":headers"],
)
