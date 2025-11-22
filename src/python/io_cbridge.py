#
# Copyright 2025 Aris Lorenzo. All rights reserved.
#

import subprocess
import shutil
from typing import List

CORE_BIN = shutil.which("amx-core")

class CoreError(RuntimeError):
    pass

def run_core(
        subcommand: str,
        args: List[str],
        verbose: bool = False,
) -> None:
    cmd = [CORE_BIN]
    if verbose:
        cmd.append("-v")
    cmd.append(subcommand)
    cmd.extend(args)

    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        raise CoreError(f"amx-core failed with code {e.returncode}") from e

