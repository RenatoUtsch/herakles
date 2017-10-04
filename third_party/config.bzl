# MIT License
#
# Copyright (c) 2017 Vladimir Antonov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Taken from https://github.com/antonovvk/bazel_rules/blob/master/config.bzl
# Modified a little bit.

def _fix_config_impl(ctx):
    input = ctx.file.file
    output = ctx.outputs.out

    script = ""
    for k, v in ctx.attr.values.items():
        v = v.replace('\\', '\\\\').replace('/', '\\/')
        if ctx.attr.cmake:
            script += r"s/\#cmakedefine\s+%s\b.*/\#define %s %s/g;" % (k, k, v)
            script += r"s/\$\{%s\}/%s/g;" % (k, v)
        script += r"s/\@%s\@/%s/g;" % (k, v)

    if ctx.attr.cmake:
        script += r"s/\#cmakedefine[\s]+(\w+).*/\/* #undef \1 *\//g;"
        script += r"s/\$\{\w+\}//g;"
    script += r"s/\@[^\@]*\@/0/g"

    ctx.action(
        inputs = [input],
        outputs = [output],
        progress_message = "Configuring %s" % input.short_path,
        command = "perl -pe '%s' < %s > %s" % (script, input.path, output.path)
    )

fix_config_rule = rule(
    attrs = {
        "file": attr.label(
            mandatory = True,
            allow_files = True,
            single_file = True,
        ),
        "cmake": attr.bool(
            default = False,
            mandatory = False,
        ),
        "output": attr.string(mandatory = True),
        "values": attr.string_dict(mandatory = True),
    },
    output_to_genfiles = True,
    outputs = {"out": "%{output}"},
    implementation = _fix_config_impl,
)

def cc_fix_config(name, files, values, cmake=False, visibility=None, includes=None):
    srcs = []
    for input, output in files.items():
        fix_config_rule(
            name = input + "_impl",
            file = input,
            cmake = cmake,
            output = output,
            values = values,
        )
        srcs.append(output)

    native.filegroup(
        name = name,
        srcs = srcs,
    )
