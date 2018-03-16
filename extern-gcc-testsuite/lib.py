import os
import subprocess

def run(cmd):
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    return proc.stdout.read()

def find_prog(prog):
    if prog is None:
        return None
    if os.path.sep in prog:
        if os.path.exists(prog):
            return prog
        else:
            return None
    else:
        paths = os.environ['PATH'].split(':')
        for path in paths:
            p = os.path.join(path, prog)
            if os.path.exists(p):
                return p
        return None

def gen_makefile(makefile_in, makefile_out, replace_vars):
    with open(makefile_in, 'r') as file :
        filedata = file.read()

    # Replace the target string
    for key, val in replace_vars.iteritems():
        filedata = filedata.replace('@%s@' % key, val)

    # Write the file out again
    with open(makefile_out, 'w') as file:
        file.write(filedata)

def gen_site_exp(target, target_cc, target_cxx, target_sim,
                 host, host_cc, test_dir, test_source_dir,
                 extra_clags, is_clang, exp):

    if is_clang:
        extra_clags = \
            "-Wno-ignored-optimization-argument -Wno-unknown-warning-option " + \
            "-Wno-return-type -Wno-implicit-function-declaration " + \
            "-Wno-unknown-attributes -Qunused-arguments " + \
            "-Wno-ignored-attributes -Wno-keyword-macro -Wno-format " + \
            "-Wno-shift-negative-value -Wno-tautological-constant-out-of-range-compare " + \
            "-Wno-parentheses -Wno-incompatible-library-redeclaration " + \
            "-Wno-unused-value -Wno-switch -Wno-int-conversion " + \
            "-Wno-tautological-pointer-compare -Wno-pointer-sign " + \
            "-Wno-non-literal-null-conversion -Wno-unsequenced " + \
            "-Wno-builtin-requires-header -Wno-empty-body -Wno-array-bounds " + \
            "-Wno-main -Wno-tautological-compare -Wno-comment " + \
            "-Wno-gnu-designator -Wno-gnu-empty-struct " + \
            "-Wno-gnu-folding-constant -Wno-microsoft-anon-tag " + \
            "-Wno-incompatible-pointer-types-discards-qualifiers " + \
            "-Wno-absolute-value -Wno-sizeof-array-decay -Wno-pedantic " + \
            "-Wno-shift-overflow -ferror-limit=1000 " + \
            "-Wno-pointer-bool-conversion -Wno-literal-conversion " + \
            "-Wno-gnu-complex-integer -Wno-duplicate-decl-specifier " + \
            "-Wno-string-compare -Wno-initializer-overrides " + \
            "-Wno-missing-declarations -Wno-knr-promoted-parameter " + \
            "-Wno-pointer-arith -Wno-bitfield-constant-conversion " + \
            "-Wno-gcc-compat -Wno-constant-conversion " + \
            "-Wno-gnu-empty-initializer -Wno-gnu-alignof-expression " + \
            "-Wno-sync-fetch-and-nand-semantics-changed -Wno-visibility " + \
            "-Wno-main-return-type -Wno-int-to-pointer-cast" + \
            extra_clags
    site_config = dict()
    site_config["rootme"] = test_dir
    site_config["host_triplet"] = host
    site_config["build_triplet"] = host
    site_config["target_triplet"] = target
    site_config["target_alias"] = target
    site_config["libiconv"] = ""
    site_config["CFLAGS"] = ""
    site_config["CXXFLAGS"] = ""
    site_config["HOSTCC"] = host_cc
    site_config["HOSTCFLAGS"] = ""
    site_config["TEST_ALWAYS_FLAGS"] = extra_clags
    site_config["TESTING_IN_BUILD_TREE"] = "0"
    site_config["HAVE_LIBSTDCXX_V3"] = "1"
    site_config["ISLVER"] = ""
    site_config["tmpdir"] = test_dir
    site_config["srcdir"] =  test_source_dir
    site_config["GCC_UNDER_TEST"] = target_cc
    site_config["GXX_UNDER_TEST"] = target_cxx
    if target_sim is not None:
        site_config["SIM"] = target_sim


    def gen_site_exp(exp_path):
       with open(exp_path, "w") as exp:
         for config_name, config in site_config.iteritems():
           exp.write("set %s \"%s\"\n" % (config_name, config))

    gen_site_exp(exp)
