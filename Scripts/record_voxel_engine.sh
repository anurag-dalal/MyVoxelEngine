#!/bin/bash

WINDOW_NAME="MyVoxelEngine"
OUTPUT_FILE="Vids/voxel_recording_$(date +%Y%m%d_%H%M%S).mp4"
DISPLAY_NUM=${DISPLAY:-:0.0}

echo "🔍 Searching for window: $WINDOW_NAME"

WINDOW_ID=$(xdotool search --name "$WINDOW_NAME" | head -n 1)

if [ -z "$WINDOW_ID" ]; then
  echo "Window '$WINDOW_NAME' not found."
  exit 1
fi

echo "✅ Found window ID: $WINDOW_ID"

WININFO=$(xwininfo -id $WINDOW_ID)

WIDTH=$(echo "$WININFO" | grep "Width" | awk '{print $2}')
HEIGHT=$(echo "$WININFO" | grep "Height" | awk '{print $2}')
X=$(echo "$WININFO" | grep "Absolute upper-left X" | awk '{print $4}')
Y=$(echo "$WININFO" | grep "Absolute upper-left Y" | awk '{print $4}')

echo "📐 Capturing area: ${WIDTH}x${HEIGHT} at (${X},${Y})"

echo "🎥 Starting ffmpeg recording..."

ffmpeg -video_size ${WIDTH}x${HEIGHT} -framerate 60 -f x11grab -i ${DISPLAY_NUM}+${X},${Y} \
-c:v libx264 -preset ultrafast -crf 40 "$OUTPUT_FILE"

echo "✅ Recording complete: $OUTPUT_FILE"
