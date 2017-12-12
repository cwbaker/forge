
local root_makefile_source = [[
.SUFFIXES:
.PHONY: build clean

AR := lib /nologo
CC := cl /nologo
CPPFLAGS := -I. -I..
CCFLAGS :=  
MAKEDEPEND := makedepend.exe
SHELL := cmd.exe

modules := $(wildcard lib_*)

build: $(foreach i,$(modules),$(i).build)

clean: $(foreach i,$(modules),$(i).clean)

include $(foreach i,$(modules),$(i)/Makefile)
]];

local library_makefile_source = [[

module := lib_%d
library := $(module)/$(module).lib
source := $(wildcard $(module)/*.cpp)
objects := $(source:.cpp=.obj)
dependencies := $(source:.cpp=.d)

.PHONY: $(module).build $(module).clean

$(module)/%%.d : $(module)/%%.cpp ; $(MAKEDEPEND) -f- -Y $(CPPFLAGS) $< >$@

$(module)/%%.obj : $(module)/%%.cpp ; $(CC) $(CPPFLAGS) $(CCFLAGS) /Fo$@ -c $<

$(module).build: $(library)

$(module).clean: library := $(library)
$(module).clean: objects := $(objects)
$(module).clean: dependencies := $(dependencies)
$(module).clean: 
	rm -rf $(dependencies) $(objects) $(library)

$(library): $(objects)
	$(AR) /out:$@ $^
	touch $@

-include $(dependencies)
]];

function generate_make_files( libraries, classes )
    local makefile = io.open( root("make/Makefile"), "wb" );
    assert( makefile, "Opening '"..root("make/Makefile").."' failed" );
    makefile:write( root_makefile_source );
    makefile:close();
    makefile = nil;

    for i = 1, libraries do
        local makefile = io.open( root("make/lib_"..tostring(i).."/Makefile"), "wb" );
        assert( makefile, "Opening '"..root("make/lib_"..tostring(i).."/Makefile").."' failed" );
        makefile:write( string.format(library_makefile_source, i) );
        makefile:close();
        makefile = nil;
    end    
end
