all:
	mkdir -p bin/macos/x64
	make -f Makefile.apple64
	cd src/artifacts; zip -FSr ../../bin/package.zip *; cd ../../bin; zip -r package.zip macos

clean:
	make -f Makefile.apple64 clean

