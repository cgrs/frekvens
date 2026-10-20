#!/bin/sh
# Usage: scripts/with-config.sh <obegransad|frekvens> <command...>
#
# The build reads the device from .env and the pins from firmware/include/config/secrets.h, and both files are
# local (.env is gitignored, secrets.h is a tracked template). This appends the device and pins for the run and
# restores both files afterwards. A missing .env starts from .env.example and is removed again.
#
# Pins default to the wiring in secrets.h and can be overridden with PIN_CS, PIN_SCLK, PIN_MOSI, PIN_OE, PIN_SW1,
# PIN_SW2 and PIN_MIC. WIFI_SSID and WIFI_KEY can be in .env, or in .env.local (mise loads it); without them a build uses
# placeholders, and WIFI_REQUIRED=1 (the upload tasks) refuses to run.
set -eu

device=${1:?usage: with-config.sh <obegransad|frekvens> <command...>}
shift
case $device in
obegransad)
    option=IKEA_OBEGRANSAD
    pins="PIN_CS ${PIN_CS:-1}
PIN_SCLK ${PIN_SCLK:-2}
PIN_MOSI ${PIN_MOSI:-3}
PIN_OE ${PIN_OE:-4}
PIN_SW2 ${PIN_SW2:-5}"
    ;;
frekvens)
    option=IKEA_FREKVENS
    pins="PIN_CS ${PIN_CS:-1}
PIN_SCLK ${PIN_SCLK:-2}
PIN_MOSI ${PIN_MOSI:-3}
PIN_OE ${PIN_OE:-4}
PIN_SW1 ${PIN_SW1:-5}
PIN_SW2 ${PIN_SW2:-6}"
    [ -z "${PIN_MIC:-}" ] || pins="$pins
PIN_MIC $PIN_MIC"
    ;;
*)
    echo "unknown device: $device (obegransad or frekvens)" >&2
    exit 2
    ;;
esac

# Credentials in .env already become -D defines (every key there does), so leave Wi-Fi alone in that case.
if grep -qs '^WIFI_SSID=' .env && grep -qs '^WIFI_KEY=' .env; then
    write_wifi=
elif [ -z "${WIFI_SSID:-}" ] || [ -z "${WIFI_KEY:-}" ]; then
    if [ -n "${WIFI_REQUIRED:-}" ]; then
        echo "set WIFI_SSID and WIFI_KEY in .env or .env.local before uploading" >&2
        exit 2
    fi
    echo "WIFI_SSID or WIFI_KEY not set, using placeholders (fine for a build, not for flashing)" >&2
    WIFI_SSID=${WIFI_SSID:-redacted}
    WIFI_KEY=${WIFI_KEY:-redacted}
    write_wifi=1
else
    write_wifi=1
fi

env_file=.env
secrets=firmware/include/config/secrets.h
backup=$(mktemp -d)
if [ -f "$env_file" ]; then
    cp "$env_file" "$backup/env"
else
    cp .env.example "$env_file" # a missing .env starts from the template, which enables the default modes
fi
cp "$secrets" "$backup/secrets"
trap 'if [ -f "$backup/env" ]; then cp "$backup/env" "$env_file"; else rm -f "$env_file"; fi; cp "$backup/secrets" "$secrets"; rm -rf "$backup"' EXIT INT TERM

printf "%s='true'\n" "$option" >>"$env_file"
{
    echo
    echo "$pins" | sed 's/^/#define /'
    [ -z "$write_wifi" ] || printf '#define WIFI_SSID "%s"\n' "$WIFI_SSID"
    [ -z "$write_wifi" ] || printf '#define WIFI_KEY "%s"\n' "$WIFI_KEY"
} >>"$secrets"

"$@"
