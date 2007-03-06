PDFLATEX = /usr/local/bin/pdflatex
PDFLATEXFLAGS = --interaction errorstopmode

all: pdf

pdf: assignment1.pdf

%.pdf: %.tex
	$(PDFLATEX) $(PDFLATEXFLAGS) $<

clean:
	rm -f *.aux *.pdf *.log
