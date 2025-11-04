#!/bin/bash

# export WAYLAND_DISPLAY=luma-0

# # Start compositor in background
# ./build/luma-compositor &

# COMPOSITOR_PID=$!

# # Wait until the Wayland socket exists
# while [ ! -S "/run/user/$UID/luma-0" ]; do
#     sleep 0.1
# done

# echo "Wayland compositor ready, launching clients..."

# # Launch terminals
# weston-terminal &
# weston-terminal &

# # Wait for compositor to exit
# wait $COMPOSITOR_PID


#!/bin/bash
set -e

# Build the compositor if needed
scons

# Clean up any old sockets
rm -f /run/user/$(id -u)/luma-* /run/user/$(id -u)/wayland-*

# Run compositor in background and capture its display socket name
echo "Starting Luma compositor..."
./build/luma-compositor &
COMP_PID=$!

# Wait for compositor to set WAYLAND_DISPLAY
sleep 1

# Export socket name for clients
export WAYLAND_DISPLAY=$(ls /run/user/$(id -u)/wayland-* | head -n 1 | xargs basename)
echo "Using display: $WAYLAND_DISPLAY"

# Launch a test client (e.g. Weston terminal)
weston-terminal &

# Wait for compositor to exit
wait $COMP_PID
