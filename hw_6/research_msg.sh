#!/bin/sh

user=$(whoami)
sys=$(uname)

Msg="Hello, $user! Welcome to $sys!"
echo "$Msg"
