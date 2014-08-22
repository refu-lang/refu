import os


options_list = ['SYMBOL_TABLE_INITIAL_SIZE',
                'REPL_BUFFER_SIZE',
                'VERBOSE_LEVEL_DEFAULT',
                'PARSER_STACK_MAX_DEPTH',
                'PARSER_BUFFER_SIZE']

bool_options_list = ['HAS_INTERPRETER']

def setup_includes(env):
    env['CLIB_DIRECTORY'] = os.path.join(os.getcwd(), env['CLIB_DIRECTORY'])
    env['REFU_ROOT'] = os.path.abspath(os.getcwd())
    CLIB_INC = os.path.join(env['CLIB_DIRECTORY'], "include")
    REFU_INC = "include"
    env.Append(CPPPATH=[CLIB_INC, REFU_INC])

def setup_defines(env):
    cld_defs = ["-D_FILE_OFFSET_BITS=64",
                "-D_GNU_SOURCE",
                "-D_LARGEFILE64_SOURCE"]
    env.Append(CPPDEFINES=cld_defs)
    if env['DEBUG']:
        env.Append(CPPDEFINES="IS_DEBUG")
        env.Append(CPPDEFINES="RF_OPTION_DEBUG")

    for b in env['BACKENDS']:
        env.Append(CPPDEFINES="HAVE_{}_BACKEND".format(b))

    # temporary maybe,
    # define the load factor. Should be hardcoded into the generated .c file
    # by the template to source generator
    env.Append(CPPDEFINES={'RF_OPTION_HASHMAP_LOAD_FACTOR':'0.7'})

    for option in options_list:
        env.Append(CPPDEFINES={option: env[option]})

    for b in bool_options_list:
        if env[b]:
            env.Append(CPPDEFINES=b)

def setup_compiler_options(env):
    if env['DEBUG']:
        env.Append(CCFLAGS="-g")
    env.Append(CCFLAGS="-std=gnu99") # c99 + gnu extensions

def setup_build(env):
    """
    Sets up all of the required things for the build
    """
    setup_compiler_options(env)
    setup_defines(env)
    setup_includes(env)

