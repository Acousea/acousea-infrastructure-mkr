import shutil
from pathlib import Path

Import("env", "projenv")


def stringify_targets(targets):
    """Convierte los objetos SCons en nombres legibles."""
    result = []
    for t in targets:
        try:
            # name or path if exists
            if hasattr(t, "get_path"):
                result.append(t.get_path())
            elif hasattr(t, "name"):
                result.append(str(t.name))
            else:
                result.append(str(t))
        except Exception as e:
            result.append(f"<unprintable:{type(t).__name__}> ({e})")
    return result

# Dump global construction environment (for debug purpose)
# print("*" * 30 + " Environment Dump " + "*" * 30)
# print(env.Dump())
# print("*" * 30 + " Project Environment Dump " + "*" * 30)
# print(projenv.Dump())


print("=" * 60)
print(f"Current CLI targets: {stringify_targets(COMMAND_LINE_TARGETS)}")
print(f"Current Build targets: {stringify_targets(BUILD_TARGETS)}")
print(f"Building project from: {env.subst('$PROJECT_DIR')}")
print(f"Project name: {env.subst('$PROGNAME')}")
print(f"Project version: {env.subst('$VERSION')}")
print(f"Board: {env.subst('$BOARD')}")
print(f"Build dir: {env.subst('$BUILD_DIR')}")
print(f"Framework: {env.subst('$PIOFRAMEWORK')}")
print("=" * 60)

def copy_nanopb_files(source, target, env):
    print("[COPY] Copiando archivos .pb.c y .pb.h generados por Nanopb...")

    build_dir = Path(env.subst("$BUILD_DIR"))
    nanopb_gen_src_dir = build_dir / "nanopb" / "generated-src"

    dest_dir = Path(env.subst("$PROJECT_DIR")) / "lib" / "NodeDevice" / "bindings"
    dest_dir.mkdir(parents=True, exist_ok=True)

    files_to_copy = list(nanopb_gen_src_dir.glob("*.pb.*"))

    if not files_to_copy:
        print("[WARN] No se encontraron archivos .pb.c/.pb.h para copiar.")
        env.Exit(1)

    for file in files_to_copy:
        try:
            print(f"--> Copiando {file.name} -> {dest_dir}")
            shutil.copy(file, dest_dir)
        except Exception as e:
            print(f"[ERROR] Fallo al copiar {file.name}: {e}")
            env.Exit(1)

    print("[OK] Archivos copiados correctamente.")


# Se ejecuta tras compilar el binario principal
env.AddPreAction("$BUILD_DIR/nanopb/generated-build/nodeDevice.pb.c.o", copy_nanopb_files) # For MKR1310
env.AddPreAction("$BUILD_DIR/nanopb/generated-build/nodeDevice.pb.o", copy_nanopb_files) # For Native

