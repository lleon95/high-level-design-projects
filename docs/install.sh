#!/bin/bash

if test `whoami` != "root"
then
  echo "  Error: You need root priviledges to run this script."
  exit $FAILURE
fi

apt-get install \
    texlive \
    texlive-binaries \
    texlive-extra-utils \
    texlive-fonts-recommended \
    texlive-font-utils \
    texlive-formats-extra \
    texlive-generic-recommended \
    texlive-lang-spanish \
    texlive-latex-base \
    texlive-latex-extra \
    texlive-latex-recommended \
    texlive-plain-extra \
    texlive-pstricks \
    texlive-science \
    texlive-bibtex-extra \
    texlive-publishers \
    xfig \
    transfig \
    imagemagick \
    ps2eps \
    bibutils \
    gnuplot \
    octave
