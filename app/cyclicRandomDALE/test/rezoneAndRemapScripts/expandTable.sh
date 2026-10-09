#!/bin/bash

# Expand a study definition (test_table.conf) into the legacy parallel arrays
# read by Test.sh, plot.sh and study.sh (test_table.dat format).
#
# Usage: ./expandTable.sh test.conf > test_table.dat

if [ $# -ne 1 ] || [ ! -f "$1" ]; then
    echo "Usage: $0 <study.conf>" >&2
    exit 1
fi

# Map conf keys to legacy array names
declare -A legacyName=(
    [M]=M
    [gradX]=nGradX
    [gradY]=nGradY
    [ddt]=ddtScheme
    [grad1]=grad1Scheme
    [grad2]=grad2Scheme
    [meshPhi]=meshPhiScheme
    [alpha]=alpha
    [nRemap]=nRemap
    [endTime]=caseEndTimes
    [deltaT]=caseTimeSteps
    [writeInterval]=caseWriteIntervals
)

# Per-case keys that may be left unset
declare -A optionalKey=([alpha]=1 [nRemap]=1 [endTime]=1 [deltaT]=1 [writeInterval]=1)

declare -a studyNames=() studyEndTimes=() studyTimeSteps=() studyWriteIntervals=()
declare -a studyNRemaps=()
declare -A defaultVal=()

# Expansion units: a single axis, or a bundle of axes that advance together
# (entry j of every member forms one combination). Units are permuted with
# each other, the first unit varying slowest.
declare -a units=()       # unit ids; members in unit_<id> arrays
declare -A unitOfKey=()   # key -> unit id
inBundle=""               # id of the open bundle, if any

# Cyclic mode: on/off
cyclic=off

# Rezone mode, i.e. the Test.sh argument
mode=""

# Keyword helpers used by the conf file

study()
{
    local name=$1; shift
    local endTime="" deltaT="" writeInterval="" nRemap="" kv

    for kv in "$@"; do
        case ${kv%%=*} in
            endTime)       endTime=${kv#*=} ;;
            deltaT)        deltaT=${kv#*=} ;;
            writeInterval) writeInterval=${kv#*=} ;;
            nRemap)        nRemap=${kv#*=} ;;
            *) echo "Error: unknown study key '${kv%%=*}'" >&2; exit 1 ;;
        esac
    done

    # endTime, deltaT, writeInterval may instead be given per case (fixed
    # axis or defaults); completeness is checked after reading the conf
    studyNames+=("$name")
    studyEndTimes+=("$endTime")
    studyTimeSteps+=("$deltaT")
    studyWriteIntervals+=("$writeInterval")

    # nRemap is optional, but must be given for all studies or none
    if [ -n "$nRemap" ] && [ ${#studyNRemaps[@]} -ne $((${#studyNames[@]} - 1)) ] \
    || [ -z "$nRemap" ] && [ ${#studyNRemaps[@]} -ne 0 ]; then
        echo "Error: nRemap must be set for all studies or none (study $name)" >&2
        exit 1
    fi
    [ -n "$nRemap" ] && studyNRemaps+=("$nRemap")
}

cyclic()
{
    case $1 in
        on|off) cyclic=$1 ;;
        *) echo "Error: cyclic must be 'on' or 'off', not '$1'" >&2; exit 1 ;;
    esac
}

mode()
{
    case $1 in
        sinOnce|once|cyclic|random|interpolate) mode=$1 ;;
        *) echo "Error: unknown mode '$1';" \
                "valid: sinOnce once cyclic random interpolate" >&2
           exit 1 ;;
    esac
}

bundleBegin()
{
    if [ -n "$inBundle" ]; then
        echo "Error: nested bundle{" >&2
        exit 1
    fi
    inBundle=${#units[@]}
    units+=("$inBundle")
    declare -g -a "unit_$inBundle=()"
}

bundleEnd()
{
    if [ -z "$inBundle" ]; then
        echo "Error: '}' without bundle{" >&2
        exit 1
    fi
    local -n members="unit_$inBundle"
    if [ ${#members[@]} -eq 0 ]; then
        echo "Error: empty bundle" >&2
        exit 1
    fi
    inBundle=""
}

checkKey()
{
    if [ "$1" != mesh ] && [ -z "${legacyName[$1]}" ]; then
        echo "Error: unknown key '$1'; valid: mesh ${!legacyName[*]}" >&2
        exit 1
    fi
}

defaults()
{
    local kv
    for kv in "$@"; do
        checkKey "${kv%%=*}"
        defaultVal[${kv%%=*}]=${kv#*=}
    done
}

axis()
{
    local key=$1; shift
    checkKey "$key"

    if [ $# -eq 0 ]; then
        echo "Error: axis $key has no values" >&2
        exit 1
    fi

    if [ -n "${unitOfKey[$key]}" ]; then
        echo "Error: axis $key is given more than once" >&2
        exit 1
    fi

    declare -g -a "axis_$key"
    local -n vals="axis_$key"
    vals=("$@")

    local id=$inBundle
    if [ -z "$id" ]; then
        id=${#units[@]}
        units+=("$id")
        declare -g -a "unit_$id=()"
    fi

    local -n members="unit_$id"
    if [ ${#members[@]} -gt 0 ]; then
        local -n first="axis_${members[0]}"
        if [ ${#first[@]} -ne $# ]; then
            echo "Error: bundle axes ${members[0]} and $key differ in" \
                 "length (${#first[@]} vs $#)" >&2
            exit 1
        fi
    fi
    members+=("$key")
    unitOfKey[$key]=$id
}

# Unknown keywords in the conf (e.g. a misspelt bundle{) abort the expansion
command_not_found_handle()
{
    echo "Error: unknown keyword '$1' in conf" >&2
    kill -TERM $$
}
trap 'exit 1' TERM

# bundle{ and } lines are not valid bash; turn them into helper calls
source <(sed -E \
    -e 's/^([[:space:]]*)bundle[[:space:]]*\{[[:space:]]*(#.*)?$/\1bundleBegin/' \
    -e 's/^([[:space:]]*)\}[[:space:]]*(#.*)?$/\1bundleEnd/' \
    "$1")

if [ -n "$inBundle" ]; then
    echo "Error: bundle{ without closing }" >&2
    exit 1
fi

if [ ${#studyNames[@]} -eq 0 ]; then
    echo "Error: no study defined" >&2
    exit 1
fi

# Build cases as the cartesian product of the units; the first unit varies
# slowest

declare -a nX=() nY=() caseNameSuffix=()
for key in "${!legacyName[@]}"; do
    declare -a "${legacyName[$key]}=()"
done

# Current combination of axis values while expanding
declare -A current=()

append()
{
    local -n target=$1
    target+=("$2")
}

addCase()
{
    local key value suffix="" mesh

    mesh=${current[mesh]:-${defaultVal[mesh]}}
    if [[ ! $mesh =~ ^[0-9]+x[0-9]+$ ]]; then
        echo "Error: mesh '$mesh' must be <nX>x<nY>" >&2
        exit 1
    fi
    nX+=("${mesh%x*}")
    nY+=("${mesh#*x}")

    for key in "${!legacyName[@]}"; do
        value=${current[$key]-${defaultVal[$key]}}
        if [ -z "$value" ]; then
            [ -n "${optionalKey[$key]}" ] && continue
            echo "Error: no value for '$key'; set it in defaults or an axis" >&2
            exit 1
        fi
        append "${legacyName[$key]}" "$value"
    done

    # Case name suffix: per unit not containing the mesh, the value of its
    # first axis
    local id
    for id in "${units[@]}"; do
        local -n members="unit_$id"
        [ "${unitOfKey[mesh]}" = "$id" ] && { unset -n members; continue; }
        value=${current[${members[0]}]}
        suffix+=${suffix:+_}${value// /_}
        unset -n members
    done
    caseNameSuffix+=("${suffix:-case}")
}

expand()
{
    local level=$1 j key

    if [ "$level" -eq ${#units[@]} ]; then
        addCase
        return
    fi

    local -n members="unit_${units[level]}"
    local -n firstVals="axis_${members[0]}"
    for j in "${!firstVals[@]}"; do
        for key in "${members[@]}"; do
            local -n vals="axis_$key"
            current[$key]=${vals[j]}
            unset -n vals
        done
        expand $((level+1))
    done
}

expand 0

# Case names must be unique
declare -A seen=()
for i in "${!nX[@]}"; do
    name="${nX[i]}x${nY[i]}_${caseNameSuffix[i]}"
    if [ -n "${seen[$name]}" ]; then
        echo "Error: duplicate case name $name; the first axis of each" \
             "bundle names the cases, so its values must differ" >&2
        exit 1
    fi
    seen[$name]=1
done

# Time controls: cyclic takes them from M and nRemap; otherwise each must be
# given per case or on every study line
if [ "$cyclic" != on ]; then
    for key in endTime deltaT writeInterval; do
        declare -n perCase=${legacyName[$key]}
        declare -n perStudy=study${key^}s
        [ "$key" = deltaT ] && declare -n perStudy=studyTimeSteps
        if [ ${#perCase[@]} -eq 0 ]; then
            for v in "${perStudy[@]}"; do
                if [ -z "$v" ]; then
                    echo "Error: $key must be set per case or on every" \
                         "study line" >&2
                    exit 1
                fi
            done
        fi
        unset -n perCase perStudy
    done
fi

# Cyclic cases take endTime from M and writeInterval from nRemap
if [ "$cyclic" = on ] && [ ${#nRemap[@]} -eq 0 ]; then
    echo "Error: cyclic on needs a per-case nRemap (defaults or axis)" >&2
    exit 1
fi

# Write in the legacy format

echo "# Generated by expandTable.sh from $1 -- edit that file instead"
echo
printf 'description=%q\n' "$description"
echo
echo "cyclic=$cyclic"
echo "mode=$mode"
echo
for arr in studyTimeSteps studyEndTimes studyWriteIntervals studyNRemaps studyNames \
           M nX nY nGradX nGradY \
           ddtScheme grad1Scheme grad2Scheme meshPhiScheme caseNameSuffix \
           alpha nRemap caseEndTimes caseTimeSteps caseWriteIntervals
do
    local_ref="$arr[@]"
    # Optional arrays: skip them when nothing sets them
    case $arr in alpha|nRemap|studyNRemaps|case*)
        declare -n ref=$arr
        [ ${#ref[@]} -eq 0 ] && { unset -n ref; continue; }
        unset -n ref ;;
    esac
    printf 'declare -a %s=(' "$arr"
    printf ' %q' "${!local_ref}"
    printf ' )\n'
done
