from build_extra.config import set_debug_mode
import os

Import('env clib_static')

local_env = env.Clone()

gperf_src = ['lexer/tokens_htable.gperf',
             'types/builtin_types_htable.gperf']
refu_src = [
    'compiler.c',
    'compiler_args.c',

    'inpfile.c',
    'inpstr.c',
    'inplocation.c',
    'inpoffset.c',
    'front_ctx.c',

    'parser/parser.c',

    'info/info.c',
    'info/msg.c',

    'lexer/lexer.c',
    'lexer/tokens.c',

    'ast/ast.c',
    'ast/block.c',
    'ast/identifier.c',
    'ast/string_literal.c',
    'ast/constant_num.c',
    'ast/type.c',
    'ast/typeclass.c',
    'ast/vardecl.c',
    'ast/function.c',
    'ast/arrayref.c',
    'ast/generics.c',
    'ast/operators.c',
    'ast/ifexpr.c',
    'ast/returnstmt.c',
    'ast/ast_utils.c',

    'analyzer/analyzer.c',
    'analyzer/symbol_table.c',
    'analyzer/analyzer_pass1.c',
    'analyzer/string_table.c',
    'analyzer/typecheck.c',

    'types/type.c',
    'types/type_function.c',
    'types/type_builtin.c',

    'ir/rir.c',
    'ir/elements.c',

    'serializer/serializer.c'
]

if local_env['PARSER_IMPLEMENTATION'] == 'RECURSIVE_DESCENT':
    refu_src += [
        'parser/recursive_descent/core.c',
        'parser/recursive_descent/identifier.c',
        'parser/recursive_descent/function.c',
        'parser/recursive_descent/generics.c',
        'parser/recursive_descent/type.c',
        'parser/recursive_descent/typeclass.c',
        'parser/recursive_descent/expression.c',
        'parser/recursive_descent/arrayref.c',
        'parser/recursive_descent/block.c',
        'parser/recursive_descent/vardecl.c',
        'parser/recursive_descent/ifexpr.c',
    ]

# add specific environment variables
local_env.Append(CPPDEFINES=[
    "RF_MODULE_STRINGS",
    "RF_MODULE_INTRUSIVE_LIST",
    "RF_MODULE_HTABLE",
    "RF_MODULE_MEMORY_POOL",
    "RF_MODULE_SYSTEM",
    "RF_MODULE_IO",
    "RF_MODULE_IO_TEXTFILE"])
local_env.Append(CPPDEFINES={
    'VERBOSE_LEVEL_DEFAULT': local_env['VERBOSE_LEVEL_DEFAULT']})
local_env.Append(CPPPATH=[os.path.abspath('include')])
local_env.Append(LIBS=[clib_static, 'pthread'])
local_env.Append(LIBPATH=local_env['CLIB_DIR'])
local_env.Append(CCFLAGS=['-Wall'])

# compile with LLVM support
linker_exec = 'gcc'
if local_env['LANG_BACKEND'] == 'LLVM':
    refu_src += [
        'backend/llvm.c',
        'backend/llvm_ast.c'
    ]
    local_env.Append(LIBS=['dl', 'z', 'ncurses'])
    local_env.ParseConfig('llvm-config --libs --cflags --ldflags core analysis'
                          ' executionengine jit interpreter native')
    linker_exec = 'g++'

# add path before the sources
refu_src = [os.path.join(os.getcwd(), "src", x) for x in refu_src]
refu_src_final = refu_src + ['src/main.c']
gperf_src = [os.path.join(os.getcwd(), "src", x) for x in gperf_src]
gperf_result = local_env.Gperf(gperf_src)

refu_obj = local_env.Object(refu_src_final)
Depends(refu_obj, gperf_result)

# for now also create the executable in debug mode
set_debug_mode(local_env, True)
refu = local_env.Program("refu", refu_obj,
                         LIBPATH=local_env['CLIB_DIR'],
                         CC=linker_exec)
local_env.Alias('refu', refu)

# -- UNIT TESTS
unit_tests_files = [
    'test_main.c',
    'testsupport.c',
    'testsupport_front.c',
    'test_input_base.c',
    'lexer/test_lexer.c',
    'lexer/testsupport_lexer.c',

    'parser/testsupport_parser.c',
    'parser/test_parser_typedesc.c',
    'parser/test_parser_generics.c',
    'parser/test_parser_function.c',
    'parser/test_parser_typeclass.c',
    'parser/test_parser_operators.c',
    'parser/test_parser_block.c',
    'parser/test_parser_ifexpr.c',

    'analyzer/testsupport_analyzer.c',
    'analyzer/test_symbol_table.c',
    'analyzer/test_typecheck.c',
    'analyzer/test_string_table.c',

    'end_to_end/test_end_to_end_basic.c',
    'end_to_end/testsupport_end_to_end.c'
]
unit_tests_files = ['test/' + s for s in unit_tests_files]
unit_tests_files.extend(refu_src)
test_env = local_env.Clone()

test_env['linker_exec'] = linker_exec
# Runs tests in debug mode
set_debug_mode(test_env, True)
test_env.Append(CHECK_EXTRA_DEFINES={
    'CLIB_TEST_HELPERS':
    "\\\"" + os.path.abspath(
        os.path.join(test_env['CLIB_DIR'], 'test', 'test_helpers.h')
    ) + "\\\""
})
lang_tests = test_env.Check(
    target="lang_tests",
    source=unit_tests_files)
Depends(lang_tests, gperf_result)

local_env.Alias('lang_tests', lang_tests)
