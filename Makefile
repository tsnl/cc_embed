# NOTE: We use Bazel as the build system. This Makefile is just a wrapper for good tab completion in the shell.

# cpp_base configuration: do not edit this section.
MAKEFLAGS+=--always-make 	# we use bazel as the build system, so we want to always build. Equal to '.PHONY: *'.
SHELL=/bin/bash
.SHELLFLAGS="-O extglob -c"

# repo configuration:
# ...

#
# cpp_base targets: do not edit this section.
#

__default__: default

compile_commands.json:
	bazelisk run refresh_compile_commands

clean:
	bazelisk clean --expunge

format:
	bash "script/clang_format.sh"

#
# targets:
#

default: cc_embed_codegen test

cc_embed_codegen: 
	bazelisk build cc_embed_codegen

test:
	bazelisk test test --cache_test_results=no
