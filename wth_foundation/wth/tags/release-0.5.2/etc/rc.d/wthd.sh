#!/bin/sh
#
# wthd startup script
# Volker Jahns, volker@thalreit.de
#

# PROVIDE: weather
# REQUIRE: LOGIN
# KEYWORD: shutdown

#
# Add the following lines to /etc/rc.conf to enable wthd:
#
#wthd_enable="YES"
#
#

. /etc/rc.subr

name=wthd
rcvar=`set_rcvar`

command=/usr/local/bin/wthd
pidfile=/var/run/wthd.pid
required_files=/usr/local/etc/wth/wth.conf

start_precmd=start_precmd
stop_postcmd=stop_postcmd

extra_commands="reload"

start_precmd()
{
#  case $sendmail_enable in
#  [Yy][Ee][Ss]|[Tt][Rr][Uu][Ee]|[Oo][Nn]|1)
#    warn "sendmail_enable should be set to NONE"
#    ;;
#  [Nn][Oo]|[Ff][Aa][Ll][Ss][Ee]|[Oo][Ff][Ff]|0)
#    case $sendmail_submit_enable in
#    [Yy][Ee][Ss]|[Tt][Rr][Uu][Ee]|[Oo][Nn]|1)
#      warn "sendmail_submit_enable should be set to NO"
#      ;;
#    esac
#    ;;
#  [Nn][Oo][Nn][Ee])
#    ;;
#  esac
}

stop_postcmd()
{
  rm -f $pidfile
}

# read settings, set default values
load_rc_config $name
: ${wthd_enable="NO"}
: ${wthd_flags="-d"}

run_rc_command "$1"
