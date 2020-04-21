#!/bin/sh


DATE=`date '+%Y-%m-%d %H:%M:%S'`
# Color Palette
RESET='\033[0m'
BOLD='\033[1m'

## Foreground
BLACK='\033[38;5;0m'
RED='\033[38;5;1m'
GREEN='\033[38;5;2m'
YELLOW='\033[38;5;3m'
BLUE='\033[38;5;4m'
MAGENTA='\033[38;5;5m'
CYAN='\033[38;5;6m'
WHITE='\033[38;5;7m'

## Background
ON_BLACK='\033[48;5;0m'
ON_RED='\033[48;5;1m'
ON_GREEN='\033[48;5;2m'
ON_YELLOW='\033[48;5;3m'
ON_BLUE='\033[48;5;4m'
ON_MAGENTA='\033[48;5;5m'
ON_CYAN='\033[48;5;6m'
ON_WHITE='\033[48;5;7m'

log() {
  echo "$DATE ${NAMI_DEBUG:+${CYAN} ${MAGENTA}$(date "+%T.%2N ")}${RESET}${@}" >&2
}

info() {
  log "${GREEN}INFO ${RESET} ==> ${@}"
}

warn() {
  log "${YELLOW}WARN ${RESET} ==> ${@}"
}

error() {
  log "${RED}ERROR${RESET} ==> ${@}"
}