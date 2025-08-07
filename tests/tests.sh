#!/bin/bash

# Config
HOST="localhost"
PORT="6666"

# Commands to send (edit these as needed)
COMMANDS=$(
  cat <<EOF
NICK testUser
USER testUser 0 * :Real Name
JOIN #channel0
PRIVMSG #channel0 :Hello everyone!
TOPIC #channel0 :Testing topic!
EOF
)

# Send commands to IRC server with proper \r\n line endings
printf '%s\r\n' $COMMANDS | nc "$HOST" "$PORT"
