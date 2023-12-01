#!/bin/bash

# Find and kill submission_server
submission_server_pid=$(pgrep -f "./submission_server")
if [ -n "$submission_server_pid" ]; then
  echo "Killing submission_server (PID: $submission_server_pid)"
  kill -9 $submission_server_pid
else
  echo "submission_server not running"
fi

# Find and kill grader_server
grader_server_pid=$(pgrep -f "./grader_server")
if [ -n "$grader_server_pid" ]; then
  echo "Killing grader_server (PID: $grader_server_pid)"
  kill -9 $grader_server_pid
else
  echo "grader_server not running"
fi

# Find and kill query_server
query_server_pid=$(pgrep -f "./query_server")
if [ -n "$query_server_pid" ]; then
  echo "Killing query_server (PID: $query_server_pid)"
  kill -9 $query_server_pid
else
  echo "query_server not running"
fi