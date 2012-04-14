MUTEX += dekker dekker_mfence dekker_lfence dekker_sfence dekker_sched
MUTEX += dekker_wiki dekker_mfence_wiki dekker_lfence_wiki dekker_sfence_wiki dekker_sched_wiki 
MUTEX += dekker_xchg dekker_atomic dekker_serial_atomic dekker_serial_nolock

#all :: $(MUTEX)

files := $(shell ls *.tex)
pdfs := $(files:.tex=.pdf)

all :: $(pdfs)

$(pdfs) : ../peeters_macros.tex

%.pdf : %.tex
	../make_pdflatex $<

clean ::
	rm -f *.ps *.dvi *.pdf *.aux *.log

clean ::
	rm -f *.o $(MUTEX)

dekker: dekker.c
	g++ dekker.c -o $@ -lpthread

dekker_atomic: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_ATOMIC

dekker_serial_atomic: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_SERIAL

dekker_serial_nolock: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_SERIAL -DNO_LOCK

dekker_xchg: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_XCHG_LOCK

dekker_mfence: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_MFENCE

dekker_sfence: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_SFENCE

dekker_lfence: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_LFENCE

dekker_sched: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_SCHED_FENCE



dekker_wiki: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_WIKI

dekker_mfence_wiki: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_MFENCE -DUSE_WIKI

dekker_sfence_wiki: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_SFENCE -DUSE_WIKI

dekker_lfence_wiki: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_LFENCE -DUSE_WIKI

dekker_sched_wiki: dekker.c
	g++ dekker.c -o $@ -lpthread -DUSE_SCHED_FENCE -DUSE_WIKI