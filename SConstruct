import os
from build_script.build import setup_build
from build_script.cleanup import cleanup


vars = SConscript('build_script/options.py')
env = Environment(variables=vars, tools=['default', 'dot'], toolpath = ['build_script/tools'])
SConscript('build_script/tools/dot_builder.py', exports='env')
setup_build(env)


# cleanup files that SCons does not cleanup on its own
# TODO: Make Scons clean them itself (if possible)
if env.GetOption('clean'):
    cleanup(os.getcwd())

# Build Refu C library 
clib = SConscript('build_script/clib.py', exports='env')
env.Alias('clib', clib)

# I think this will put all object files in obj
env.VariantDir("obj", "src", duplicate=0) # no it does not :(
cld_sources = [os.path.join(dirpath, f)
               for dirpath, dirnames, files in os.walk(
                       os.path.join('.', "src")
               )
               for f in files if f.endswith(".c") and not f == "bison.c"
               and not f == "flex.c"]

cld_sources = [
    'main.c',
    'messaging.c',
    'argparser.c',
    'parser/parser.c',
    'parser/offset.c',
    'ast.c'
]


# add path before the sources
cld_sources = [os.path.join(os.getcwd(), "src", x) for x in cld_sources]
Ignore(cld_sources, 'clib/include/Data_Structures/hashmap_shallow_generic.h')
Ignore(cld_sources, 'clib/include/Data_Structures/hashmap_shallow_generic_decl.h')

cld_objects = env.Object(cld_sources)
Ignore(cld_objects, 'clib/include/Data_Structures/hashmap_shallow_generic.h')
Ignore(cld_objects, 'clib/include/Data_Structures/hashmap_shallow_generic_decl.h')


refu = env.Program("refu", cld_objects,
                   LIBS=["refu", "pthread"],
                   LIBPATH=env['CLIB_DIRECTORY'])
Ignore(refu, 'clib/include/Data_Structures/hashmap_shallow_generic.h')
Ignore(refu, 'clib/include/Data_Structures/hashmap_shallow_generic_decl.h')
Depends(refu, clib)

docs = env.Dot("docs/graphs/architecture.dot", output_format="png")
docs = env.Dot("docs/graphs/ast.dot", output_format="png")


Default(refu)
