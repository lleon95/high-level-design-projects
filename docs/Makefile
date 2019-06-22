PROJECT = hld
TEX = pdflatex
BIB = bibtex
FLAGS = --shell-escape


.PHONY = all okular open clean-all clean skim

$(PROJECT).pdf: $(PROJECT).tex
		$(TEX) $(FLAGS) $(PROJECT).tex


all:
		$(TEX) $(FLAGS) $(PROJECT).tex
		$(BIB) $(PROJECT)||true
		$(TEX) $(FLAGS) $(PROJECT).tex
		$(TEX) $(FLAGS) $(PROJECT).tex


okular:
		okular $(PROJECT).pdf

open:
		open $(PROJECT).pdf

skim:
		open -a Skim $(PROJECT).pdf
		defaults write -app Skim SKAutoReloadFileUpdate -boolean true

clean-all:
		rm -f *.dvi *.log *.bak *.aux *.bbl *.blg *.idx *.ps *.pdf *.toc *.out *~ *.synctex.gz

clean:
		rm -f *.log *.bak *.aux *.bbl *.blg *.idx *.toc *.out *~ *.synctex.gz

