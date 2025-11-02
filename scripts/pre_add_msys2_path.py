Import("env")

import os
import shutil
from platform import system


print("=" * 80)
print("[PlatformIO pre-script] Checking MSYS2 paths before build...")
print("Current CLI targets", COMMAND_LINE_TARGETS)
print("Current Build targets", BUILD_TARGETS)
def add_msys2_paths():
    """Ensure MSYS2 MinGW64 runtime DLLs are found when running tests."""
    if not system().lower().startswith("win"):
        print("[PlatformIO pre-script] Non-Windows detected, skipping MSYS2 setup.")
        return
    else:
        print("[PlatformIO pre-script] Windows detected, setting up MSYS2 paths.")

    msys_root = r"D:\admin\MSYS2"  # <-- ajusta si lo tienes en otra ruta
    msys_bin = os.path.join(msys_root, "mingw64", "bin")
    usr_bin = os.path.join(msys_root, "usr", "bin")
    ucrt_bin = os.path.join(msys_root, "ucrt64", "bin")

    paths = [p for p in [msys_bin, usr_bin, ucrt_bin] if os.path.isdir(p)]
    os.environ["PATH"] = os.pathsep.join(paths + [os.environ.get("PATH", "")])

    print("[PlatformIO pre-script] Added MSYS2 paths to PATH:")
    for p in paths:
        print(f"  -> {p}")
        # Opcional: verificación rápida

    if not shutil.which("gcc"):
        print("[WARNING] gcc not found in PATH after MSYS2 addition!")
    else:
        print(f"[OK] gcc found at: {shutil.which('gcc')}")
    if not shutil.which("g++"):
        print("[WARNING] g++ not found in PATH after MSYS2 addition!")
    else:
        print(f"[OK] g++ found at: {shutil.which('g++')}")

def copy_msys2_dlls():
    """Optional fallback: copy MSYS2 DLLs to build dir if PATH fails."""
    if not system().lower().startswith("win"):
        return
    build_dir = env.subst("$BUILD_DIR")
    dlls = ["libstdc++-6.dll", "libgcc_s_seh-1.dll", "libwinpthread-1.dll"]
    msys_bin = r"D:\admin\MSYS2\mingw64\bin"
    for dll in dlls:
        src = os.path.join(msys_bin, dll)
        if os.path.exists(src):
            shutil.copy(src, build_dir)
            print(f"[OK] Copied {dll} to {build_dir}")


add_msys2_paths()
copy_msys2_dlls()

print("=" * 80)