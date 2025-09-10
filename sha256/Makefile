EXECUTABLES := server client
BUILD_DIR := build

.PHONY: all build move clean

all: build move

build:
	@echo "Creazione cartella build..."
	@rm -f /tmp/sha256_fifo_request
	@rm -f /tmp/sha256_fifo_response*
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
		-DCMAKE_CXX_FLAGS
	cmake --build $(BUILD_DIR)

move: build
	@for exe in $(EXECUTABLES); do \
	  if [ -f $(BUILD_DIR)/$$exe ]; then \
	    cp $(BUILD_DIR)/$$exe . ; \
	    echo "  - $$exe moved."; \
	  else \
	    echo "  ! Warning: $$exe not found in $(BUILD_DIR)."; \
	  fi \
	done
	@ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json

clean:
	rm -rf $(BUILD_DIR)/*
	@for exe in $(EXECUTABLES); do \
	  if [ -f $$exe ]; then \
	    rm $$exe && echo "  - $$exe removed."; \
	  fi \
	done
	@echo "Pulizia completata"
