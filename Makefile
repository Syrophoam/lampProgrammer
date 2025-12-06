BUILDDIR = Build
SRCDIR = src
PKG_CONFIG = /opt/homebrew/bin/pkg-config

X11_CFLAGS = $(shell $(PKG_CONFIG) --cflags x11)
X11_LIBS = $(shell $(PKG_CONFIG) --libs x11)
 
CFLAGS += $(X11_CFLAGS)
LDFLAGS += $(X11_LIBS)
 
CFLAGS += $(SDL_CFLAGS)
LDFLAGS += $(SDL_LIBS)

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SOURCES))

all: lampProgrammer
 
lampProgrammer: $(OBJECTS)
	cc -o Build/lampProgrammer $(OBJECTS) $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	cc -c $(CFLAGS) -g $< -o $@ -Wall
	
	
app: lampProgrammer
	rm -rf LampProgrammer.app
	mkdir LampProgrammer.app
	mkdir LampProgrammer.app/Contents
	mkdir LampProgrammer.app/Contents/MacOS
	mkdir LampProgrammer.app/Contents/Resources
	cp app/Icon.icns LampProgrammer.app/Contents/Resources
	cp app/info.plist LampProgrammer.app/Contents
	cp app/run.sh LampProgrammer.app/Contents/MacOS
	chmod +x LampProgrammer.app/Contents/MacOS/run.sh
	cp Build/lampProgrammer LampProgrammer.app/Contents/MacOS

.PHONY clean:
	rm -rf Build
	mkdir Build

