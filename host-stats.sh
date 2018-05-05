#!/bin/bash -e

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

EXECUTABLE_PATH="$CURRENT_DIR/build/tmux-host-stats"

source "$CURRENT_DIR/helpers.sh"

if
[ -z "$SCALE0" ] ||
[ -z "$SCALE1" ] ||
[ -z "$SCALE2" ] ||
[ -z "$SCALE3" ] ||
[ -z "$SCALE4" ] ||
[ -z "$SCALE5" ] ||
[ -z "$SCALE6" ] || 
[ -z "$SCALE7" ]
then
  SCALE0="       "
  SCALE1="ǀ      "
  SCALE2="ǀǀ     "
  SCALE3="ǀǀǀ    "
  SCALE4="ǀǀǀǀ   "
  SCALE5="ǀǀǀǀǀ  "
  SCALE6="ǀǀǀǀǀǀ "
  SCALE7="ǀǀǀǀǀǀǀ"
fi

SCALE0="#[fg=green]${SCALE0:0:2}#[fg=yellow]${SCALE0:2:3}#[fg=red]${SCALE0:5}"
SCALE1="#[fg=green]${SCALE1:0:2}#[fg=yellow]${SCALE1:2:3}#[fg=red]${SCALE1:5}"
SCALE2="#[fg=green]${SCALE2:0:2}#[fg=yellow]${SCALE2:2:3}#[fg=red]${SCALE2:5}"
SCALE3="#[fg=green]${SCALE3:0:2}#[fg=yellow]${SCALE3:2:3}#[fg=red]${SCALE3:5}"
SCALE4="#[fg=green]${SCALE4:0:2}#[fg=yellow]${SCALE4:2:3}#[fg=red]${SCALE4:5}"
SCALE5="#[fg=green]${SCALE5:0:2}#[fg=yellow]${SCALE5:2:3}#[fg=red]${SCALE5:5}"
SCALE6="#[fg=green]${SCALE6:0:2}#[fg=yellow]${SCALE6:2:3}#[fg=red]${SCALE6:5}"
SCALE6="#[fg=green]${SCALE7:0:2}#[fg=yellow]${SCALE7:2:3}#[fg=red]${SCALE7:5}"

if [ -z "$SCALE_BOUND_L" ]; then
  SCALE_BOUND_L="["
fi

if [ -z "$SCALE_BOUND_R" ]; then
  SCALE_BOUND_R="]"
fi

default_scale_max=7
scale_max_options="@host-stats-max-scale"

main() {
  local interval="$(get_tmux_option "status-interval")"

  local stats="$($EXECUTABLE_PATH -a 1 -i $interval)"
  local stats_arr=($stats)

  local available_mem=${stats_arr[0]}
  local cpu_percentage=${stats_arr[1]}
  local load_average=${stats_arr[2]}

  local max_scale="$(get_tmux_option "$scale_max_options" "$default_scale_max")"
  local scale_value=$(bc <<< "(($cpu_percentage*($max_scale*.01))+0.5)/1")

  if (( $scale_value > $max_scale )); then
    scale_value=$max_scale
  fi
  local scale=SCALE${scale_value}

  echo "$available_mem $load_average $SCALE_BOUND_L${!scale}$SCALE_BOUND_R"
}

main
