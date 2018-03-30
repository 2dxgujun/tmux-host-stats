#!/usr/bin/env bash

set -e

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

EXECUTABLE_PATH="$CURRENT_DIR/build/tmux-host-stats"

source "$CURRENT_DIR/helpers.sh"

main() {
	local interval="$(get_tmux_option "status-interval")"

  # local stats="$($EXECUTABLE_PATH -i $interval -a 1)"
  local stats="$($EXECUTABLE_PATH -a 1)"
  local stats_arr=($stats)

  # echo '[ǀǀǀǀǀǀǀ]'

  local available_mem=${stats_arr[0]}
  local cpu_percentage=${stats_arr[1]}
  local load_avg=${stats_arr[2]}

  local level=$(bc <<< "(($cpu_percentage*0.07)+0.5)/1") # 0..7
  # echo $level
 	if (( $level > 7 )); then
 		level=7
 	fi

  echo "$available_mem $load_avg [$(printf "ǀ%.0s" $(seq 1 $level))$(printf " %.0s" $(seq 1 $((7-$level))))]"

  # printf "\x1b[38;5;${i}mcolour${i}\x1b[0m\n"
  # for i in 2 2 2 3 3 1 1; do
  #   printf "\x1b[38;5;${i}mcolour${i}\x1b[0m\n"
  # done

  # echo "#[bg=default][#[fg=colour1]HELLOǀ#[fg=colour1]ǀ#[fg=colour1]ǀ#[default]]"
}

main