BLDDIR = build
TARGET = trust
$(shell mkdir -p $(BLDDIR))
DEPFLAGS = -MT $@ -MMD -MP -MF $(BLDDIR)/$*.Td
CFLAGS += -Wall -pedantic -O2 -march=native -mtune=native
LDFLAGS +=

.PHONY: all clean

SRCS=automaton.c main.c

OBJS=$(patsubst %, $(BLDDIR)/%.o, $(basename $(SRCS)))

all: $(TARGET)

$(BLDDIR):
	mkdir $(BLDDIR)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(BLDDIR)/%.o: src/%.c $(BLDDIR)/%.d | $(BLDDIR)
	$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ -c $<
	mv -f $(BLDDIR)/$*.Td $(BLDDIR)/$*.d

$(BLDDIR)/%.d: $(BLDDIR) ;
.PRECIOUS: $(BLDDIR)/%.d

include $(wildcard $(patsubst %, $(BLDDIR)/%.d, $(basename $(SRCS))))

clean:
	rm -f $(BLDDIR)/*.o $(BLDDIR)/*.d
	rmdir $(BLDDIR)