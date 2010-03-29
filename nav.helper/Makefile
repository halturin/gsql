DEFS='pkg-config --variable=defsdir pygtk-2.0'
CFLAGS=-fPIC -I/usr/include/python2.6 -I/usr/include/pygtk-2.0 -I.. `pkg-config --cflags glib-2.0 gtk+-2.0`
LDFLAGS=`pkg-config --libs gtk+-2.0 pygtk-2.0`

all: gsql.c gsql.so

gsql.defs: navtree.h Makefile
	python /usr/share/pygobject/2.0/codegen/h2def.py navtree.h > gsql.defs
	patch -p0 < gsql.defs.patch

gsql.c: gsql.defs gsql.override
	pygobject-codegen-2.0 --prefix gsql_navtree \
	--override gsql.override \
	gsql.defs > $@

gsql.so: gsql.o navtree.o gsqlmodule.o
	$(CC) $(LDFLAGS) -shared $^ -o $@

clean:
	rm *.so
	rm *.o
	rm *.pyc
