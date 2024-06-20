TARGET:= cdb
TEST := tests
DEBUG := cdb_dbg

BUILDPATH := target
SRCS := $(wildcard src/*.c)


CFLAGS := -Og -g -Wall -Wextra -Wpedantic -Wno-unused-parameter -std=gnu17

.PHONY:	all clean

all: $(TARGET)
progs: $(PROGRAMS)

test : $(TEST)
$(TEST): $(SRCS) $(wildcard test/*.c)
	for file in $(wildcard test/*.c); do \
		$(CC) $(CFLAGS) $$file $(SRCS) -o $(BUILDPATH)/test_bin/$$(basename $$file .c); \
	done

$(TARGET): 
	$(MAKE) -C src/ $(BUILDPATH)/$(TARGET)

clean:

	rm -f $(BUILDPATH)/$(TARGET) $(BUILDPATH)/test_bin/*

.PHONY: clean
