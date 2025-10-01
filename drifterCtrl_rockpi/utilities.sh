#!/bin/bash
# file: utilities.sh

export LC_ALL=en_GB.UTF-8

log2file()
{
  local datetime='[xxxx-xx-xx xx:xx:xx]'
  if [ $TIME_UNKNOWN -eq 0 ]; then
    datetime=$(date +'[%Y-%m-%d %H:%M:%S]')
  elif [ $TIME_UNKNOWN -eq 2 ]; then
    datetime=$(date +'<%Y-%m-%d %H:%M:%S>')
  fi
  local msg="$datetime $1"
  echo $msg >> /home/user/drifterCtl/drifterCtl.log
}

log()
{
  if [ $# -gt 1 ] ; then
    echo $2 "$1"
  else
    echo "$1"
  fi
  log2file "$1"
}
