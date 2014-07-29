import os

def cleanup(root_dir):
    try:
        os.remove(os.path.join(root_dir, "src", "bison.output"))
    except:
        pass
