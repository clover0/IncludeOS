#!/usr/bin/env bash
#
# transfer to USB for macOS
#

set -eu

function main() {
  dev="$1"
  part="$2"
  #
  # show USB device
  #    diskutil list
  # 
  if [ "$dev" = "" ]; then
    echo "arg1: require device name (e.g. disk2)"
    exit 1
  fi
  if [ "$part" = "" ]; then
    echo "arg2: require partition name (e.g. disk2s1)"
    exit 1
  fi

  echo "copy includeos src to USB"
  mkdir -p /tmp/mnt
  diskutil unmount /dev/"$part"
  sudo diskutil mount -mountPoint /tmp/mnt /dev/"$part"
  sudo mkdir -p /tmp/mnt/includeos_usb
#   cp -rf ./ /tmp/mnt/includeos_usb
#   rsync -a ./ /tmp/mnt/includeos_usb
  rsync -a --delete ./runner_build /tmp/mnt/includeos_usb
  sync -f /tmp/mnt/includeos_usb/runner_build
  diskutil unmount /tmp/mnt 
  rmdir /tmp/mnt
  echo "finish copy."

}

main "$1" "$2"
