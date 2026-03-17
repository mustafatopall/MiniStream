/*
 * ============================================================================
 *  MiniStream — Bellek Yönetimi Testleri
 *  test/test_bellek.c
 * ============================================================================
 *
 *  Bu dosya bellek sızıntısı olmadığını kanıtlamak için tasarlanmıştır.
 *  Her testin sonunda:
 *    izci_malloc_sayisi() == izci_free_sayisi()
 *  koşulu sağlanmalıdır.
 */

#include "../src/bellek_izci.h"
#include "../src/hash_map.h"
#include "../src/ministream.h"
#include <assert.h>

static int test_sayaci = 0;
static int basarili = 0;

#define TEST_BASLAT(isim)                                                      \
  do {                                                                         \
    test_sayaci++;                                                             \
    printf("  [TEST %2d] %-50s ", test_sayaci, isim);                          \
  } while (0)

#define TEST_GECTI()                                                           \
  do {                                                                         \
    basarili++;                                                                \
    printf("GECTI ✓\n");                                                       \
  } while (0)

#define TEST_KONTROL(kosul, mesaj)                                             \
  do {                                                                         \
    if (!(kosul)) {                                                            \
      printf("BASARISIZ ✗ (%s)\n", mesaj);                                     \
      return;                                                                  \
    }                                                                          \
  } while (0)

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test 1: Toplu oluştur/sil — sızıntı yok mu?
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_toplu_olustur_sil(void) {
  TEST_BASLAT("1000 sarki olustur/sil — sizinti kontrolu");

  izci_sifirla();

  Sarki *sarkilar[1000];
  for (int i = 0; i < 1000; i++) {
    sarkilar[i] = sarki_olustur(i + 1, "Test", "Art", "Alb", 200, 2020);
  }

  /* Hepsini sil */
  for (int i = 0; i < 1000; i++) {
    sarki_sil(sarkilar[i]);
  }

  TEST_KONTROL(izci_malloc_sayisi() == izci_free_sayisi(),
               "malloc ve free sayilari esit olmali");
  TEST_KONTROL(aktif_bellek() == 0, "aktif bellek 0 olmali");

  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test 2: Liste oluştur → şarkı ekle → temizle — sızıntı yok mu?
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_liste_bellek(void) {
  TEST_BASLAT("liste olustur/ekle/temizle — sizinti kontrolu");

  izci_sifirla();

  CalmaListesi *liste = liste_olustur(1, "Test Listesi");
  for (int i = 0; i < 50; i++) {
    Sarki *s = sarki_olustur(i + 1, "Sarki", "Art", "Alb", 200, 2020);
    liste_sarki_ekle(liste, s);
  }

  /* liste_temizle şarkıların ref_sayisi'nı azaltır ve ref_sayisi==0 olanları
   * siler */
  liste_temizle(liste);

  TEST_KONTROL(izci_malloc_sayisi() == izci_free_sayisi(),
               "malloc ve free sayilari esit olmali");
  TEST_KONTROL(aktif_bellek() == 0, "aktif bellek 0 olmali");

  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test 3: Çoklu liste paylaşımı — sızıntı yok mu?
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_coklu_liste_paylasim(void) {
  TEST_BASLAT("5 liste 100 sarki paylasimi — sizinti kontrolu");

  izci_sifirla();

  /* 100 şarkı oluştur */
  Sarki *sarkilar[100];
  for (int i = 0; i < 100; i++) {
    sarkilar[i] = sarki_olustur(i + 1, "Sarki", "Art", "Alb", 200, 2020);
  }

  /* 5 liste oluştur, her birine tüm şarkıları ekle */
  CalmaListesi *listeler[5];
  for (int j = 0; j < 5; j++) {
    char isim[50];
    snprintf(isim, sizeof(isim), "Liste_%d", j + 1);
    listeler[j] = liste_olustur(j + 1, isim);

    for (int i = 0; i < 100; i++) {
      liste_sarki_ekle(listeler[j], sarkilar[i]);
    }
  }

  /* Her şarkının ref_sayisi = 5 olmalı */
  TEST_KONTROL(sarkilar[0]->ref_sayisi == 5,
               "ref_sayisi 5 olmali (5 listede var)");

  /* Tüm listeleri temizle */
  for (int j = 0; j < 5; j++) {
    liste_temizle(listeler[j]);
  }

  /* Tüm şarkılar silinmiş olmalı (son liste temizlenince ref_sayisi=0 olur) */
  TEST_KONTROL(izci_malloc_sayisi() == izci_free_sayisi(),
               "malloc ve free sayilari esit olmali");
  TEST_KONTROL(aktif_bellek() == 0, "aktif bellek 0 olmali");

  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test 4: HashMap oluştur/temizle — sızıntı yok mu?
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_hashmap_bellek(void) {
  TEST_BASLAT("hashmap olustur/temizle — sizinti kontrolu");

  izci_sifirla();

  HashMap *map = veri_uret_map(500);
  /* veri_uret_map şarkıları da oluşturur */

  /* Şarkıları hashmap'ten bulup sil, sonra hashmap'i temizle */
  /* Önce tüm şarkıları bul ve referanslarını tut */
  Sarki *sarkilar[500];
  for (int i = 0; i < 500; i++) {
    sarkilar[i] = sarki_ara_map(map, i + 1);
  }

  hashmap_temizle(map);

  /* Şarkıları sil */
  for (int i = 0; i < 500; i++) {
    if (sarkilar[i] != NULL) {
      sarki_sil(sarkilar[i]);
    }
  }

  TEST_KONTROL(izci_malloc_sayisi() == izci_free_sayisi(),
               "malloc ve free sayilari esit olmali");
  TEST_KONTROL(aktif_bellek() == 0, "aktif bellek 0 olmali");

  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test 5: CSV yükle ve temizle — sızıntı yok mu?
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_csv_bellek(void) {
  TEST_BASLAT("csv 1000 sarki yukle/temizle — sizinti kontrolu");

  izci_sifirla();

  int toplam = 0;
  Sarki *liste = csv_yukle("data/sarkilar.csv", 1000, &toplam);

  TEST_KONTROL(liste != NULL, "csv_yukle NULL dondu");
  TEST_KONTROL(toplam == 1000, "1000 sarki yuklenmeli");

  printf("\n");
  bellek_raporu_yazdir();
  printf("  [TEST %2d] %-50s ", test_sayaci, "(devam)");

  liste_temizle_hepsi(liste);

  TEST_KONTROL(aktif_bellek() == 0, "aktif bellek 0 olmali");

  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Ana Test Fonksiyonu
 * ═══════════════════════════════════════════════════════════════════════════
 */
int main(void) {
  printf("\n");
  printf("╔══════════════════════════════════════════════╗\n");
  printf("║      MiniStream — Bellek Yonetimi Testleri   ║\n");
  printf("╠══════════════════════════════════════════════╣\n\n");

  test_toplu_olustur_sil();
  test_liste_bellek();
  test_coklu_liste_paylasim();
  test_hashmap_bellek();
  test_csv_bellek();

  printf("\n╠══════════════════════════════════════════════╣\n");
  printf("║  Sonuc: %d / %d test GECTI                    ║\n", basarili,
         test_sayaci);
  printf("╚══════════════════════════════════════════════╝\n");

  return (basarili == test_sayaci) ? 0 : 1;
}
