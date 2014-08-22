import os
from build_script.build import setup_build
from build_script.cleanup import cleanup

vars = SConscript('build_script/options.py')
env = Environment(variables=vars,
                  tools=['default', 'dot'],
                  toolpath=['build_script/tools'])
SConscript('build_script/tools/dot_builder.py',
           exports='env')
setup_build(env)

# configure the environment
env = SConscript('clib/build_script/config.py', exports='env')


# cleanup files that SCons does not cleanup on its own
# TODO: Make Scons clean them itself (if possible)
if env.GetOption('clean'):
    cleanup(os.getcwd())

# Build Refu C library 
clib = SConscript('build_script/clib.py', exports='env')
env.Alias('clib', clib)

# I think this will put all object files in obj
env.VariantDir("obj", "src", duplicate=0)  # no it does not :(

refu_src = [
    'compiler_args.c',
    'info/info.c',
    'info/msg.c',
    'parser/parser.c',
    'parser/offset.c',
    'parser/string.c',
    'parser/file.c',
    'parser/function.c',
    'parser/generics.c',
    'parser/data.c',

    'parser/tokens.c',
    'ast/ast.c',
    'ast/location.c',
    'ast/identifier.c',
    'ast/datadecl.c',
    'ast/vardecl.c',
    'ast/fndecl.c',
    'ast/genrdecl.c',
]


# add path before the sources
refu_src = [os.path.join(os.getcwd(), "src", x) for x in refu_src]
refu_src_final = refu_src + ['src/main.c']
Ignore(refu_src_final, 'clib/include/Data_Structures/hashmap_shallow_generic.h')
Ignore(refu_src_final, 'clib/include/Data_Structures/hashmap_shallow_generic_decl.h')

refu_obj = env.Object(refu_src_final)
Ignore(refu_obj, 'clib/include/Data_Structures/hashmap_shallow_generic.h')
Ignore(refu_obj, 'clib/include/Data_Structures/hashmap_shallow_generic_decl.h')


refu = env.Program("refu", refu_obj,
                   LIBS=["refu", "pthread"],
                   LIBPATH=env['CLIB_DIRECTORY'])
Ignore(refu, 'clib/include/Data_Structures/hashmap_shallow_generic.h')
Ignore(refu, 'clib/include/Data_Structures/hashmap_shallow_generic_decl.h')
Depends(refu, clib)

docs = env.Dot("docs/graphs/architecture.dot", output_format="png")
docs = env.Dot("docs/graphs/ast.dot", output_format="png")


# -- UNIT TESTS
unit_tests_files = [
    'test_main.c',
    'parser/testsupport_parser.c',
    'parser/test_parser_identifier.c',
]
unit_tests_files = ['test/' + s for s in unit_tests_files]
unit_tests_files.extend(refu_src)

libs_check = ['check', 'refu', 'pthread']
cppdefines_check = env['CPPDEFINES']


# not set, so can't unset
# cppdefines_check['RF_OPTION_DEBUG'] = None
# cppdefines_check['RF_OPTION_INSANITY_CHECKS'] = None
try:
    del cppdefines_check['NDEBUG']
except:
    pass
program = env.Program(
    'check',
    # Instead of just providing source files, give objects
    # to avoid "2 different environments for same target warning
    # [env.Object(s, CPPPATH=env['CPPPATH'] + ['./']) for s in unit_tests_files],
    [env.Object(s) for s in unit_tests_files],
    LIBS=libs_check,
    CPPDEFINES=cppdefines_check,
    CPPPATH=env['CPPPATH'] + ['./'],
    LIBPATH=env['CLIB_DIRECTORY'])

refulang_test_run = env.Command(
    target="refulang_test_run",
    source='check',
    action="./check {} {}".format(
        env['UNIT_TESTS_OUTPUT'],
        env['UNIT_TESTS_FORK']))
check_refulang_alias = Alias('check', [refulang_test_run])
# Simply required.  Without it, 'check' is never considered out of date.
AlwaysBuild(check_refulang_alias)

# If we have valgrind also run the unit tests through valgrind
if env['has_valgrind']:
    valgrind_cmd = ("valgrind --tool=memcheck "
                    "--leak-check=yes "
                    "--track-origins=yes "
                    "--show-reachable=yes "
                    "--num-callers=20 "
                    "--track-fds=yes")
    refulang_test_run_val = env.Command(
        target="refulang_test_run_val",
        source='check',
        action="{} ./check {} {}".format(
            valgrind_cmd,
            env['UNIT_TESTS_OUTPUT'],
            False))  # do not fork tests when running in valgrind
    check_refulang_alias_val = Alias('check_val', [refulang_test_run_val])
    # Simply required.  Without it, 'check' is never considered out of date.
    AlwaysBuild(check_refulang_alias_val)

Default(refu)
