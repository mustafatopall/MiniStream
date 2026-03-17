# ============================================================================
#  MiniStream — Makefile
#  Hedefler: make, make test, make benchmark, make valgrind, make clean
# ============================================================================

CC       = gcc
CFLAGS   = -Wall -Wextra -Werror -std=c99 -g -O2
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer

SRC_DIR  = src
TEST_DIR = test
BIN_DIR  = bin

# Kaynak dosyalar
SRCS = $(SRC_DIR)/ministream.c $(SRC_DIR)/bellek_izci.c $(SRC_DIR)/hash_map.c $(SRC_DIR)/linked_list.c

# Hedef yürütülebilir dosyalar
TEST_TEMEL  = $(BIN_DIR)/test_temel
TEST_BELLEK = $(BIN_DIR)/test_bellek
BENCHMARK   = $(BIN_DIR)/benchmark

# ASAN versiyonları
TEST_TEMEL_ASAN  = $(BIN_DIR)/test_temel_asan
TEST_BELLEK_ASAN = $(BIN_DIR)/test_bellek_asan
BENCHMARK_ASAN   = $(BIN_DIR)/benchmark_asan

# ─── Varsayılan hedef ─────────────────────────────────────────────────────
.PHONY: all
all: $(TEST_TEMEL) $(TEST_BELLEK) $(BENCHMARK)
	@echo ""
	@echo "╔══════════════════════════════════════════════╗"
	@echo "║  Derleme basarili!                           ║"
	@echo "║  Kullanim: make test | make benchmark        ║"
	@echo "╚══════════════════════════════════════════════╝"

# ─── bin/ klasörü oluştur ──────────────────────────────────────────────────
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# ─── Derleme kuralları ─────────────────────────────────────────────────────
$(TEST_TEMEL): $(TEST_DIR)/test_temel.c $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_BELLEK): $(TEST_DIR)/test_bellek.c $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BENCHMARK): $(TEST_DIR)/benchmark.c $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# ─── ASAN derleme kuralları ────────────────────────────────────────────────
$(TEST_TEMEL_ASAN): $(TEST_DIR)/test_temel.c $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $@ $^

$(TEST_BELLEK_ASAN): $(TEST_DIR)/test_bellek.c $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $@ $^

$(BENCHMARK_ASAN): $(TEST_DIR)/benchmark.c $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -o $@ $^

# ─── Test hedefleri ────────────────────────────────────────────────────────
.PHONY: test
test: $(TEST_TEMEL) $(TEST_BELLEK)
	@echo ""
	@echo "═══════════════════════ TEST_TEMEL ═══════════════════════"
	@./$(TEST_TEMEL)
	@echo ""
	@echo "═══════════════════════ TEST_BELLEK ══════════════════════"
	@./$(TEST_BELLEK)
	@echo ""
	@echo "═══════════════ TUM TESTLER TAMAMLANDI ═══════════════════"

# ─── Benchmark hedefi ─────────────────────────────────────────────────────
.PHONY: benchmark
benchmark: $(BENCHMARK)
	@echo ""
	@./$(BENCHMARK)

# ─── Valgrind / AddressSanitizer hedefi ────────────────────────────────────
.PHONY: valgrind
valgrind: $(TEST_TEMEL_ASAN) $(TEST_BELLEK_ASAN) $(BENCHMARK_ASAN)
	@mkdir -p rapor/valgrind
	@echo ""
	@echo "╔══════════════════════════════════════════════╗"
	@echo "║    AddressSanitizer ile Bellek Kontrolu       ║"
	@echo "╚══════════════════════════════════════════════╝"
	@echo ""
	@echo "─── test_temel (ASAN) ───"
	@./$(TEST_TEMEL_ASAN) 2>&1 | tee rapor/valgrind/test_temel.txt
	@echo ""
	@echo "─── test_bellek (ASAN) ───"
	@./$(TEST_BELLEK_ASAN) 2>&1 | tee rapor/valgrind/test_bellek.txt
	@echo ""
	@echo "─── benchmark (ASAN) ───"
	@./$(BENCHMARK_ASAN) 2>&1 | tee rapor/valgrind/benchmark.txt
	@echo ""
	@echo "╔══════════════════════════════════════════════╗"
	@echo "║  ASAN raporlari rapor/valgrind/ altina        ║"
	@echo "║  kaydedildi.                                  ║"
	@echo "╚══════════════════════════════════════════════╝"

# ─── Temizlik ──────────────────────────────────────────────────────────────
.PHONY: clean
clean:
	@rm -rf $(BIN_DIR)
	@echo "Temizlendi."
