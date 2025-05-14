"""
Module for embedding data files into C++ code.
This rule generates C++ source and header files with the contents of data files embedded as byte arrays.
"""

def cc_embed(name, data_file, visibility = ["//visibility:private"]):
    src = "{}.cpp".format(name)
    hdr = "{}.hpp".format(name)
    var_name = name.replace("-", "_").replace(".", "_")

    native.genrule(
        name = "{}_codegen".format(name),
        srcs = [data_file],
        outs = [src, hdr],
        tools = ["@tsnl.cc_embed//:cc_embed_codegen"],
        cmd = "$(location @tsnl.cc_embed//:cc_embed_codegen) '$(location {})' '{}' '$(location {})' '$(location {})'".format(data_file, var_name, hdr, src),
    )
    native.cc_library(
        name = name,
        srcs = [src],
        hdrs = [hdr],
        visibility = visibility,
    )
