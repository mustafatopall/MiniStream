/*
 * ============================================================================
 *  MiniStream — Temel Birim Testleri
 *  test/test_temel.c
 * ============================================================================
 *
 *  Testler:
 *    1. sarki_olustur — doğru alanlar mı?
 *    2. liste_sarki_ekle — pointer kopyalandı mı (veri değil)?
 *    3. ref_sayisi senaryosu — paylaşım, silme reddi, sonra silme
 *    4. Kapasite büyüme — realloc çalışıyor mu?
 *    5. CSV yükleme — dosya parse edilebiliyor mu?
 */

#include "../src/bellek_izci.h"
#include "../src/hash_map.h"
#include "../src/linked_list.h"
#include "../src/ministream.h"
#include <assert.h>

/* ═══════════════════════════════════════════════════════════════════════════
 */
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
 *  Test 1: sarki_olustur
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_sarki_olustur(void) {
  TEST_BASLAT("sarki_olustur — alan kontrolu");

  Sarki *s = sarki_olustur(1, "Bohemian Rhapsody", "Queen",
                           "A Night at the Opera", 354, 1975);

  TEST_KONTROL(s != NULL, "sarki NULL dondu");
  TEST_KONTROL(s->id == 1, "id hatasi");
  TEST_KONTROL(strcmp(s->baslik, "Bohemian Rhapsody") == 0, "baslik hatasi");
  TEST_KONTROL(strcmp(s->sanatci, "Queen") == 0, "sanatci hatasi");
  TEST_KONTROL(strcmp(s->album, "A Night at the Opera") == 0, "album hatasi");
  TEST_KONTROL(s->sure_sn == 354, "sure hatasi");
  TEST_KONTROL(s->yil == 1975, "yil hatasi");
  TEST_KONTROL(s->ref_sayisi == 0, "ref_sayisi 0 olmali");
  TEST_KONTROL(s->sonraki == NULL, "sonraki NULL olmali");

  sarki_sil(s);
  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test 2: liste_sarki_ekle — pointer paylaşımı
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_pointer_paylasimi(void) {
  TEST_BASLAT("liste_sarki_ekle — pointer paylasimi");

  Sarki *s = sarki_olustur(1, "Test", "Sanatci", "Album", 200, 2020);
  CalmaListesi *l = liste_olustur(1, "Favoriler");

  liste_sarki_ekle(l, s);

  /* Pointer aynı mı? (kopya değil!) */
  TEST_KONTROL(l->sarkilar[0] == s,
               "pointer kopyalanmadi, veri kopyasi olusturulmus");
  TEST_KONTROL(s->ref_sayisi == 1, "ref_sayisi 1 olmali");

  liste_temizle(l); /* ref_sayisi=0, sarki_sil çağrılır */
  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test 3: ref_sayisi — tam senaryo
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_ref_sayisi_senaryo(void) {
  TEST_BASLAT("ref_sayisi — 2 liste paylasimi senaryosu");

  Sarki *s = sarki_olustur(42, "Shared Song", "Artist", "Album", 180, 2022);

  CalmaListesi *l1 = liste_olustur(1, "Liste1");
  CalmaListesi *l2 = liste_olustur(2, "Liste2");

  /* 1) İki listeye ekle */
  liste_sarki_ekle(l1, s);
  liste_sarki_ekle(l2, s);
  TEST_KONTROL(s->ref_sayisi == 2, "ekleme sonrasi ref_sayisi 2 olmali");

  /* 2) Silmeyi dene — reddedilmeli */
  int sonuc = sarki_sil(s);
  TEST_KONTROL(sonuc == -1, "ref_sayisi>0 iken silme reddedilmeli");

  /* 3) İlk listeden çıkar */
  liste_sarki_cikar(l1, 0);
  TEST_KONTROL(s->ref_sayisi == 1, "cikarma sonrasi ref_sayisi 1 olmali");

  /* 4) İkinci listeden çıkar */
  liste_sarki_cikar(l2, 0);
  TEST_KONTROL(s->ref_sayisi == 0,
               "ikinci cikarma sonrasi ref_sayisi 0 olmali");

  /* 5) Şimdi silme başarılı olmalı */
  sonuc = sarki_sil(s);
  TEST_KONTROL(sonuc == 0, "ref_sayisi=0 iken silme basarili olmali");

  /* Listeleri temizle (şarkı zaten silindi, pointer dizilerini serbest bırak)
   */
  /* elle temizle çünkü şarkılar zaten silindi */
  l1->sarki_sayisi = 0;
  l2->sarki_sayisi = 0;
  liste_temizle(l1);
  liste_temizle(l2);

  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test 4: Kapasite büyüme (realloc testi)
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_kapasite_buyume(void) {
  TEST_BASLAT("liste — kapasite buyume (10 -> 20)");

  CalmaListesi *liste = liste_olustur(1, "Buyuyen Liste");
  TEST_KONTROL(liste->kapasite == 10, "baslangic kapasitesi 10 olmali");

  /* 15 şarkı ekle — kapasite 10'dan 20'ye büyümeli */
  Sarki *sarkilar[15];
  for (int i = 0; i < 15; i++) {
    sarkilar[i] = sarki_olustur(i + 1, "Test", "Art", "Alb", 200, 2020);
    liste_sarki_ekle(liste, sarkilar[i]);
  }

  TEST_KONTROL(liste->kapasite == 20, "kapasite 20'ye buyumeli");
  TEST_KONTROL(liste->sarki_sayisi == 15, "sarki_sayisi 15 olmali");

  liste_temizle(liste);
  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test 5: Hash Map arama
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_hashmap_arama(void) {
  TEST_BASLAT("hashmap — ekle ve ara");

  HashMap *map = hashmap_olustur();
  Sarki *s1 = sarki_olustur(100, "Sarki A", "Art A", "Alb A", 200, 2020);
  Sarki *s2 = sarki_olustur(200, "Sarki B", "Art B", "Alb B", 300, 2021);
  Sarki *s3 = sarki_olustur(1124, "Sarki C", "Art C", "Alb C", 250, 2022);

  hashmap_ekle(map, s1);
  hashmap_ekle(map, s2);
  hashmap_ekle(map, s3);

  /* Doğru arama */
  TEST_KONTROL(sarki_ara_map(map, 100) == s1, "id=100 bulunamadi");
  TEST_KONTROL(sarki_ara_map(map, 200) == s2, "id=200 bulunamadi");
  TEST_KONTROL(sarki_ara_map(map, 1124) == s3, "id=1124 bulunamadi");

  /* Olmayan ID */
  TEST_KONTROL(sarki_ara_map(map, 999) == NULL, "id=999 NULL donmeli");

  hashmap_temizle(map);
  /* Şarkıları ayrıca sil */
  sarki_sil(s1);
  sarki_sil(s2);
  sarki_sil(s3);

  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test 6: Linked List arama
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_linkedlist_arama(void) {
  TEST_BASLAT("linked list — lineer arama");

  Sarki *liste = veri_uret_liste(100);

  TEST_KONTROL(sarki_ara_liste(liste, 1) != NULL, "id=1 bulunamadi");
  TEST_KONTROL(sarki_ara_liste(liste, 50) != NULL, "id=50 bulunamadi");
  TEST_KONTROL(sarki_ara_liste(liste, 100) != NULL, "id=100 bulunamadi");
  TEST_KONTROL(sarki_ara_liste(liste, 101) == NULL, "id=101 NULL donmeli");

  liste_temizle_hepsi(liste);
  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Test 7: CSV yükleme
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void test_csv_yukleme(void) {
  TEST_BASLAT("csv_yukle — 100 sarki yukle");

  int toplam = 0;
  Sarki *liste = csv_yukle("data/sarkilar.csv", 100, &toplam);

  TEST_KONTROL(liste != NULL, "csv_yukle NULL dondu");
  TEST_KONTROL(toplam == 100, "100 sarki yuklenmeli");

  /* İlk şarkıyı kontrol et */
  TEST_KONTROL(strlen(liste->baslik) > 0, "baslik bos olmamali");
  TEST_KONTROL(liste->sure_sn > 0, "sure_sn > 0 olmali");
  TEST_KONTROL(liste->yil > 1900, "yil > 1900 olmali");

  liste_temizle_hepsi(liste);
  TEST_GECTI();
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Ana Test Fonksiyonu
 * ═══════════════════════════════════════════════════════════════════════════
 */
int main(void) {
  printf("\n");
  printf("╔══════════════════════════════════════════════╗\n");
  printf("║       MiniStream — Temel Birim Testleri      ║\n");
  printf("╠══════════════════════════════════════════════╣\n\n");

  izci_sifirla();

  test_sarki_olustur();
  test_pointer_paylasimi();
  test_ref_sayisi_senaryo();
  test_kapasite_buyume();
  test_hashmap_arama();
  test_linkedlist_arama();
  test_csv_yukleme();

  printf("\n╠══════════════════════════════════════════════╣\n");
  printf("║  Sonuc: %d / %d test GECTI                    ║\n", basarili,
         test_sayaci);
  printf("╚══════════════════════════════════════════════╝\n");

  bellek_raporu_yazdir();

  return (basarili == test_sayaci) ? 0 : 1;
}
