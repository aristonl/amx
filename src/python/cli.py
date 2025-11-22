#
# Copyright 2025 Aris Lorenzo. All rights reserved
#

import argparse
import os
import subprocess
import sys
from io_cbridge import CORE_BIN

def main() -> None:
    if len(sys.argv) == 1:
        # TODO: this will be handled in python later, but since there is no real
        # python side to anything, this will do for now...
        os.execvp(CORE_BIN, [CORE_BIN] + sys.argv[1:])

    parser = argparse.ArgumentParser(
        prog="amx"
    )
