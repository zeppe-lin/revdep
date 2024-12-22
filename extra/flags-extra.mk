######################################################################
# Extra warning flags.                                               #
######################################################################

# includes and libs
INCS     =
LIBS     = -lelf

# compiler flags
CPPFLAGS = -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 \
           -DVERSION=\"$(VERSION)\"
CXXFLAGS = -std=c++0x \
           -pedantic \
           -Wall \
           -Warray-bounds=2 \
           -Wcast-qual \
           -Wconversion \
           -Wctor-dtor-privacy \
           -Wduplicated-cond \
           -Wextra \
           -Wformat \
           -Wformat-security \
           -Wflex-array-member-not-at-end \
           -Wfloat-equal \
           -Wimplicit-fallthrough \
           -Winline \
           -Wlogical-op \
           -Wmissing-declarations \
           -Wnon-virtual-dtor \
           -Wnrvo \
           -Wold-style-cast \
           -Wpacked \
           -Wparentheses \
           -Wredundant-decls \
           -Wshadow \
           -Wshift-negative-value \
           -Wsign-conversion \
           -Wstrict-overflow=5 \
           -Wstringop-overflow=4 \
           -Wswitch-enum \
           -Wuninitialized \
           -Wunsafe-loop-optimizations \
           -Wunused \
           -Wuseless-cast \
           -Wcast-align \
           -Wswitch-default
LDFLAGS  = $(LIBS)
