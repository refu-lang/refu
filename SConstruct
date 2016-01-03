from build_extra.config import set_debug_mode, remove_envvar_values
from build_extra.utils import build_msg
import os

Import('env clib_static')

local_env = env.Clone()

gperf_src = [
    'lexer/tokens_htable.gperf',
    'types/elementary_types_htable.gperf',
    'ir/parser/rirtoken_htable.gperf'
]
refu_src = [
    'compiler.c',
    'compiler_args.c',

    'module.c',
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
    'ast/constants.c',
    'ast/type.c',
    'ast/typeclass.c',
    'ast/vardecl.c',
    'ast/function.c',
    'ast/generics.c',
    'ast/operators.c',
    'ast/ifexpr.c',
    'ast/matchexpr.c',
    'ast/returnstmt.c',
    'ast/module.c',
    'ast/ast_utils.c',
    'ast/ast_type_traversal.c',

    'utils/traversal.c',
    'utils/string_set.c',
    'utils/common_strings.c',
    'utils/data.c',

    'analyzer/analyzer.c',
    'analyzer/symbol_table.c',
    'analyzer/analyzer_pass1.c',
    'analyzer/typecheck.c',
    'analyzer/type_set.c',
    'analyzer/typecheck_matchexpr.c',

    'types/type.c',
    'types/type_comparisons.c',
    'types/type_function.c',
    'types/type_operators.c',
    'types/type_elementary.c',
    'types/type_utils.c',

    'ir/rir.c',
    'ir/rir_utils.c',
    'ir/rir_object.c',
    'ir/rir_argument.c',
    'ir/rir_function.c',
    'ir/rir_call.c',
    'ir/rir_block.c',
    'ir/rir_binaryop.c',
    'ir/rir_unaryop.c',
    'ir/rir_constant.c',
    'ir/rir_convert.c',
    'ir/rir_branch.c',
    'ir/rir_strmap.c',
    'ir/rir_expression.c',
    'ir/rir_value.c',
    'ir/rir_variable.c',
    'ir/rir_global.c',
    'ir/rir_type.c',
    'ir/rir_typedef.c',
    'ir/rir_process.c',
    'ir/rir_process_cond.c',
    'ir/rir_process_match.c',

    'ir/parser/rirparser.c',
    'ir/parser/rirtoken.c',
    'ir/parser/rparse_global.c',
    'ir/parser/rparse_typedef.c',
    'ir/parser/rparse_fndef.c',

    'ownership/ownership.c',
    'ownership/ow_graph.c',
    'ownership/ow_node.c',
    'ownership/ow_edge.c',

    'serializer/serializer.c',
    'serializer/astprinter.c',
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
        'parser/recursive_descent/block.c',
        'parser/recursive_descent/vardecl.c',
        'parser/recursive_descent/ifexpr.c',
        'parser/recursive_descent/matchexpr.c',
        'parser/recursive_descent/module.c',
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
    'RF_LANG_MAJOR_VERSION': 0,
    'RF_LANG_MINOR_VERSION': 0,
    'RF_LANG_PATCH_VERSION': 1,
    'VERBOSE_LEVEL_DEFAULT': local_env['VERBOSE_LEVEL_DEFAULT'],
    'INFO_CTX_BUFF_INITIAL_SIZE': local_env['INFO_CTX_BUFF_INITIAL_SIZE'],
    'INPUT_FILE_BUFF_INITIAL_SIZE': local_env['INPUT_FILE_BUFF_INITIAL_SIZE'],
    'INPUT_STRING_STARTING_LINES': local_env['INPUT_STRING_STARTING_LINES'],
    'RF_CLIB_ROOT': "\\\"" + local_env['CLIB_DIR'] + "\\\"",
    'RF_LANG_CORE_ROOT': "\\\"" + local_env['LANG_DIR'] + "\\\"",
})
local_env.Append(CPPPATH=[os.path.abspath('include')])
local_env.Append(CPPPATH=[os.path.abspath('lib')])
local_env.Append(LIBS=[clib_static, 'pthread'])
local_env.Append(LIBPATH=local_env['CLIB_DIR'])

# compile with LLVM support
linker_exec = 'gcc'
if local_env['LANG_BACKEND'] == 'LLVM':
    refu_src += [
        'backend/llvm.c',
        'backend/llvm_ast.c',
        'backend/llvm_utils.c',
        'backend/llvm_globals.c',
        'backend/llvm_operators.c',
        'backend/llvm_conversion.c',
        'backend/llvm_functions.c',
        'backend/llvm_types.c',
        'backend/llvm_values.c',
    ]
    local_env.ParseConfig('llvm-config --libs --cflags --ldflags core analysis'
                          ' executionengine interpreter native linker')
    # llvm-config adds some flags we don't need so remove them
    remove_envvar_values(local_env, 'CCFLAGS', ['-pedantic', '-Wwrite-strings'])
    local_env.Append(LIBS=['dl', 'z', 'ncurses'])
    linker_exec = env['CXX']

gv_object = []
if local_env['WITH_GRAPHVIZ']:
    if local_env['has_graphviz']:
        local_env.Append(CPPDEFINES={'RF_WITH_GRAPHVIZ': None})
        local_env.Append(LIBS=['gvc', 'cgraph', 'cdt'])
        gv_env = local_env.Clone()
        set_debug_mode(gv_env, True)
        gv_src = ['ownership/ow_graphviz.c']
        gv_src = [os.path.join(os.getcwd(), 'src', x) for x in gv_src]
        gv_object = gv_env.Object(gv_src)
    else:
        build_msg(
            "Requested building with graphviz but graphviz was not found",
            "Warning",
            local_env
        )


# add src prefix before the sources that reside at src/
refu_src = [os.path.join(os.getcwd(), "src", x) for x in refu_src]
# external library sources
refu_external_src = [
    'lib/argtable/argtable3.c'
    # argtable can go here when they fix the problem with Non Ansi-C
    # compilation. Until then it needs to be compiled into separate object file
]

# add up all the sources
refu_src_final = refu_src + refu_external_src + ['src/main.c']
gperf_src = [os.path.join(os.getcwd(), "src", x) for x in gperf_src]
gperf_result = local_env.Gperf(gperf_src)

refu_obj = local_env.Object(refu_src_final)
Depends(refu_obj, gperf_result)

# for now also create the executable in debug mode
set_debug_mode(local_env, True)
refu = local_env.Program("refu", [refu_obj] + gv_object,
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
    'parser/test_parser_misc.c',
    'parser/test_parser_matchexpr.c',
    'parser/test_parser_modules.c',

    'analyzer/testsupport_analyzer.c',
    'analyzer/test_symbol_table.c',
    'analyzer/test_typecheck.c',
    'analyzer/test_typecheck_conversion.c',
    'analyzer/test_typecheck_functions.c',
    'analyzer/test_typecheck_matchexpr.c',
    'analyzer/test_typecheck_operators.c',
    'analyzer/test_modules.c',

    'types/test_types.c',
    'types/test_typeset.c',

    'rir/testsupport_rir.c',
    'rir/test_finalized_ast.c',
    'rir/test_ownership.c',
    'rir/creation/test_create_simple.c',

    'end_to_end/testsupport_end_to_end.c',
    'end_to_end/test_end_to_end_basic.c',
    'end_to_end/test_end_to_end_modules.c',
]
unit_tests_files = ['test/' + s for s in unit_tests_files]
unit_tests_files.extend(refu_src)
unit_tests_files.extend(refu_external_src)
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
