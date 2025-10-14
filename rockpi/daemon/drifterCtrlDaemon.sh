#!/bin/bash

# Cargar utilidades
cur_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

. "$cur_dir/utilities.sh"


log "Drifter daemon is starting."

# Esperar a que el GPIO se estabilice
wait_for_gpio_stable() {
  local gpio_chip_and_pin=$1
  local retries=5
  local counter=0

  log "Waiting for PIN=$gpio_chip_and_pin to stabilize"

  while [ $counter -lt $retries ]; do
    if [ "$(gpioget $gpio_chip_and_pin)" == "0" ]; then
      ((counter++))
      log "Waiting for signals to stabilize count=$counter"
    else
      counter=0
      log "Signals not yet stable, resetting counter=$counter"
    fi
    sleep 1
  done
}

# Monitorizar señal de apagado con verificación
monitor_shutdown_signal() {
  local stable_count=0
  local required=3   # nº de lecturas consecutivas
  local pin=$1

  log "Monitoring shutdown signal on $pin"

  while true; do
    val=$(gpioget $pin)
    if [ "$val" == "1" ]; then
      ((stable_count++))
      log "Shutdown signal candidate $stable_count/$required"
      if [ $stable_count -ge $required ]; then
        log "Confirmed shutdown signal"
        break
      fi
    else
      stable_count=0
    fi
    sleep 1
  done
}
# --------------------------------------------------
# > SSH Connection: ssh -oKexAlgorithms=diffie-hellman-group14-sha1 -oHostKeyAlgorithms=ssh-rsa -oMACs=hmac-sha1 -c aes256-cbc icListen@192.168.10.150
# > SCP Connection: scp -r -oKexAlgorithms=diffie-hellman-group14-sha1 -oHostKeyAlgorithms=ssh-rsa -c aes256-cbc icListen@192.168.10.150:/home/icListen/ ./iclisten_backup
#   Password: (empty)
# --------------------------------------------------
shutdown_iclisten() {
  log "Initiating icListen shutdown sequence..."

  # 1. Mandar comando seguro al icListen para que se apague por software
  sshpass -p '' ssh \
    -oKexAlgorithms=+diffie-hellman-group14-sha1 \
    -oHostKeyAlgorithms=+ssh-rsa \
    -oCiphers=aes128-cbc,aes256-cbc,3des-cbc \
    -oUserKnownHostsFile=/dev/null \
    -oStrictHostKeyChecking=no \
    icListen@192.168.10.150 'busybox sync && busybox poweroff' || \
    log "WARNING: SSH poweroff command to icListen failed."

  log "SSH poweroff command sent to icListen."

  # 2. Cortar la alimentación del TRACO tras un pequeño retardo
  gpioset $TRACO_POWER_ON_OFF_PIN=0
  log "Power to icListen cut off via TRACO."
}

# Define shutdown signal pin
SHUTDOWN_SIGNAL_PIN="gpiochip0 16"
TRACO_POWER_ON_OFF_PIN="gpiochip0 17"

mode=$1 # Receives the mode command to either startup or shutdown the system
case "$mode" in
  run)
    log "Drifter daemon is starting in run mode."

    gpioset $TRACO_POWER_ON_OFF_PIN=0 # Turn the tracopower device off at the beginning to stabilize (ICLISTEN)

    wait_for_gpio_stable "$SHUTDOWN_SIGNAL_PIN"
    log "GPIO signals are stable."

    gpioset $TRACO_POWER_ON_OFF_PIN=1 # Turn on the tracopower device (ICLISTEN)

    monitor_shutdown_signal "$SHUTDOWN_SIGNAL_PIN"

    shutdown_iclisten
    log "System will shutdown now."

    shutdown -h now
    ;;
  stop)
    log "System is shutting down, forcing TRACO power off..."
    shutdown_iclisten
    ;;
  *)
    echo "Error=> Usage: $0 {run|stop}"
    exit 1
    ;;
esac