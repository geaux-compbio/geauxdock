#!/bin/sh -x

MAKE="gmake -B -j10"
ARCH=gpuk20
#ARCH=gpu780
#ARCH=gpu780x2
#ARCH=gpu980

${MAKE} MARCRO_MAKE="-DIS_NO_BRANCH=1 -DCALC_PRT=1 -DCALC_KDE=1 -DCALC_MCS=1 -DCALC_DST=1" EXE=${ARCH}_nobranch_full
${MAKE} MARCRO_MAKE="-DIS_NO_BRANCH=1 -DCALC_PRT=0 -DCALC_KDE=0 -DCALC_MCS=0 -DCALC_DST=0" EXE=${ARCH}_nobranch_none
${MAKE} MARCRO_MAKE="-DIS_NO_BRANCH=1 -DCALC_PRT=1 -DCALC_KDE=0 -DCALC_MCS=0 -DCALC_DST=0" EXE=${ARCH}_nobranch_prt
${MAKE} MARCRO_MAKE="-DIS_NO_BRANCH=1 -DCALC_PRT=0 -DCALC_KDE=1 -DCALC_MCS=0 -DCALC_DST=0" EXE=${ARCH}_nobranch_kde
${MAKE} MARCRO_MAKE="-DIS_NO_BRANCH=1 -DCALC_PRT=0 -DCALC_KDE=0 -DCALC_MCS=1 -DCALC_DST=0" EXE=${ARCH}_nobranch_mcs

${MAKE} MARCRO_MAKE="-DIS_NO_BRANCH=0 -DCALC_PRT=1 -DCALC_KDE=1 -DCALC_MCS=1 -DCALC_DST=1" EXE=${ARCH}_branch_full
${MAKE} MARCRO_MAKE="-DIS_NO_BRANCH=0 -DCALC_PRT=0 -DCALC_KDE=0 -DCALC_MCS=0 -DCALC_DST=0" EXE=${ARCH}_branch_none
${MAKE} MARCRO_MAKE="-DIS_NO_BRANCH=0 -DCALC_PRT=1 -DCALC_KDE=0 -DCALC_MCS=0 -DCALC_DST=0" EXE=${ARCH}_branch_prt
${MAKE} MARCRO_MAKE="-DIS_NO_BRANCH=0 -DCALC_PRT=0 -DCALC_KDE=1 -DCALC_MCS=0 -DCALC_DST=0" EXE=${ARCH}_branch_kde
${MAKE} MARCRO_MAKE="-DIS_NO_BRANCH=0 -DCALC_PRT=0 -DCALC_KDE=0 -DCALC_MCS=1 -DCALC_DST=0" EXE=${ARCH}_branch_mcs







