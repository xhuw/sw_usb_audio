"""
Utility functions for building and running the application within 
the jupyter notebook
"""
import subprocess
import ipywidgets as widgets
from IPython import display
from pathlib import Path
import shutil
import os

APP_DIR = Path(__file__).parent

def build_and_try_run():
    """
    Attempt to build and xrun the application
    """
    build_dir = APP_DIR / "build"
    source_dir = APP_DIR
    if not (APP_DIR/"build"/"CMakeCache.txt").exists():
        # need to configure, default to Ninja because its better
        generator = "Ninja" if shutil.which("ninja") else "Unix Makefiles"
        print("Configuring...\r")
        ret = subprocess.run([*(f"cmake -S {source_dir} -B {build_dir} -G".split()), generator])
        if ret.returncode:
            print("Configuring failed, check log for details\r")
    
    print("Compiling...\r")
    nproc = os.cpu_count() or 1
    ret = subprocess.run(f"cmake --build {build_dir} -j {nproc} --target 2AMi2o2xxxxxx".split())
    if ret.returncode:
        print("Building failed, check log for details\r")
    
    print("Running...\r")
    app = APP_DIR / "bin\\2AMi2o2xxxxxx\\app_usb_aud_xk_evk_xu316_dsp_2AMi2o2xxxxxx.xe"
    ret = subprocess.run(f"xrun {app}".split())
    if ret.returncode:
        print("xrun failed, is device connected?\r")
    print("Done\r")
