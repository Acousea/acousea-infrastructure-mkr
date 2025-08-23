import shutil
from pathlib import Path

Import("env")

def copy_nanopb_files(source, target, env):
    print("[COPY] Copiando archivos .pb.c y .pb.h generados por Nanopb...")

    build_dir = Path(env.subst("$BUILD_DIR"))
    nanopb_gen_src_dir = build_dir / "nanopb" / "generated-src"

    dest_dir = Path(env.subst("$PROJECT_DIR")) / "lib"  / "NodeDevice" / "generated"
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
# env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", copy_nanopb_files)
# env.AddPreAction("$BUILD_DIR/${PROGNAME}.elf", copy_nanopb_files)
env.AddPreAction("buildprog", copy_nanopb_files)
