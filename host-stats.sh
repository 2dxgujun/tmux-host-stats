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
  SCALE0="[       ]"
  SCALE1="[ǀ      ]"
  SCALE2="[ǀǀ     ]"
  SCALE3="[ǀǀǀ    ]"
  SCALE4="[ǀǀǀǀ   ]"
  SCALE5="[ǀǀǀǀǀ  ]"
  SCALE6="[ǀǀǀǀǀǀ ]"
  SCALE7="[ǀǀǀǀǀǀǀ]"
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

  echo "$available_mem $load_average ${!scale}"
}

main
