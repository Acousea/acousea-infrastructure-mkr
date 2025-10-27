#!/usr/bin/env bash
set -euo pipefail # Exit on error, undefined var, or pipe failure

# =========================
#  RockPi-S Dependencies Installer
# =========================

# Utils
log() { echo -e "\033[1;32m[INFO]\033[0m $*"; }
warn() { echo -e "\033[1;33m[WARN]\033[0m $*"; }
err() { echo -e "\033[1;31m[ERR]\033[0m  $*"; }

# -------------------------
# Namespace: system
# -------------------------
system::update() {
  log "Updating package lists..."
  sudo apt update
}

system::install_base() {
  log "Installing base development tools..."
  sudo apt install -y build-essential cmake git pkg-config autoconf automake libtool wget curl unzip sshpass gpiod
}

# -------------------------
# Namespace: sqlite
# -------------------------
sqlite::install() {
  log "Installing SQLite3..."
  sudo apt install -y libsqlite3-dev
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
  sudo apt install -y gpsd gpsd-clients pps-tools chrony

  log "PPS/GPS packages installed:"
  log " - gpsd (daemon)"
  log " - gpsd-clients (cgps, gpsmon, etc)"
  log " - pps-tools (ppstest)"
  log " - chrony (NTP server with PPS support)"
}

pps::configure() {
  local gps_device="/dev/ttyS1"
  local gpsd_conf="/etc/default/gpsd"
  local chrony_conf="/etc/chrony/chrony.conf"

  log "Configuring PPS + GPS support"

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

# Configuración del GPS (NMEA)
refclock SHM 0 refid GPS precision 1e-1 offset 0.9999 delay 0.2

# Configuración de PPS
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
test::deps() {
  log "Testing installed dependencies with CMake..."

  local tmp_dir="/tmp/test_deps"
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

  # Run cmake in test mode
  cmake -S "$tmp_dir" -B "$tmp_dir/build"
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
  gtest::install
  asio::install
  sndfile::install
  crow::install
  overlays::install
  pps::install
  pps::configure
  daemon::install
  iclisten::info
  iclisten::api
  iclisten::configure_network_eth_route
  test::deps
  log "All dependencies installed."
}

only_packages::install() {
  system::update
  system::install_base
  sqlite::install
  protobuf::install
  libusb::install
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
  gtest)      gtest::install ;;
  asio)       asio::install ;;
  sndfile)    sndfile::install ;;
  crow)       crow::install ;;
  overlays)   overlays::install ;;
  pps)        pps::install; pps::configure ;;
  daemon)     daemon::install ;;
  iclisten)   iclisten::info; iclisten::api; iclisten::configure_network_eth_route ;;
  test)       test::deps ;;
  *)          echo "Usage: $0 {all|only_packages|system|sqlite|protobuf|libusb|gtest|asio|sndfile|crow|overlays|pps|daemon|iclisten|test}" ;;
esac
