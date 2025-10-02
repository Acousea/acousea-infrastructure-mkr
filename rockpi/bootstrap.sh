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
  sudo apt install -y build-essential cmake git pkg-config
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

  # Recargar systemd y habilitar
  sudo systemctl daemon-reload
  sudo systemctl enable drifterCtrl.service

  log "Drifter Control Daemon installed to $target_dir"
  log "Service enabled: drifterCtrl.service"
  log "Start with: sudo systemctl start drifterCtrl"
}


# -------------------------
# Main
# -------------------------
all::install() {
  system::update
  system::install_base
  sqlite::install
  protobuf::install
  gtest::install
  asio::install
  sndfile::install
  crow::install
  overlays::install
  pps::install
  pps::configure
  daemon::install
  iclisten::info
  log "All dependencies installed."
}


# -------------------------
# Entry Point
# -------------------------
case "${1:-}" in
  all)        all::install ;;
  sqlite)     sqlite::install ;;
  protobuf)   protobuf::install ;;
  gtest)      gtest::install ;;
  asio)       asio::install ;;
  sndfile)    sndfile::install ;;
  crow)       crow::install ;;
  overlays)   overlays::install ;;
  pps)        pps::install; pps::configure ;;
  daemon)     daemon::install ;;
  iclisten)   iclisten::info ;;
  *)          echo "Usage: $0 {all|sqlite|protobuf|gtest|asio|sndfile|crow|iclisten}" ;;
esac
