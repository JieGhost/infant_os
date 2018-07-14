cc_library(
  name = "stdlib",
  srcs = ["lib.c"],
  hdrs = [
    "lib.h",
    "types.h",
  ],
  copts = [
    "-m32",
    "-nostdlib",
    "-nostdinc",
    "-fno-builtin",
    "-fno-stack-protector",
    # "-nostartfiles",
  ],
)
