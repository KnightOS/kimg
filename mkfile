</$objtype/mkfile
name=kimg

$name: main.c
	pcc -B -o $target $prereq

install: $name
	mkdir -p /amd64/bin/knightos/
	cp $name /amd64/bin/knightos/
