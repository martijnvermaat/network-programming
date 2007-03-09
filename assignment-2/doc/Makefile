PDFLATEX = /usr/local/bin/pdflatex
PDFLATEXFLAGS = --interaction errorstopmode

all: pdf

pdf: assignment2.pdf

%.pdf: %.tex
	$(PDFLATEX) $(PDFLATEXFLAGS) $<

clean:
	rm -f *.aux *.pdf *.log
