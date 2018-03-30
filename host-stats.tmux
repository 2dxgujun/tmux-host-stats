#!/usr/bin/env bash

set -e

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

EXECUTABLE_PATH="$CURRENT_DIR/build/tmux-host-stats"

make_executable() {
	pushd $CURRENT_DIR

	if output=$(cmake -H. -Bbuild 2>&1); then tmux run-shell "echo \"'cmake $CURRENT_DIR' failed.
	$output
	\""; else exit 1; fi

	if output=$(cd build && make 2>&1); then tmux run-shell "echo \"tmux-host-stats failed to build.
	$output
	\""; else exit 1; fi

	popd
}

main() {
	if ! type "$EXECUTABLE_PATH" > /dev/null; then
	  make_executable
	fi
}
main