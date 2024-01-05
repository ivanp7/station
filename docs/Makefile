DOC_DIR="build"

.PHONY: all doxygen
all: doxygen
doxygen:
	mkdir -p "${DOC_DIR}"
	doxygen

.PHONY: clean clean-all
clean:
	rm -rf -- "${DOC_DIR}"
clean-all: clean

