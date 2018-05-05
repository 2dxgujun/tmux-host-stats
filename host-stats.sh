#!/bin/bash -e

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

EXECUTABLE_PATH="$CURRENT_DIR/build/tmux-host-stats"

source "$CURRENT_DIR/helpers.sh"

SCALE_MAX_DEFAULT=7
SCALE_CHAR_DEFAULT="ǀ"

SCALE_MAX="$(get_tmux_option "@host-stats-scale-max" "$SCALE_MAX_DEFAULT")"
SCALE_CHAR="$(get_tmux_option "@host-stats-scale-char" "$SCALE_CHAR_DEFAULT")"
SCALE_LEFT="$(get_tmux_option "@host-stats-scale-left" "[")"
SCALE_RIGHT="$(get_tmux_option "@host-stats-scale-right" "]")"

for (( i=0; i<=$SCALE_MAX; i++)); do
  n1=$(printf "%*s" $i "")
  n2=$(printf "%*s" $(bc <<< "$SCALE_MAX-$i"))
  declare "SCALE$i=${n1// /$SCALE_CHAR}${n2// /" "}"
  n3=$(bc <<< "(($SCALE_MAX*.333)+.5)/1")
  n4=$(bc <<< "$SCALE_MAX/2")
  n5=$(bc <<< "$n3+$n4")
  scale="SCALE$i"
  declare "SCALE$i=#[fg=green]${!scale:0:$n3}#[fg=yellow]${!scale:$n3:$n4}#[fg=red]${!scale:$n5}"
  declare "SCALE$i=#[fg=default]$SCALE_LEFT${!scale}#[fg=default]$SCALE_RIGHT"
done

main() {
  local status_interval="$(get_tmux_option "status-interval")"

  local stats="$($EXECUTABLE_PATH -a 1 -i $status_interval)"
  local stats_arr=($stats)

  local available_mem=${stats_arr[0]}
  local cpu_percentage=${stats_arr[1]}
  local load_average=${stats_arr[2]}

  local scale=$(bc <<< "(($cpu_percentage*($SCALE_MAX*.01))+0.5)/1")

  if (( $scale > $SCALE_MAX )); then
    scale="$SCALE_MAX"
  fi
  local scale_result="SCALE${scale}"
  echo "$available_mem $load_average ${!scale_result}"

  #for (( i=0; i<=$SCALE_MAX; i++ )); do
  #  local scale="SCALE$i"
  #  echo "${!scale}"
  #done
}

main
