#!/bin/bash
# file: utilities.sh

export LC_ALL=en_GB.UTF-8

# Directorio y fichero de log junto al daemon
BASE_DIR="/usr/local/drifterCtrl"
LOG_DIR="$BASE_DIR/logs"
LOG_FILE="$LOG_DIR/drifterCtl.log"

# Crear directorio de logs si no existe
mkdir -p "$LOG_DIR"
chmod 755 "$LOG_DIR"

# Estado cacheado de sincronización
_LAST_CHECK=0
TIME_SYNC_STATUS=2 # 0=ok, 1=desconocido, 2=no sincronizado

check_time_sync_cached() {
  local now
  now=$(date +%s)

  # Solo refrescar si ha pasado al menos 60s
  if [ $((now - _LAST_CHECK)) -lt 60 ]; then
    return
  fi

  if [ "$(timedatectl show -p NTPSynchronized --value 2>/dev/null)" = "yes" ]; then
    TIME_SYNC_STATUS=0
  else
    TIME_SYNC_STATUS=2
  fi

  _LAST_CHECK=$now
}

log2file() {
  check_time_sync_cached
  local datetime='[xxxx-xx-xx xx:xx:xx]'
  if [ "$TIME_SYNC_STATUS" -eq 0 ]; then
    datetime=$(date +'[%Y-%m-%d %H:%M:%S]')
  elif [ "$TIME_SYNC_STATUS" -eq 2 ]; then
    datetime=$(date +'<%Y-%m-%d %H:%M:%S>')
  fi
  local msg="$datetime $1"
  echo "$msg" >> "$LOG_FILE"
}

log() {
  if [ $# -gt 1 ] ; then
    echo "$2" "$1"
  else
    echo "$1"
  fi
  log2file "$1"
}
