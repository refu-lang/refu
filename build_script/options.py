# Initialize the variables object
vars = Variables()

vars.Add(
    ListVariable('BACKENDS',
                 'The backends to compile the refu compiler with',
                 'all',
                 [
                     'GCC',
                     'LLVM',
                     'INTERPRETER'
                 ]
             )
)


vars.Add(
    BoolVariable('DEBUG',
                 'Controls if it\'s a debug build or not',
                 'yes'
))

vars.Add(
    BoolVariable('DEBUG_BISON',
                 'If this option is on then the bison parser will be'
                 'quite very verbose stating each and every one of '
                 'its actions.'
                 'Use only for debugging',
                 'no'
))

vars.Add(
    BoolVariable('DEBUG_FLEX',
                 'If this option is on then the lexer will be'
                 'quite very verbose stating each and every one of '
                 'its actions.'
                 'Use only for debugging',
                 'no'
))

vars.Add(
    PathVariable('CLIB_DIRECTORY',
                 'The directory of Refu C library',
                 'clib',
                 PathVariable.PathIsDir
))

vars.Add(
    BoolVariable('BISON_PARSER',
                 'This denotes I am using bison to generate the parser'
                 'for refu lang. If this is off then the hand made '
                 'parser will be used. Is work in progress',
                 'no'
))

vars.Add(
    ListVariable('BISON_REPORT',
                 'If not empty bison will also produce a .output file which '
                 'will contain a report concering the parser. Possible values '
                 'are a list of comma separated words that can include:\n'
                 'state\n'
                 '\tdescribe the states\n'
                 'itemset\n'
                 '\tcomplete the core item sets with their closure\n'
                 'lookahead\n'
                 '\texplicitly associate lookahead tokens to items\n'
                 'solved\n'
                 '\tdescribe shift/reduce conflicts solving\n'
                 'all\n'
                 '\tinclude all the above information\n'
                 'none\n'
                 '\tdisable the report',
                 [],
                 ['state', 'itemset', 'lookahead', 'solved', 'all', 'none']
))

vars.Add(
    'SYMBOL_TABLE_INITIAL_SIZE',
    'The initial capacity of the hash tables of all blocks in symbols',
    256
)

vars.Add(
    'REPL_BUFFER_SIZE',
    'The initial size of the Read Eval Print Loop buffer',
    512
)

vars.Add(
    'PARSER_BUFFER_SIZE',
    'The initial size of the buffer for a parsing context',
    4096
)

vars.Add(
    EnumVariable(
        'VERBOSE_LEVEL_DEFAULT',
        'The default verbosity level. Should range between 1 and 4',
        '1',
        allowed_values=('1', '2', '3', '4')
))

vars.Add(
    'PARSER_STACK_MAX_DEPTH',
    'The maximum depth that the parser can search in. That is the number of'
    'imports that can be followed while parsing source',
    64
)


vars.Add(
    BoolVariable('HAS_INTERPRETER',
                 'This denotes that the compiler has been compiled with '
                 'interpreting capabilities',
                 'yes'
))    

Return('vars')
