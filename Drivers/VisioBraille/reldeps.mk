# Dependencies for braille.o:
braille.o: $(SRC_DIR)/braille.c
braille.o: $(BLD_TOP)config.h
braille.o: $(SRC_DIR)/brlconf.h
braille.o: $(SRC_DIR)/brldefs-vs.h
braille.o: $(SRC_TOP)Programs/brl.h
braille.o: $(SRC_TOP)Programs/brldefs.h
braille.o: $(SRC_TOP)Programs/misc.h
braille.o: $(SRC_TOP)Programs/scr.h
braille.o: $(SRC_TOP)Programs/message.h
braille.o: $(SRC_TOP)Programs/brl_driver.h

