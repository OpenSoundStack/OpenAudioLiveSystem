#!/usr/bin/env bash
# Launch the three-process OALS dev stack (sim_switch + OALSEngine + io_simulator)
# in a tmux session. Idempotent: kills any previous session, removes the socket,
# re-runs. Per-pane logs land in /tmp/osst-dev-<role>.log.
#
# Usage:
#   scripts/dev-up.sh                     # default: sim:default, io_sim.example.json
#   scripts/dev-up.sh --name foo          # use sim:foo (separate socket)
#   scripts/dev-up.sh --config path.json  # io_sim config
#   scripts/dev-up.sh --inspect           # launch oaninspect as a 4th pane
#   scripts/dev-up.sh --build-dir build   # default ./build
#   scripts/dev-up.sh --down              # kill the session and exit

set -euo pipefail

NAME="default"
CONFIG=""
BUILD_DIR="build"
ATTACH=1
INSPECT=0
DOWN=0
SESSION="osst-dev"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --name)       NAME="$2"; shift 2 ;;
        --config)     CONFIG="$2"; shift 2 ;;
        --build-dir)  BUILD_DIR="$2"; shift 2 ;;
        --inspect)    INSPECT=1; shift ;;
        --no-attach)  ATTACH=0; shift ;;
        --down)       DOWN=1; shift ;;
        -h|--help)
            sed -n '2,/^set -euo/p' "$0" | sed 's/^# //; s/^#//'
            exit 0 ;;
        *) echo "unknown arg: $1" >&2; exit 2 ;;
    esac
done

cd "$(dirname "$0")/.."
REPO_ROOT="$(pwd)"

SOCK="/tmp/osst-sim-${NAME}.sock"
[[ -z "$CONFIG" ]] && CONFIG="$REPO_ROOT/io_sim/io_sim.example.json"

if ! command -v tmux >/dev/null; then
    echo "dev-up: tmux is required (brew install tmux)" >&2
    exit 1
fi

# Always start clean: kill the previous session and stray binaries from a
# crashed run. Leaving a previous OALSEngine alive would silently bind to
# our socket name and the new one would log "connect failed".
tmux kill-session -t "$SESSION" 2>/dev/null || true
pkill -x sim_switch     2>/dev/null || true
pkill -x OALSEngine     2>/dev/null || true
pkill -x io_simulator   2>/dev/null || true
pkill -x oaninspect     2>/dev/null || true
rm -f "$SOCK"

if [[ "$DOWN" == "1" ]]; then
    echo "dev-up: stack down (session ${SESSION} killed, socket removed)"
    exit 0
fi

SWITCH_BIN="$REPO_ROOT/$BUILD_DIR/tools/sim_switch/sim_switch"
ENGINE_BIN="$REPO_ROOT/$BUILD_DIR/engine/OALSEngine"
IOSIM_BIN="$REPO_ROOT/$BUILD_DIR/io_sim/io_simulator"
INSPECT_BIN="$REPO_ROOT/$BUILD_DIR/tools/oaninspect/oaninspect"

for bin in "$SWITCH_BIN" "$ENGINE_BIN" "$IOSIM_BIN"; do
    if [[ ! -x "$bin" ]]; then
        echo "dev-up: missing $bin — run 'cmake --build $BUILD_DIR' first" >&2
        exit 1
    fi
done

# Per-pane stdout/stderr also gets tee'd to a file so you can grep later.
LOG_SWITCH="/tmp/osst-dev-switch.log"
LOG_ENGINE="/tmp/osst-dev-engine.log"
LOG_IOSIM="/tmp/osst-dev-iosim.log"
LOG_INSPECT="/tmp/osst-dev-inspect.log"
: > "$LOG_SWITCH" "$LOG_ENGINE" "$LOG_IOSIM" "$LOG_INSPECT"

CMD_SWITCH="$SWITCH_BIN --socket-path $SOCK 2>&1 | tee $LOG_SWITCH"
CMD_ENGINE="sleep 0.5 && $ENGINE_BIN sim:$NAME 2>&1 | tee $LOG_ENGINE"
CMD_IOSIM="sleep 1.0 && $IOSIM_BIN sim:$NAME $CONFIG 2>&1 | tee $LOG_IOSIM"
CMD_INSPECT="sleep 1.5 && $INSPECT_BIN --socket-path $SOCK --filter ethertype=disco,control,sync 2>&1 | tee $LOG_INSPECT"

tmux new-session  -d -s "$SESSION" -n stack "$CMD_SWITCH"
tmux split-window -t "$SESSION:stack" -v "$CMD_ENGINE"
tmux split-window -t "$SESSION:stack" -h "$CMD_IOSIM"

if [[ "$INSPECT" == "1" ]]; then
    tmux split-window -t "$SESSION:stack.0" -h "$CMD_INSPECT"
fi

tmux select-layout -t "$SESSION:stack" tiled
tmux set-option   -t "$SESSION" mouse on

echo "dev-up: session '$SESSION' running on socket $SOCK"
echo "  switch: $LOG_SWITCH"
echo "  engine: $LOG_ENGINE"
echo "  iosim:  $LOG_IOSIM"
[[ "$INSPECT" == "1" ]] && echo "  inspect: $LOG_INSPECT"
echo "  detach: Ctrl-b d        kill: scripts/dev-up.sh --down"

if [[ "$ATTACH" == "1" ]]; then
    tmux attach -t "$SESSION"
fi
