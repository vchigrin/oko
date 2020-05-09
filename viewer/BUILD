cc_binary(
    name = "oko",
    srcs = [
        "log_file.h",
        "log_record.h",
        "log_view.h",
        "log_window.cc",
        "log_window.h",
        "main.cc",
        "memorylog_log_file.cc",
        "memorylog_log_file.h",
        "ncurses_helpers.h",
        "status_window.cc",
        "status_window.h",
    ],
    deps = [
        "@boost//:iostreams",
        "@boost//:program_options",
        "@libcxx",
        "@ncurses",
    ],
)
