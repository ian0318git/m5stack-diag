#!/bin/bash

#####################help utils##########################
function trim() {
    shopt -s extglob
    local a="${1/%*( )/}"
    echo "'${a/#*( )/}'"
    shopt -u extglob
}

function uniq_words() {
    echo "$@" |awk '{for(i=1; i<=NF; i++) printf("%s\n", $i);}' |sort -u |xargs
}

function timestr() {
    TZ=Asia/Shanghai date +%y.%m.%d_%H.%M.%S
}
#########################################################

function usage() {
cat << _USAGE
$0 [-nc] [all] [curie1] [curie2] [tabeil] [promth]
_USAGE
}

function chk_param() {
    local args="$@"

    [[ $# -eq 0 ]] && return 0

    args="${args//curie1/}"
    args="${args//curie2/}"
    args="${args//tabeil/}"
    args="${args//promth/}"
    args="${args//all/}"
    args="${args//-nc/}"
    args="${args// /}"

    [[ x$args != x ]] && return 2
    return 0
}

function build_curie1() {
    [[ $do_clean -eq 1 ]] &&  \
    make -C  ${switzer_dir}/../overlord/neptune/curie_1RU clean

    ( cd ${switzer_dir}/../overlord/neptune/curie_1RU && make -j ) &&  \
    cp -fs ${switzer_dir}/../overlord/neptune/curie_1RU/curie1ru_diag.bz2 ${switzer_dir}/ && \
    cp -fs ${switzer_dir}/../overlord/neptune/curie_1RU/curie1ru_diag.bz2 ${switzer_dir}/curie1ru_diag-$(timestr).bz2
}

function build_curie2() {
    [[ $do_clean -eq 1 ]] &&  \
    make -C  ${switzer_dir}/../overlord/neptune/curie_2RU clean

    ( cd ${switzer_dir}/../overlord/neptune/curie_2RU && make -j ) &&  \
    cp -fs ${switzer_dir}/../overlord/neptune/curie_2RU/curie2ru_diag.bz2 ${switzer_dir}/ && \
    cp -fs ${switzer_dir}/../overlord/neptune/curie_2RU/curie2ru_diag.bz2 ${switzer_dir}/curie2ru_diag-$(timestr).bz2
}

function build_tabeiL() {
    [[ $do_clean -eq 1 ]] &&  \
    make -C  ${switzer_dir}/../tabei-l clean

    ( cd ${switzer_dir}/../tabei-l && make -j ) && \
    cp -fs ${switzer_dir}/../tabei-l/tabeil_prome_lnx ${switzer_dir}/ && \
    cp -fs ${switzer_dir}/../tabei-l/tabeil_prome_lnx ${switzer_dir}/tabeil_prome_lnx-$(timestr)
}

function build_all() {
   build_curie1 && \
   build_curie2 && \
   build_tabeiL
}

function has_key() {
    local key=$1
    shift
    for a in $@; do
        [[ "$key" == "$a" ]] && return 0
    done
    return 1
}

function has_key() {
    local key=$1
    shift
    for a in $@; do
        [[ "$key" == "$a" ]] && return 0
    done
    return 1
}

function main() {
    local args="$@"
    has_key "-nc" $args && { do_clean=0; args=${args//-nc/}; }
    chk_param $args || return 1

    args=$(uniq_words ${args//promth/tabeil}) #promth and tabeiL use same source dir
    [[ x$args == "x" ]] && args=all

    has_key "all"    $args && { build_all    ;  return $? ; }
    has_key "curie1" $args && { build_curie1 || return $? ; }
    has_key "curie2" $args && { build_curie2 || return $? ; }
    has_key "tabeil" $args && { build_tabeiL || return $? ; }

    return 0
}

do_clean=1
switzer_dir=$(dirname $0)
main $@ || usage
