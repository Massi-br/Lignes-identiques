# holdall_dir = ../holdall/
# hashtable_dir = ../hashtable/
# option_dir = ../option
# CC = gcc
# CFLAGS = -std=c18 \
#   -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings \
#   -O2 \
#   -I$(holdall_dir) -I$(hashtable_dir) -I$(option_dir) \
#   -DHASHTABLE_STATS=1
# vpath %.c $(holdall_dir) $(hashtable_dir)
# vpath %.h $(holdall_dir) $(hashtable_dir)
# vpath %.h $(option_dir)  $(option_dir)
# objects = hashtable.o holdall.o option.o main.o
# executable = lnid
# makefile_indicator = .\#makefile\#

# .PHONY: all clean

# all: $(executable)

# clean:
# 	$(RM) $(objects) $(executable)
# 	@$(RM) $(makefile_indicator)

# $(executable): $(objects)
# 	$(CC) $(objects) -o $(executable)

# holdall.o: holdall.c holdall.h
# main.o: main.c hashtable.h holdall.h option.h
# hashtable.o: hashtable.c hashtable.h
# option.o: option.c option.h

# include $(makefile_indicator)

# $(makefile_indicator): makefile
# 	@touch $@
# 	@$(RM) $(objects) $(executable)

holdall_dir = ../holdall/
hashtable_dir = ../hashtable/
option_dir = ../option/
CC = gcc
CFLAGS = -std=c18 \
  -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings \
  -O2 \
  -I$(holdall_dir) -I$(hashtable_dir) -I$(option_dir) \
  -DHASHTABLE_STATS=0 -DHOLDALL_PUT_TAIL=1
vpath %.c $(holdall_dir) $(hashtable_dir) $(option_dir)
vpath %.h $(holdall_dir) $(hashtable_dir) $(option_dir)
objects = hashtable.o holdall.o option.o main.o
executable = lnid
makefile_indicator = .\#makefile\#

.PHONY: all clean

all: $(executable)

clean:
	$(RM) $(objects) $(executable)
	@$(RM) $(makefile_indicator)

$(executable): $(objects)
	$(CC) $(objects) -o $(executable)

holdall.o: holdall.c holdall.h
main.o: main.c hashtable.h holdall.h option.h
hashtable.o: hashtable.c hashtable.h
option.o: $(option_dir)option.c $(option_dir)option.h

include $(makefile_indicator)

$(makefile_indicator): makefile
	@touch $@
	@$(RM) $(objects) $(executable)
