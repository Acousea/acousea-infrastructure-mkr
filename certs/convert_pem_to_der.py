import os
import subprocess
import argparse

def convert_all_pem_der_to_separate_files(input_folder, output_header, output_cpp):
    anchors = []

    # Generar header (.h)
    with open(output_header, "w") as hfile:
        hfile.write("// Auto-generated trust anchors header\n")
        hfile.write("#pragma once\n\n")

        # Estructura
        hfile.write("struct TrustAnchor {\n")
        hfile.write("    const char* name;\n")
        hfile.write("    const unsigned char* data;\n")
        hfile.write("    const int size;\n")
        hfile.write("};\n\n")

        print(f"[+] Procesando certificados en {input_folder}")

        # Primero recopilar información de todos los archivos
        for file in sorted(os.listdir(input_folder)):
            if not (file.endswith(".pem") or file.endswith(".der")):
                continue

            base_name = os.path.splitext(file)[0]
            safe_name = base_name.replace("-", "_").replace(" ", "_")
            array_name = safe_name + "_der"
            array_len = array_name + "_len"
            anchors.append((base_name, safe_name, array_name, array_len))

        # Escribir declaraciones extern en el header
        for base_name, safe_name, array_name, array_len in anchors:
            hfile.write(f"// Certificado: {base_name}\n")
            hfile.write(f"extern const unsigned char {array_name}[];\n")
            hfile.write(f"extern const unsigned int {array_len};\n\n")

        # Declarar el array principal y su tamaño
        hfile.write("extern const TrustAnchor trust_anchors[];\n")
        hfile.write("extern const unsigned int trust_anchors_num;\n")

    # Generar implementación (.cpp)
    with open(output_cpp, "w") as cppfile:
        header_name = os.path.basename(output_header)
        cppfile.write("// Auto-generated trust anchors implementation\n")
        cppfile.write(f'#include "{header_name}"\n\n')

        # Procesar cada certificado y generar las definiciones
        for base_name, safe_name, array_name, array_len in anchors:
            pem_path = os.path.join(input_folder, f"{base_name}.pem")
            der_path = os.path.join(input_folder, f"{base_name}.der")

            # Determinar qué archivo usar
            if os.path.exists(pem_path):
                input_file = pem_path
                needs_conversion = True
            elif os.path.exists(der_path):
                input_file = der_path
                needs_conversion = False
            else:
                print(f"[!] No se encontró {base_name}.pem ni {base_name}.der")
                continue

            # Convertir si es PEM
            if needs_conversion:
                print(f"[+] Convirtiendo {input_file} → {der_path}")
                subprocess.run(
                    ["openssl", "x509", "-in", input_file, "-outform", "der", "-out", der_path],
                    check=True
                )
                input_file = der_path
            else:
                print(f"[+] Usando DER existente {input_file}")

            # Ejecutar xxd para generar el array
            xxd_out = subprocess.run(
                ["xxd", "-i", "-n", array_name, input_file],
                check=True, capture_output=True, text=True
            )

            # Escribir en el archivo cpp (sin inline, pero con const)
            fixed = []
            for line in xxd_out.stdout.splitlines():
                if line.strip().startswith("unsigned char"):
                    # Cambiar a const
                    fixed.append(line.replace("unsigned char", "const unsigned char"))
                elif line.strip().startswith("unsigned int"):
                    # Cambiar a const
                    fixed.append(line.replace("unsigned int", "const unsigned int"))
                else:
                    fixed.append(line)

            cppfile.write(f"// Certificado: {base_name}\n")
            cppfile.write("\n".join(fixed))
            cppfile.write("\n\n")

        # Generar el array principal
        cppfile.write("const TrustAnchor trust_anchors[] = {\n")
        for base_name, safe_name, array_name, array_len in anchors:
            cppfile.write(f'    {{"{base_name}", {array_name}, static_cast<int>({array_len})}},\n')
        cppfile.write("};\n\n")
        cppfile.write(f"const unsigned int trust_anchors_num = {len(anchors)};\n")

    print(f"[✓] Header generado en {output_header}")
    print(f"[✓] Implementación generada en {output_cpp}")
    print(f"[✓] {len(anchors)} certificados procesados")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convertir múltiples PEM/DER a header y cpp separados")
    parser.add_argument("input", help="Carpeta con .pem/.der")
    parser.add_argument("output_header", help="Archivo .h de salida (ej: trust_anchors.h)")
    parser.add_argument("output_cpp", help="Archivo .cpp de salida (ej: trust_anchors.cpp)")

    args = parser.parse_args()
    convert_all_pem_der_to_separate_files(args.input, args.output_header, args.output_cpp)


# import os
# import subprocess
# import argparse
#
# def convert_all_pem_der_to_single_header(input_folder, output_header):
#     anchors = []
#
#     with open(output_header, "w") as hfile:
#         hfile.write("// Auto-generated trust anchors header (single header version)\n")
#         hfile.write("#pragma once\n\n")
#         # hfile.write("#include <cstddef>\n\n")
#
#         # Estructura
#         hfile.write("struct TrustAnchor {\n")
#         hfile.write("  const char* name;\n")
#         hfile.write("  const unsigned char* data;\n")
#         hfile.write("  const int size;\n")
#         hfile.write("};\n\n")
#
#         print(f"[+] Procesando certificados en {input_folder}")
#         for file in sorted(os.listdir(input_folder)):
#             if not (file.endswith(".pem") or file.endswith(".der")):
#                 continue
#
#             base_name = os.path.splitext(file)[0]
#             safe_name = base_name.replace("-", "_").replace(" ", "_")
#
#             pem_path = os.path.join(input_folder, file)
#             der_path = os.path.join(input_folder, f"{base_name}.der")
#
#             # Convertir si es PEM
#             if file.endswith(".pem"):
#                 print(f"[+] Convirtiendo {pem_path} → {der_path}")
#                 subprocess.run(
#                     ["openssl", "x509", "-in", pem_path, "-outform", "der", "-out", der_path],
#                     check=True
#                 )
#             else:
#                 print(f"[+] Usando DER existente {pem_path}")
#                 der_path = pem_path
#
#             array_name = safe_name + "_der"
#             array_len = array_name + "_len"
#
#             # Ejecutar xxd
#             xxd_out = subprocess.run(
#                 ["xxd", "-i", "-n", array_name, der_path],
#                 check=True, capture_output=True, text=True
#             )
#
#             # Forzar inline en las definiciones
#             fixed = []
#             for line in xxd_out.stdout.splitlines():
#                 if line.strip().startswith("unsigned char"):
#                     fixed.append("inline " + line)
#                 elif line.strip().startswith("unsigned int"):
#                     fixed.append("inline " + line)
#                 else:
#                     fixed.append(line)
#
#             hfile.write(f"// Certificado: {base_name}\n")
#             hfile.write("\n".join(fixed))
#             hfile.write("\n\n")
#
#             anchors.append((base_name, array_name, array_len))
#
#         # Tabla global inline
#         hfile.write("inline const TrustAnchor trust_anchors[] = {\n")
#         for name, arr, length in anchors:
#             hfile.write(f'  {{ "{name}", {arr}, static_cast<int>({length}) }},\n')
#         hfile.write("};\n\n")
#         hfile.write(f"inline constexpr size_t trust_anchors_num = {len(anchors)};\n")
#
#     print(f"[✓] Header único generado en {output_header} con {len(anchors)} anchors")
#
#
# if __name__ == "__main__":
#     parser = argparse.ArgumentParser(description="Convertir múltiples PEM/DER a un solo header C++ con arrays inline")
#     parser.add_argument("input", help="Carpeta con .pem/.der")
#     parser.add_argument("output", help="Archivo .h de salida (ej: TrustAnchors.h)")
#
#     args = parser.parse_args()
#     convert_all_pem_der_to_single_header(args.input, args.output)
