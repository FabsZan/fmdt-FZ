#!/bin/bash
set -x

WD=$(pwd)
build_root=build

if test -f "${build_root}/exe/fmdt-detect-rt-seq"; then
list_exe="fmdt-detect, fmdt-detect-rt-pip, fmdt-detect-rt-seq"
else
list_exe="fmdt-detect"
fi

cd scripts/regression
curl https://lip6.fr/adrien.cassagne/data/tauh/in/2022_05_31_tauh_34_meteors.mp4 --output 2022_05_31_tauh_34_meteors.mp4
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
curl https://lip6.fr/adrien.cassagne/data/fmdt/refs_2022_05_31_tauh_34_meteors_13ab8547.zip --output refs.zip
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
unzip refs.zip
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
rm refs/tracks.txt
rm refs/mag.txt

./compare.py --exe-args "--in-video ${WD}/scripts/regression/2022_05_31_tauh_34_meteors.mp4 --track-all" --list-exe "${list_exe}" --refs-path ${WD}/scripts/regression/refs
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

cd ${WD}