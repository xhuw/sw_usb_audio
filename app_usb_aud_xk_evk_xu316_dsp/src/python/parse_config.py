import yaml
import glob
from mako.template import Template
import argparse
import os
from pathlib import Path

files = glob.glob("*.yaml")

pkg_dir = Path(__file__).parent
files = glob.glob(f"{pkg_dir}/../yaml/*.yaml")
templates_dir = f"{pkg_dir}/../templates"

def parse_arguments():
    """ Parse command line arguments """

    parser = argparse.ArgumentParser(description='Generate config struct files')
    parser.add_argument('--out-dir', '-o', type=str, default=f'{str(Path.cwd())}/output_files',
                        help="output directory for host and device files")
    args = parser.parse_args()
    return args

## Parse command line arguments
print("In parse_config.py!!\n\n")
args = parse_arguments()
print(f"out_dir = {args.out_dir}")
os.makedirs(args.out_dir, exist_ok=True)
struct_def_template = Template(filename=f'{templates_dir}/struct_def_h.mako')
struct_offset_template = Template(filename=f'{templates_dir}/command_map_offsets_c.mako')
cmd_map_template = Template(filename=f'{templates_dir}/command_map_c.mako')

cmd_map = {}

for fl in files:
    with open(fl, "r") as fd:
        data = yaml.safe_load(fd)

        struct_name=list(data["module"].keys())[0]
        includes = data["includes"] if "includes" in data else []
        defines = data["defines"] if "defines" in data else dict()
        with open(f"{args.out_dir}/{struct_name}.h", "w") as f_op:
            f_op.write(struct_def_template.render(name=struct_name, data=data["module"][struct_name],
                                                  includes=includes, defines=defines))

        cmd_map[struct_name] = data["module"][struct_name]

with open(f"{args.out_dir}/cmd_map_offset.c", "w") as f_op:
    f_op.write(struct_offset_template.render(cmd_map=cmd_map))

with open(f"{args.out_dir}/cmd_map.c", "w") as f_op:
    f_op.write(cmd_map_template.render(cmd_map=cmd_map))
