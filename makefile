########### Project Environment ###########

PROJECTPATH   = $(shell pwd)
BINOUT        = $(shell basename $(PROJECTPATH))
VERSION       = $(shell $(PROJECTPATH)/version.sh)
VERSIONSHORT  = $(shell $(PROJECTPATH)/version.sh --short)
ARCHITECTURE  = $(shell dpkg --print-architecture)

CC            = g++ -std=c++17

DEBUGFLAGS    = -g -O0 -DDEBUG
RELEASEFLAGS  = -O3 -DNDEBUG

########### Project Directories ###########

SRCDIR        = src/
BINDIR        = build/
BUILDPATH     = $(BINDIR)$(ARCHITECTURE)/
INSTALLDIR    = /usr/lib/
HEADERDIR     = /usr/include/


SOURCES       = $(shell find -L $(SRCDIR) -name '*.cpp')
DIRSTRUCTURE  = $(shell find -L $(SRCDIR) -type d)
INCSTRUCTURE  = $(patsubst %, -I%, $(DIRSTRUCTURE))
LOG_LEVEL     = ALL

LINK_DEPS          = -lcrypto -lcrypto -lPocoNetSSL -lPocoNet -lPocoUtil -lPocoFoundation -lPocoCrypto -lPocoData -lpqxx -lpq
INSTALL_DEPS       = libssl-dev libpoco-dev libpqxx-dev libpq-dev

###############################################################################
# 1. Choose mode once (dbg|rel)
###############################################################################
MODE ?= rel                     # default when user just runs `make`

ifeq ($(MODE),dbg)
  	CXXFLAGS += -g -O0 -DDEBUG
	BUILDROOT   := $(BUILDPATH)dbg
else
  	CXXFLAGS += -O3 -DNDEBUG
	BUILDROOT   := $(BUILDPATH)rel
endif

###############################################################################
# 2. Derived paths
###############################################################################
OBJDIR      := $(BUILDROOT)/obj
TARGET      := $(OUTDIR)/$(APP)

OBJS        := $(patsubst $(SRCDIR)%.cpp,$(OBJDIR)/%.o,$(SOURCES))


.PHONY: all clean debug release print-%

all: release
#	$(info SRCDIR = $(SRCDIR))
debug: ; $(MAKE) MODE=dbg
release: $(BUILDROOT)/$(BINOUT)

# Link executable
$(BUILDROOT)/$(BINOUT): $(OBJS) main.cpp
	@mkdir -p $(@D)
	$(CC) $(CXXFLAGS) $(INCSTRUCTURE) -o $@ $^ $(LINK_DEPS)

# Compile and generate deps
$(OBJDIR)/%.o: $(SRCDIR)%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) $(INCSTRUCTURE) -c $< -o $@
	$(CC) $(CXXFLAGS) $(INCSTRUCTURE) -MM $< -MT $@ -MF $(@:.o=.d)


-include $(OBJS:.o=.d)

clean:
	rm -rf $(BINDIR)

print-%:
	@echo '$*=$($*)'


docker-image-rel:
	docker build -f Dockerfile-rel -t $(BINOUT):$(VERSIONSHORT) --build-arg VERSION='$(VERSION)' --build-arg ARCHITECTURE=$(ARCHITECTURE) --build-arg BINOUT=$(BINOUT) --build-arg INSTALL_DEPS="$(INSTALL_DEPS)" .
	docker build -f Dockerfile-rel -t $(BINOUT):latest --build-arg VERSION='$(VERSION)' --build-arg ARCHITECTURE=$(ARCHITECTURE) --build-arg BINOUT=$(BINOUT) --build-arg INSTALL_DEPS="$(INSTALL_DEPS)" .

docker-image-dev:
	docker build -f Dockerfile-dev -t $(BINOUT):dev --build-arg VERSION='$(VERSION)' --build-arg ARCHITECTURE=$(ARCHITECTURE) --build-arg BINOUT=$(BINOUT) --build-arg INSTALL_DEPS="$(INSTALL_DEPS)" .


install-deps:
	apt install -y ${INSTALL_DEPS}