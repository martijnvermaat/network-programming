PDFLATEX = /usr/bin/pdflatex
PDFLATEXFLAGS = --interaction errorstopmode

all: pdf

pdf: assignment3.pdf

%.pdf: %.tex
	$(PDFLATEX) $(PDFLATEXFLAGS) $<

clean:
	rm -f *.aux *.pdf *.log
