#!/usr/bin/env bash
set -euo pipefail # Exit on error, undefined var, or pipe failure

# =========================
#  SBC Dependencies Installer
# =========================

# Utils
log() { echo -e "\033[1;32m[INFO]\033[0m $*"; }
warn() { echo -e "\033[1;33m[WARN]\033[0m $*"; }
err() { echo -e "\033[1;31m[ERR]\033[0m  $*"; }

# -------------------------
# Namespace: system
# -------------------------

# Detecta el tipo de placa en ejecución
system::detect_device() {
  local model
  model=$(tr -d '\0' < /proc/device-tree/model 2>/dev/null || echo "")

  if [[ "$model" =~ "Rock Pi S" ]]; then
    echo "rockpis"
  elif [[ "$model" =~ "Raspberry Pi" ]]; then
    echo "raspberrypi"
  else
    echo "unknown"
  fi
}

system::update() {
  log "Updating package lists..."
  sudo apt update
}

system::install_go() {
  log "Installing Go 1.25.4..."

  wget -O go.tar.gz https://go.dev/dl/go1.25.4.linux-arm64.tar.gz
  rm -rf /usr/local/go && sudo tar -C /usr/local -xzvf go.tar.gz

  rm -f go1.25.4.linux-arm64.tar.gz

  # Export PATH for current session
  export PATH="$PATH:/usr/local/go/bin"

  # Ensure ~/.bashrc contains Go PATH exactly once
  local go_path_line='export PATH="$PATH:/usr/local/go/bin"'

  if ! grep -qxF "$go_path_line" ~/.bashrc; then
    log "Adding Go path to ~/.bashrc..."
    echo "$go_path_line" >> ~/.bashrc
  else
    log "Go path already present in ~/.bashrc"
  fi

  log "Go installed to /usr/local/go"
}


system::install_base() {
  log "Installing base development tools..."
  sudo apt install -y build-essential cmake git pkg-config autoconf automake autopoint libtool wget curl unzip sshpass gpiod scons python3-full python3-pip python3-venv iptables-persistent ncdu
  system::install_go
}


# -------------------------
# Namespace: sqlite
# -------------------------
sqlite::termdbms_install() {
  log "Installing TermDBMS (SQLite3 TUI)..."
  go install github.com/mathaou/termdbms@latest
  go install -x -v github.com/mathaou/termdbms@latest
  mv -v "$HOME/go/bin/termdbms" "/usr/local/bin/termdbms"

  log "TermDBMS (SQLite3 TUI) installed to /usr/local/bin/termdbms"
}

sqlite::sqlite_web_install() {
  log "Installing sqlite-web (SQLite3 Web UI)..."

  local app_dir="/usr/local/sqlite-web"
  local venv_dir="$app_dir/.venv"

  sudo mkdir -p "$app_dir"

  # Instalar dependencias del sistema necesarias
  log "Updating package lists..."
  sudo apt update
  log "Installing system dependencies for sqlite-web..."
  sudo apt install -y python3-full python3-venv python3-pip

  # Crear entorno virtual
  log "Creating Python virtual environment in $venv_dir..."
  sudo python3 -m venv "$venv_dir"

  # Instalar sqlite-web dentro del venv
  log "Installing sqlite-web inside venv..."
  sudo "$venv_dir/bin/pip" install --upgrade pip wheel
  sudo "$venv_dir/bin/pip" install sqlite-web

  # Crear servicio systemd
  local service_file="/etc/systemd/system/sqlite-web.service"
  log "Installing sqlite-web systemd service..."

  sudo tee "$service_file" > /dev/null <<EOF
[Unit]
Description=SQLite Web UI
After=network.target

[Service]
ExecStart=$venv_dir/bin/sqlite_web /usr/local/iclisten-api/iclisten.db --host 0.0.0.0 --port 8081
WorkingDirectory=$app_dir
Restart=always
User=root

[Install]
WantedBy=multi-user.target
EOF

  sudo systemctl daemon-reload
  sudo systemctl enable sqlite-web.service
  sudo systemctl start sqlite-web.service

  log "sqlite-web installed using a dedicated venv."
  log "Access UI at: http://<THIS_DEVICE_IP>:8081"
}


sqlite::install() {
  log "Installing SQLite3..."
  sudo apt install -y libsqlite3-dev
#  sqlite::termdbms_install  # Disabled by default due to poor usability through SSH
#  sqlite::sqlite_web_install # Disabled by default to avoid extra resource usage (very slow compilation on SBCs)
}

# -------------------------
# Namespace: protobuf
# -------------------------
protobuf::install() {
  log "Installing Protobuf..."
  sudo apt install -y protobuf-compiler libprotobuf-dev
}

# -------------------------
# Namespace: libusb
# -------------------------
libusb::install() {
  log "Installing libusb..."
  sudo apt install -y libusb-1.0-0-dev
}

# -------------------------
# Namespace: util_linux
# -------------------------
util_linux::install() {
  log "Installing util_linux..."
  sudo apt install -y util-linux libmount-dev libblkid-dev
}

libboost::install() {
  log "Installing Boost libraries..."
#  sudo apt install -y libboost-thread-dev libboost-system-dev libxxhash-dev
   sudo apt install -y libboost-thread1.74-dev libboost-system1.74-dev libboost-program-options1.74-dev libxxhash-dev
}

# -------------------------
# Namespace: gtest
# -------------------------
gtest::install() {
  log "Installing GoogleTest headers..."
  sudo apt install -y libgtest-dev
  # Build static libs manually (Ubuntu only ships sources)
  local gtest_dir="/usr/src/googletest"
  if [ -d "$gtest_dir" ]; then
    log "Building GoogleTest from sources..."
    sudo cmake -S "$gtest_dir" -B "$gtest_dir/build"
    sudo cmake --build "$gtest_dir/build" --target install
  else
    warn "GoogleTest sources not found in /usr/src/googletest"
  fi
}

# -------------------------
# Namespace: asio
# -------------------------
asio::install() {
  log "Installing Asio headers..."
  sudo apt install -y libasio-dev
  # Optional: Create CMake config so find_package(asio) works
  local cmake_dir="/usr/local/lib/cmake/asio"
  sudo mkdir -p "$cmake_dir"
  cat <<'EOF' | sudo tee "$cmake_dir/asioConfig.cmake" > /dev/null
find_path(ASIO_INCLUDE_DIR NAMES asio.hpp PATHS /usr/include /usr/local/include)
if(NOT ASIO_INCLUDE_DIR)
  message(FATAL_ERROR "Asio headers not found")
endif()
add_library(asio::asio INTERFACE IMPORTED)
set_target_properties(asio::asio PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${ASIO_INCLUDE_DIR}"
)
EOF
  log "Custom asioConfig.cmake installed to $cmake_dir"
}

# -------------------------
# Namespace: sndfile
# -------------------------
sndfile::install() {
  log "Building and installing libsndfile from source..."
  sudo apt install -y autoconf autogen automake libasound2-dev \
    libflac-dev libogg-dev libtool libvorbis-dev libopus-dev \
    libmp3lame-dev libmpg123-dev

  git clone --depth=1 https://github.com/libsndfile/libsndfile.git /tmp/libsndfile
  cmake -S /tmp/libsndfile -B /tmp/libsndfile/build -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF \
    -DENABLE_PACKAGE_CONFIG=ON
  cmake --build /tmp/libsndfile/build -j"$(nproc)"
  sudo cmake --install /tmp/libsndfile/build
  rm -rf /tmp/libsndfile
}

# -------------------------
# Namespace: crow
# -------------------------
crow::install() {
  log "Building and installing Crow from source..."
  git clone --depth=1 https://github.com/CrowCpp/Crow.git /tmp/Crow
  cmake -S /tmp/Crow -B /tmp/Crow/build -DCMAKE_BUILD_TYPE=Release \
    -DCROW_BUILD_EXAMPLES=OFF -DCROW_BUILD_TESTS=OFF
  sudo cmake --build /tmp/Crow/build --target install
  rm -rf /tmp/Crow
}

# -------------------------
# Namespace: iclisten
# -------------------------
iclisten::configure_network_eth_route() {
  local target_ip="192.168.10.150"
  local static_ip="192.168.10.10/32"
  local static_conn_name="ICListen-Fallback"
  local dhcp_conn_name="Ethernet-DHCP"

  log "Configuring clean dual-profile setup for ICListen ($target_ip)..."

  # Detectar todas las interfaces Ethernet reales
  local eth_ifaces
  eth_ifaces=$(ip -o link show | awk -F': ' '{print $2}' | grep -E '^(end|eth|enx)' | tr '\n' ' ')
  if [ -z "$eth_ifaces" ]; then
    error "No Ethernet interfaces found. Aborting."
    return 1
  fi

  # Detectar todos los perfiles NM de tipo ethernet
  local eth_profiles
  eth_profiles=$(nmcli -g NAME,TYPE connection show | grep "ethernet" | cut -d: -f1)

  for eth_iface in $eth_ifaces; do
    log "Processing Ethernet interface: $eth_iface"
    # Eliminar todas las conexiones NM tipo ethernet asociadas
    for conn in $eth_profiles; do
      local conn_iface
      conn_iface=$(nmcli -g connection.interface-name connection show "$conn" 2>/dev/null)
      if [ "$conn_iface" = "$eth_iface" ]; then
        log "Deleting old NetworkManager ethernet connection '$conn' for $eth_iface..."
        nmcli connection delete "$conn" >/dev/null 2>&1 || true
      fi
    done

    # Crear conexión DHCP principal
    log "Creating DHCP connection '$dhcp_conn_name' on $eth_iface..."
    nmcli connection add type ethernet ifname "$eth_iface" con-name "$dhcp_conn_name" \
      ipv4.method auto \
      ipv4.may-fail yes \
      ipv6.method ignore \
      connection.autoconnect yes \
      connection.autoconnect-retries -1 \
      connection.wait-device-timeout 0

    # Crear conexión estática secundaria
    log "Creating static fallback connection '$static_conn_name' on $eth_iface..."
    nmcli connection add type ethernet ifname "$eth_iface" con-name "$static_conn_name" \
      ipv4.method manual \
      ipv4.addresses "$static_ip" \
      ipv4.routes "$target_ip/32" \
      ipv6.method ignore \
      connection.autoconnect yes \
      connection.autoconnect-retries -1 \
      connection.wait-device-timeout 0

    # Asegurar que NM no desactive la interfaz aunque no haya carrier
    log "Ensuring NetworkManager keeps $eth_iface active..."
    nmcli device set "$eth_iface" managed yes
    nmcli device set "$eth_iface" autoconnect yes

    # Activar ambas conexiones
    log "Bringing up DHCP and fallback connections for $eth_iface..."
    nmcli connection up "$dhcp_conn_name" || warn "DHCP may not be active yet on $eth_iface."
    nmcli connection up "$static_conn_name" || warn "Static fallback may not be active yet on $eth_iface."

    # Verificar la ruta
    if ip route get "$target_ip" 2>/dev/null | grep -q "$eth_iface"; then
      log "Verified: $target_ip is routed via $eth_iface ✅"
    else
      warn "Route to $target_ip not yet visible. NetworkManager may still be settling."
    fi
  done
}

iclisten::configure_iptables_redirect_8080() {
    local target_ip="192.168.10.150"
    local target_port="80"

    log "Enabling IPv4 forwarding..."
    # Runtime
    sudo sysctl -w net.ipv4.ip_forward=1 >/dev/null
    # Persistente
    if ! grep -q "^net.ipv4.ip_forward=1" /etc/sysctl.conf; then
        sudo sed -i 's/^#*net.ipv4.ip_forward=.*/net.ipv4.ip_forward=1/' /etc/sysctl.conf
    fi

    log "Applying iptables NAT rules (idempotent)..."

    # Avoid duplicates for DNAT
    # Redirects all incoming traffic on this machine at port 8080 → 192.168.10.150:80
    if ! sudo iptables -t nat -C PREROUTING -p tcp --dport 8080 -j DNAT --to-destination ${target_ip}:${target_port} 2>/dev/null; then
        sudo iptables -t nat -A PREROUTING -p tcp --dport 8080 -j DNAT --to-destination ${target_ip}:${target_port}
        log "Added DNAT rule: 8080 → ${target_ip}:${target_port}"
    else
        log "DNAT rule already exists, skipping"
    fi

    # Evitar duplicados para MASQUERADE
    if ! sudo iptables -t nat -C POSTROUTING -j MASQUERADE 2>/dev/null; then
        sudo iptables -t nat -A POSTROUTING -j MASQUERADE
        log "Added MASQUERADE rule"
    else
        log "MASQUERADE already exists, skipping"
    fi

    log "Installing iptables-persistent if required..."
    sudo apt install -y iptables-persistent

    log "Saving iptables rules persistently..."
    sudo netfilter-persistent save

    log "Final NAT rules:"
    sudo iptables -t nat -L -v -n
}




iclisten::api() {
  dirname=$(dirname "$0")
  local src_dir="$dirname/iclisten-api"
  local target_dir="/usr/local/iclisten-api"
  local service_file="/etc/systemd/system/iclisten-api.service"
  local logrotate_file="/etc/logrotate.d/iclisten-api"

  log "Installing ICListen API Service..."

  # Crear estructura de directorios
  sudo mkdir -p "$target_dir/logs"

  # Copiar binario y base de datos
  sudo cp "$src_dir/api" "$target_dir/"
  sudo cp "$src_dir/iclisten.db" "$target_dir/"
  sudo chmod +x "$target_dir/api"

  # Instalar unit de systemd
  sudo cp "$src_dir/iclisten-api.service" "$service_file"

  # Configurar logrotate
  log "Configuring logrotate for iclisten-api logs..."
  sudo tee "$logrotate_file" > /dev/null <<'EOF'
/usr/local/iclisten-api/logs/api.log {
    daily
    rotate 7
    compress
    delaycompress
    missingok
    notifempty
    create 0644 root root
    postrotate
        systemctl kill -s HUP iclisten-api.service >/dev/null 2>&1 || true
    endscript
}
EOF

  # Validar logrotate y habilitar servicio
  sudo logrotate -d /etc/logrotate.d/iclisten-api
  sudo systemctl daemon-reload
  sudo systemctl enable iclisten-api.service

  log "ICListen API installed to $target_dir"
  log "Service enabled: iclisten-api.service"
  log "Start with: sudo systemctl start iclisten-api"
}


iclisten::info() {
  warn "ICListen SDK must be installed manually from vendor."
  warn "Ensure it provides /usr/local/lib/cmake/iclisten/iclistenConfig.cmake"
}

iclisten::all() {
  iclisten::configure_network_eth_route
  iclisten::configure_iptables_redirect_8080
  iclisten::api
  iclisten::info
}

# -------------------------
# Namespace: overlays
# -------------------------
overlays::install() {
  current_dir="$(dirname "$0")"
  local overlay_dir="$current_dir/overlays"
  local target_dir="/boot/overlay-user"
  local env_file="/boot/armbianEnv.txt"

  log "Installing device tree overlays from: $overlay_dir"

  if command -v armbian-add-overlay >/dev/null 2>&1; then
    log "Using armbian-add-overlay command"
    for dts in "$overlay_dir"/*.dts; do
      log "Adding overlay $dts"
      sudo armbian-add-overlay "$dts"
    done
  else
    warn "armbian-add-overlay not found, using manual fallback"
    sudo apt install -y device-tree-compiler
    sudo mkdir -p "$target_dir"
    local overlays=()
    for dts in "$overlay_dir"/*.dts; do
      local base
      base="$(basename "$dts" .dts)"
      local dtbo="$target_dir/${base}.dtbo"
      log "Compiling $dts -> $dtbo"
      sudo dtc -@ -I dts -O dtb -o "$dtbo" "$dts"
      overlays+=("$base")
    done

    if [ -f "$env_file" ]; then
      log "Updating $env_file with overlays: ${overlays[*]}"
      sudo sed -i "/^user_overlays=/d" "$env_file"
      echo "user_overlays=${overlays[*]}" | sudo tee -a "$env_file" > /dev/null
    else
      err "$env_file not found! Cannot update overlays."
    fi
  fi

  log "Overlays installation finished."
}

# -------------------------
# Namespace: pps
# -------------------------
pps::install() {
  log "Installing GPSD, PPS tools, and Chrony..."

  sudo apt update
  sudo apt install -y gpsd gpsd-clients pps-tools chrony libgps-dev

  log "PPS/GPS packages installed:"
  log " - gpsd (daemon)"
  log " - gpsd-clients (cgps, gpsmon, etc)"
  log " - pps-tools (ppstest)"
  log " - chrony (NTP server with PPS support)"
}

pps::configure() {
  log "Configuring PPS + GPS support"

  local board
  board=$(system::detect_device)

  local gps_device=""
  case "$board" in
    rockpis)
      log "Detected => Rock Pi S"
      gps_device="/dev/ttyS1"
      ;;
    raspberrypi)
      log "Detected => Raspberry Pi"
      if [[ -e /dev/serial0 ]]; then
        gps_device="/dev/serial0"
      elif [[ -e /dev/ttyAMA0 ]]; then
        gps_device="/dev/ttyAMA0"
      else
        err "No suitable serial device found for GPS on Raspberry Pi."
        exit 1
      fi
      ;;
    *)
      err "Unsupported or unknown board type: '$board'. Cannot configure PPS."
      exit 1
      ;;
  esac

  log "Detected platform: $board"
  log "Using GPS device: $gps_device"

  local gpsd_conf="/etc/default/gpsd"
  local chrony_conf="/etc/chrony/chrony.conf"

  # --- gpsd config ---
  log "Updating $gpsd_conf"
  sudo cp "$gpsd_conf" "$gpsd_conf.bak.$(date +%Y%m%d%H%M%S)" || true
  sudo sed -i "s|^DEVICES=.*|DEVICES=\"$gps_device\"|" "$gpsd_conf"
  sudo sed -i "s|^GPSD_OPTIONS=.*|GPSD_OPTIONS=\"-n\"|" "$gpsd_conf"
  sudo sed -i "s|^USBAUTO=.*|USBAUTO=\"true\"|" "$gpsd_conf"

  # --- chrony config ---
  log "Updating $chrony_conf"
  sudo cp "$chrony_conf" "$chrony_conf.bak.$(date +%Y%m%d%H%M%S)" || true
  sudo sed -i '/^refclock SHM/d' "$chrony_conf"
  sudo sed -i '/^refclock PPS/d' "$chrony_conf"
  cat <<EOF | sudo tee -a "$chrony_conf" > /dev/null

# GPS (NMEA)
refclock SHM 0 refid GPS precision 1e-1 offset 0.9999 delay 0.2

# PPS
refclock PPS /dev/pps0 refid PPS
EOF

  # --- restart services ---
  log "Restarting gpsd and chrony"
  sudo systemctl restart gpsd
  sudo systemctl restart chrony || sudo systemctl restart chronyd

  log "PPS + GPS configuration applied successfully"
  log "Test with: ppstest /dev/pps0, cgps -s, chronyc sources -v"
}

# -------------------------
# Namespace: daemon
# -------------------------
daemon::install() {
  dirname=$(dirname "$0")
  local src_dir="$dirname/daemon"
  local target_dir="/usr/local/drifterCtrl"
  local service_file="/etc/systemd/system/drifterCtrl.service"
  local logrotate_file="/etc/logrotate.d/drifterCtrl"

  log "Installing Drifter Control Daemon..."

  # Crear directorio destino
  sudo mkdir -p "$target_dir"

  # Copiar scripts
  sudo cp "$src_dir"/drifterCtrlDaemon.sh "$target_dir/"
  sudo cp "$src_dir"/utilities.sh "$target_dir/"
  sudo chmod +x "$target_dir"/drifterCtrlDaemon.sh
  sudo chmod +x "$target_dir"/utilities.sh

  # Instalar service en systemd
  sudo cp "$src_dir/drifterCtrl.service" "$service_file"
  # Instalar configuración de logrotate
  log "Configuring logrotate for drifterCtrl logs..."
  sudo tee "$logrotate_file" > /dev/null <<'EOF'
/usr/local/drifterCtrl/logs/drifterCtrl.log {
    daily
    rotate 7
    compress
    delaycompress
    missingok
    notifempty
    create 0644 root root
    postrotate
        systemctl kill -s HUP drifterCtrl.service >/dev/null 2>&1 || true
    endscript
}
EOF
  # Probar configuración de logrotate
  sudo logrotate -d /etc/logrotate.d/drifterCtrl

  # Recargar systemd y habilitar
  sudo systemctl daemon-reload
  sudo systemctl enable drifterCtrl.service

  log "Drifter Control Daemon installed to $target_dir"
  log "Service enabled: drifterCtrl.service"
  log "Start with: sudo systemctl start drifterCtrl"
}


# -------------------------
# Namespace: test
# -------------------------
test::generate_CMakeLists() {
  local tmp_dir="${1:-/tmp/test_deps}"   # si no pasas argumento usa /tmp/test_deps por defecto

  log "Testing installed dependencies with CMake..."
  rm -rf "$tmp_dir"
  mkdir -p "$tmp_dir"
  cat > "$tmp_dir/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.18)
project(test_deps)

# ---- ASIO ----
find_path(ASIO_INCLUDE_DIR NAMES asio.hpp PATHS /usr/include /usr/local/include)
if(ASIO_INCLUDE_DIR)
    message(STATUS "[ASIO] Found in: ${ASIO_INCLUDE_DIR}")
    add_library(asio::asio INTERFACE IMPORTED)
    set_target_properties(asio::asio PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ASIO_INCLUDE_DIR}")
    find_file(ASIO_VERSION_HPP "asio/version.hpp" PATHS ${ASIO_INCLUDE_DIR}/asio)
    if (ASIO_VERSION_HPP)
        file(READ ${ASIO_VERSION_HPP} ASIO_VERSION_CONTENT)
        string(REGEX MATCH "#define ASIO_VERSION ([0-9]+)" _ ${ASIO_VERSION_CONTENT})
        if (CMAKE_MATCH_1)
            math(EXPR ASIO_VERSION_MAJOR "${CMAKE_MATCH_1} / 100000")
            math(EXPR ASIO_VERSION_MINOR "(${CMAKE_MATCH_1} / 1000) % 100")
            math(EXPR ASIO_VERSION_PATCH "${CMAKE_MATCH_1} % 1000")
            set(ASIO_VERSION_STRING "${ASIO_VERSION_MAJOR}.${ASIO_VERSION_MINOR}.${ASIO_VERSION_PATCH}")
            message(STATUS "[ASIO] Detected version: ${ASIO_VERSION_STRING}")
        endif()
    endif()
else()
    message(WARNING "[ASIO] Not found on system")
endif()

# ---- CROW ----
find_package(Crow QUIET)
if (Crow_FOUND)
    message(STATUS "[CROW] Found: ${Crow_VERSION}")
else ()
    message(WARNING "[CROW] Not found")
endif ()

# ---- SQLITE3 ----
find_package(SQLite3 QUIET)
if (SQLite3_FOUND)
    message(STATUS "[SQLITE3] Found: v${SQLite3_VERSION}")
else ()
    message(WARNING "[SQLITE3] Not found")
endif ()

# ---- SNDFILE ----
find_package(SndFile QUIET)
if (SndFile_FOUND)
    message(STATUS "[SNDFILE] Found: ${SndFile_VERSION}")
else ()
    message(WARNING "[SNDFILE] Not found")
endif ()

# ---- PROTOBUF ----
find_package(Protobuf QUIET)
if (Protobuf_FOUND)
    message(STATUS "[PROTOBUF] Found: v${Protobuf_VERSION} Executable: ${Protobuf_PROTOC_EXECUTABLE}")
else ()
    message(WARNING "[PROTOBUF] Not found")
endif ()

# ---- LIBUSB ----
find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(LIBUSB libusb-1.0)
endif ()
if (LIBUSB_FOUND)
    message(STATUS "[LIBUSB] Found: v${LIBUSB_VERSION}")
else ()
    message(WARNING "[LIBUSB] Not found")
endif ()

# ---- GTEST ----
find_package(GTest QUIET)
if (GTest_FOUND)
    message(STATUS "[GTEST] Found include dirs: ${GTEST_INCLUDE_DIRS}")
else ()
    message(WARNING "[GTEST] Not found")
endif ()

# ---- ICLISTEN ----
set(iclisten_DIR "/usr/local/lib/cmake/iclisten")
find_package(iclisten QUIET)
if (iclisten_FOUND)
    message(STATUS "[ICLISTEN] Found")
else ()
    message(WARNING "[ICLISTEN] Not found (manual install required)")
endif ()
EOF

}


test::cmake_dependencies() {
  log "Generating installation summary table..."
  local cmakelists_file="/tmp/test_dependencies"
  test::generate_CMakeLists "$cmakelists_file"

  local tmp_dir="/tmp/test_summary"
  rm -rf "$tmp_dir"
  mkdir -p "$tmp_dir"

  # Ejecutar CMake y capturar salida
  local output_file="$tmp_dir/output.log"
  cmake -S "$cmakelists_file" -B "$tmp_dir/build" 2>&1 | tee "$output_file" || true


  # Mapa local de dependencias esperadas
  declare -A TEST_DEPS_MAP=(
    ["ASIO"]="1"
    ["CROW"]="1"
    ["SQLITE3"]="1"
    ["SNDFILE"]="1"
    ["PROTOBUF"]="1"
    ["LIBUSB"]="1"
    ["GTEST"]="1"
    ["ICLISTEN"]="1"
  )

  # Tabla
  printf "\n%-15s | %-6s | %s\n" "DEPENDENCY" "STATUS" "DETAILS"
  printf "%-15s-+-%-6s-+-%s\n" "---------------" "------" "----------------------------"

  # Leer resultados
  while read -r line; do
    if [[ "$line" =~ \[(.*)\]\ (.*) ]]; then
      dep="${BASH_REMATCH[1]}"
      msg="${BASH_REMATCH[2]}"

      if [[ -n "${TEST_DEPS_MAP[$dep]+exists}" ]]; then
        if [[ "$msg" == *"Found"* ]] || [[ "$msg" == *"Detected"* ]]; then
          printf "%-15s | %-6s | %s\n" "$dep" "OK" "$msg"
        else
          printf "%-15s | %-6s | %s\n" "$dep" "FAIL" "$msg"
        fi
      fi
    fi
  done < "$output_file"

  printf "\n"
}

test::system_programs() {
  log "Checking system-level installed tools..."

  declare -A PROGRAMS=(
    ["gcc"]="command -v gcc"
    ["g++"]="command -v g++"
    ["cmake"]="command -v cmake"
    ["git"]="command -v git"
    ["pkg-config"]="command -v pkg-config"
    ["autoconf"]="command -v autoconf"
    ["automake"]="command -v automake"
    ["libtool"]="command -v libtoolize"
    ["wget"]="command -v wget"
    ["curl"]="command -v curl"
    ["unzip"]="command -v unzip"
    ["sshpass"]="command -v sshpass"
    ["gpiod"]="command -v gpioinfo && command -v gpioset && command -v gpioget && command -v gpiodetect && command -v gpiofind"
    ["scons"]="command -v scons"
    ["go"]="command -v go"
    ["termdbms"]="command -v termdbms"
    ["sqlite-web"]="command -v /usr/local/sqlite-web/.venv/bin/sqlite_web"
    ["python3"]="command -v python3"
    ["pip3"]="command -v pip3"
    ["python3-venv"]="command -v python3 -m venv --help"
    ["protoc"]="command -v protoc"
    ["dtc"]="command -v dtc"
    ["gpsd"]="command -v gpsd"
    ["ppstest"]="command -v ppstest"
    ["chronyc"]="command -v chronyc"
    ["cgps"]="command -v cgps"
    ["netfilter-persistent"]="command -v netfilter-persistent"
    ["ncdu"]="command -v ncdu"


    # Servicios
    ["iclisten-api"]="systemctl status iclisten-api.service"
    ["drifterCtrl"]="systemctl status drifterCtrl.service"
  )

  printf "\n%-20s | %-6s | %s\n" "PROGRAM" "STATUS" "DETAILS"
  printf "%-20s-+-%-6s-+-%s\n" "--------------------" "------" "----------------------------"

  for prog in "${!PROGRAMS[@]}"; do
    check_cmd="${PROGRAMS[$prog]}"

    if eval $check_cmd > /dev/null 2>&1; then
      printf "%-20s | %-6s | %s\n" "$prog" "OK" "Installed"
    else
      printf "%-20s | %-6s | %s\n" "$prog" "FAIL" "Not installed or not in PATH"
    fi
  done

  printf "\n"
}

test::summary() {
  test::cmake_dependencies
  test::system_programs
}


# -------------------------
# Main
# -------------------------
all::install() {
  system::update
  system::install_base
  sqlite::install
  protobuf::install
  libusb::install
  util_linux::install
  libboost::install
  gtest::install
  asio::install
  sndfile::install
  crow::install
  overlays::install
  pps::install
  pps::configure
  daemon::install
  iclisten::all
  test::summary
  log "All dependencies installed."
}

only_packages::install() {
  system::update
  system::install_base
  sqlite::install
  protobuf::install
  libusb::install
  util_linux::install
  libboost::install
  gtest::install
  asio::install
  sndfile::install
  crow::install
  log "Selected packages installed."
}

# -------------------------
# Entry Point
# -------------------------
case "${1:-}" in
  all)        all::install ;;
  only_packages) only_packages::install ;;
  system)     system::update; system::install_base ;;
  sqlite)     sqlite::install ;;
  protobuf)   protobuf::install ;;
  libusb)     libusb::install ;;
  util_linux) util_linux::install ;;
  libboost)   libboost::install ;;
  gtest)      gtest::install ;;
  asio)       asio::install ;;
  sndfile)    sndfile::install ;;
  crow)       crow::install ;;
  overlays)   overlays::install ;;
  pps)        pps::install; pps::configure ;;
  daemon)     daemon::install ;;
  iclisten)   iclisten::all ;;
  test)       test::summary ;;
  test_cmake)    test::cmake_dependencies ;;
  test_system)       test::system_programs ;;

  *)          echo "Usage: $0 {all|only_packages|system|sqlite|protobuf|libusb|util_linux|libboost|gtest|asio
  |sndfile|crow|overlays|pps|daemon|iclisten|test|test_cmake|test_system}" ;;
esac
