import os

Import('env clib_static')

local_env = env.Clone()

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

# add specific environment variables
local_env.Append(CPPDEFINES=[
    "RF_MODULE_STRINGS",
    "RF_MODULE_INTRUSIVE_LIST",
    "RF_MODULE_SYSTEM",
    "RF_MODULE_IO",
    "RF_MODULE_IO_TEXTFILE"])
local_env.Append(CPPDEFINES={
    'VERBOSE_LEVEL_DEFAULT': local_env['VERBOSE_LEVEL_DEFAULT']})
local_env.Append(CPPPATH=['include'])
local_env.Append(LIBS=[clib_static, 'pthread'])
local_env.Append(LIBPATH=local_env['CLIB_DIR'])

# add path before the sources
refu_src = [os.path.join(os.getcwd(), "src", x) for x in refu_src]
refu_src_final = refu_src + ['src/main.c']

refu_obj = local_env.Object(refu_src_final)
refu = local_env.Program("refu", refu_obj,
                         LIBPATH=local_env['CLIB_DIR'])
local_env.Alias('refu', refu)

# -- UNIT TESTS
unit_tests_files = [
    'test_main.c',
    'parser/testsupport_parser.c',
    'parser/test_parser_identifier.c',
]
unit_tests_files = ['test/' + s for s in unit_tests_files]
unit_tests_files.extend(refu_src)
lang_tests = local_env.Check(
    target="lang_tests",
    source=[local_env.Object(s) for s in unit_tests_files]
)
local_env.Alias('lang_tests', lang_tests)
