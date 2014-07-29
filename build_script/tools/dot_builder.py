Import('env')

def dot_builder_gen(source, target, env, for_signature):
    """ Generates the dot build command """
    output_format = "-Tsvg" if env.get('output_format') == None else "-T" + env.get('output_format')
    for s, t in zip(source, target):
        return "dot {} {} -o {} ".format(str(s), output_format, str(t))

def dot_gen_suffix(env, sources):
    """ Generates the suffix that the dot target must have """
    if env.get('output_format') == "svg":
        return ".svg"
    elif env.get('output_format') == "png":
        return ".png"
    elif env.get('output_format') == None:
        return ".svg"
    else:
        print "Invalid dot output format provided. Quitting ..."
        Exit(-1)

# add the builder to the environment
dot_builder = Builder(generator=dot_builder_gen, suffix=dot_gen_suffix,
                      ensure_suffix=True)
env.Append(BUILDERS = {'Dot':dot_builder})


