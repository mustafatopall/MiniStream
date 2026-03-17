/*
 * ============================================================================
 *  MiniStream — Benchmark & Ana Deney
 *  test/benchmark.c
 * ============================================================================
 *
 *  İçerik:
 *    1. Linked List vs Hash Map arama performansı karşılaştırması
 *    2. Kopya Modeli vs Pointer Modeli bellek/hız karşılaştırması
 */

#include "../src/bellek_izci.h"
#include "../src/hash_map.h"
#include "../src/linked_list.h"
#include "../src/ministream.h"
#include <time.h>

/* ─────────────────────────────────────────────────────────────────────────────
 *  Yardımcı: ms cinsinden süre ölçümü
 * ─────────────────────────────────────────────────────────────────────────────
 */
static double ms_farki(clock_t baslangic, clock_t bitis) {
  return ((double)(bitis - baslangic) / CLOCKS_PER_SEC) * 1000.0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  BENCHMARK 1: Linked List vs Hash Map Arama Performansı
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void benchmark_arama(void) {
  printf("\n");
  printf("╔══════════════════════════════════════════════════════════════╗\n");
  printf("║   BENCHMARK 1: Linked List vs Hash Map Arama Performansi   ║\n");
  printf("╠══════════════════════════════════════════════════════════════╣\n");
  printf("║   N Deger   │  Linked List (ms)  │    Hash Map (ms)        ║\n");
  printf("╠═════════════╪════════════════════╪═════════════════════════╣\n");

  int N_degerleri[] = {100, 1000, 10000, 100000};
  int N_adet = 4;
  int SORGU_SAYISI = 1000;

  for (int t = 0; t < N_adet; t++) {
    int N = N_degerleri[t];

    /* Veri üret */
    Sarki *ll_veri = veri_uret_liste(N);
    HashMap *hm_veri = veri_uret_map(N);

    /* Rastgele sorgu ID'leri oluştur */
    int sorgular[1000];
    for (int i = 0; i < SORGU_SAYISI; i++) {
      sorgular[i] = (rand() % N) + 1;
    }

    /* --- Linked List Arama --- */
    clock_t t1 = clock();
    for (int i = 0; i < SORGU_SAYISI; i++) {
      sarki_ara_liste(ll_veri, sorgular[i]);
    }
    clock_t t2 = clock();
    double ll_sure = ms_farki(t1, t2);

    /* --- Hash Map Arama --- */
    clock_t t3 = clock();
    for (int i = 0; i < SORGU_SAYISI; i++) {
      sarki_ara_map(hm_veri, sorgular[i]);
    }
    clock_t t4 = clock();
    double hm_sure = ms_farki(t3, t4);

    printf("║  %9d  │     %10.3f     │     %10.3f           ║\n", N, ll_sure,
           hm_sure);

    /* Temizle */
    liste_temizle_hepsi(ll_veri);

    /* Hash map temizliği: önce şarkıları bul, hashmap'i temizle, sonra
     * şarkıları sil */
    Sarki **hm_sarkilar = (Sarki **)malloc(sizeof(Sarki *) * (size_t)N);
    for (int i = 0; i < N; i++) {
      hm_sarkilar[i] = sarki_ara_map(hm_veri, i + 1);
    }
    hashmap_temizle(hm_veri);
    for (int i = 0; i < N; i++) {
      if (hm_sarkilar[i] != NULL) {
        izlenen_free(hm_sarkilar[i], sizeof(Sarki));
      }
    }
    free(hm_sarkilar);
  }

  printf("╠═════════════╧════════════════════╧═════════════════════════╣\n");
  printf("║  (Her test %d rastgele sorgu ile yapildi)                  ║\n",
         SORGU_SAYISI);
  printf("╚══════════════════════════════════════════════════════════════╝\n");
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  BENCHMARK 2: Kopya Modeli vs Pointer Modeli
 * ═══════════════════════════════════════════════════════════════════════════
 */

/*
 * kopya_modeli_test — Her listeye şarkı eklerken tam kopya oluşturur.
 * Bu model bellek israfını göstermek içindir.
 */
static void kopya_modeli_test(int n_sarki, int n_liste, int sarki_per_liste) {
  izci_sifirla();
  clock_t baslangic = clock();

  /* Orijinal şarkıları oluştur */
  Sarki **orijinaller =
      (Sarki **)izlenen_malloc(sizeof(Sarki *) * (size_t)n_sarki);
  for (int i = 0; i < n_sarki; i++) {
    orijinaller[i] = sarki_olustur(i + 1, "Sarki", "Art", "Alb", 200, 2020);
  }

  /* Listeleri oluştur — her ekleme için KOPYA yap */
  CalmaListesi **listeler =
      (CalmaListesi **)izlenen_malloc(sizeof(CalmaListesi *) * (size_t)n_liste);

  for (int j = 0; j < n_liste; j++) {
    listeler[j] = liste_olustur(j + 1, "KopyaListe");

    for (int i = 0; i < sarki_per_liste && i < n_sarki; i++) {
      /* KOPYA oluştur — malloc + memcpy */
      Sarki *kopya = (Sarki *)izlenen_malloc(sizeof(Sarki));
      memcpy(kopya, orijinaller[i], sizeof(Sarki));
      kopya->ref_sayisi = 0;
      kopya->sonraki = NULL;

      /* Listeye doğrudan pointer olarak ekle (ama bu kopyanın pointer'ı) */
      liste_sarki_ekle(listeler[j], kopya);
    }
  }

  clock_t bitis = clock();
  double sure_ms = ms_farki(baslangic, bitis);
  int malloc_adet = izci_malloc_sayisi();
  size_t bellek = izci_toplam_ayrildi();

  printf("║  Kopya Modeli │ %10d │ %10.2f │ %10.3f │", malloc_adet,
         (double)bellek / (1024.0 * 1024.0), sure_ms);

  /* Temizle — her kopya sadece 1 listede, liste_temizle ref_sayisi'yi azaltıp
   * 0 olunca sarki_sil çağırır, bu yüzden doğrudan liste_temizle yeterli */
  for (int j = 0; j < n_liste; j++) {
    liste_temizle(listeler[j]);
  }
  izlenen_free(listeler, sizeof(CalmaListesi *) * (size_t)n_liste);

  /* Orijinalleri sil (bunlar hiçbir listeye eklenmedi, ref_sayisi=0) */
  for (int i = 0; i < n_sarki; i++) {
    sarki_sil(orijinaller[i]);
  }
  izlenen_free(orijinaller, sizeof(Sarki *) * (size_t)n_sarki);

  size_t sizinti = aktif_bellek();
  printf("  %s  ║\n", (sizinti == 0) ? "YOK ✓" : "VAR ✗");
}

/*
 * pointer_modeli_test — Listeye sadece pointer ekler, kopya oluşturmaz.
 * ref_sayisi ile yaşam döngüsü yönetilir.
 */
static void pointer_modeli_test(int n_sarki, int n_liste, int sarki_per_liste) {
  izci_sifirla();
  clock_t baslangic = clock();

  /* Şarkıları oluştur (tek kopya!) */
  Sarki **sarkilar =
      (Sarki **)izlenen_malloc(sizeof(Sarki *) * (size_t)n_sarki);
  for (int i = 0; i < n_sarki; i++) {
    sarkilar[i] = sarki_olustur(i + 1, "Sarki", "Art", "Alb", 200, 2020);
  }

  /* Listeleri oluştur — sadece POINTER ekle */
  CalmaListesi **listeler =
      (CalmaListesi **)izlenen_malloc(sizeof(CalmaListesi *) * (size_t)n_liste);

  for (int j = 0; j < n_liste; j++) {
    listeler[j] = liste_olustur(j + 1, "PointerListe");

    for (int i = 0; i < sarki_per_liste && i < n_sarki; i++) {
      liste_sarki_ekle(listeler[j], sarkilar[i]);
    }
  }

  clock_t bitis = clock();
  double sure_ms = ms_farki(baslangic, bitis);
  int malloc_adet = izci_malloc_sayisi();
  size_t bellek = izci_toplam_ayrildi();

  printf("║ Pointer Model │ %10d │ %10.2f │ %10.3f │", malloc_adet,
         (double)bellek / (1024.0 * 1024.0), sure_ms);

  /* Temizle — listeleri temizle (ref_sayisi otomatik yönetilir)
   * Son liste temizlendiğinde her şarkının ref_sayisi 0 olur ve silinir */
  for (int j = 0; j < n_liste; j++) {
    liste_temizle(listeler[j]);
  }
  izlenen_free(listeler, sizeof(CalmaListesi *) * (size_t)n_liste);

  /* Listeye eklenmemiş şarkıları sil (sarki_per_liste < n_sarki durumunda) */
  int spl = (sarki_per_liste < n_sarki) ? sarki_per_liste : n_sarki;
  for (int i = spl; i < n_sarki; i++) {
    sarki_sil(sarkilar[i]);
  }

  /* Sarki pointer dizisini sil */
  izlenen_free(sarkilar, sizeof(Sarki *) * (size_t)n_sarki);

  size_t sizinti = aktif_bellek();
  printf("  %s  ║\n", (sizinti == 0) ? "YOK ✓" : "VAR ✗");
}

static void benchmark_kopya_vs_pointer(void) {
  printf("\n");
  printf(
      "╔══════════════════════════════════════════════════════════════════╗\n");
  printf(
      "║    BENCHMARK 2: Kopya Modeli vs Pointer Modeli                  ║\n");
  printf(
      "╠══════════════════════════════════════════════════════════════════╣\n");

  /* ─── Deney 1: Küçük ölçek ─── */
  int n_sarki1 = 1000, n_liste1 = 10, spl1 = 50;
  printf(
      "║                                                                  ║\n");
  printf(
      "║  Deney 1: %d sarki, %d liste, %d sarki/liste                    ║\n",
      n_sarki1, n_liste1, spl1);
  printf(
      "╠═══════════════╪════════════╪════════════╪═══════════╪══════════╣\n");
  printf(
      "║    Model      │ malloc (#) │ Bellek(MB) │ Sure (ms) │ Sizinti  ║\n");
  printf(
      "╠═══════════════╪════════════╪════════════╪═══════════╪══════════╣\n");

  kopya_modeli_test(n_sarki1, n_liste1, spl1);
  pointer_modeli_test(n_sarki1, n_liste1, spl1);

  printf(
      "╠═══════════════╧════════════╧════════════╧═══════════╧══════════╣\n");

  /* ─── Deney 2: Orta ölçek ─── */
  int n_sarki2 = 10000, n_liste2 = 100, spl2 = 50;
  printf(
      "║                                                                  ║\n");
  printf("║  Deney 2: %d sarki, %d liste, %d sarki/liste                  ║\n",
         n_sarki2, n_liste2, spl2);
  printf(
      "╠═══════════════╪════════════╪════════════╪═══════════╪══════════╣\n");
  printf(
      "║    Model      │ malloc (#) │ Bellek(MB) │ Sure (ms) │ Sizinti  ║\n");
  printf(
      "╠═══════════════╪════════════╪════════════╪═══════════╪══════════╣\n");

  kopya_modeli_test(n_sarki2, n_liste2, spl2);
  pointer_modeli_test(n_sarki2, n_liste2, spl2);

  printf(
      "╠═══════════════╧════════════╧════════════╧═══════════╧══════════╣\n");

  /* ─── Deney 3: Büyük ölçek ─── */
  int n_sarki3 = 50000, n_liste3 = 500, spl3 = 50;
  printf(
      "║                                                                  ║\n");
  printf("║  Deney 3: %d sarki, %d liste, %d sarki/liste                 ║\n",
         n_sarki3, n_liste3, spl3);
  printf(
      "╠═══════════════╪════════════╪════════════╪═══════════╪══════════╣\n");
  printf(
      "║    Model      │ malloc (#) │ Bellek(MB) │ Sure (ms) │ Sizinti  ║\n");
  printf(
      "╠═══════════════╪════════════╪════════════╪═══════════╪══════════╣\n");

  kopya_modeli_test(n_sarki3, n_liste3, spl3);
  pointer_modeli_test(n_sarki3, n_liste3, spl3);

  printf(
      "╚═══════════════╧════════════╧════════════╧═══════════╧══════════╝\n");
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  BENCHMARK 3: CSV Yükleme Performansı
 * ═══════════════════════════════════════════════════════════════════════════
 */
static void benchmark_csv(void) {
  printf("\n");
  printf("╔══════════════════════════════════════════════════════════════╗\n");
  printf("║         BENCHMARK 3: CSV Yukleme Performansi               ║\n");
  printf("╠══════════════════════════════════════════════════════════════╣\n");

  izci_sifirla();

  clock_t t1 = clock();
  int toplam = 0;
  Sarki *liste = csv_yukle("data/sarkilar.csv", 10000, &toplam);
  clock_t t2 = clock();

  printf("║  %d sarki yuklendi                                        ║\n",
         toplam);
  printf("║  Yukleme suresi: %.3f ms                                  ║\n",
         ms_farki(t1, t2));

  bellek_raporu_yazdir();

  if (liste != NULL) {
    liste_temizle_hepsi(liste);
  }

  printf("╚══════════════════════════════════════════════════════════════╝\n");
}

/* ═══════════════════════════════════════════════════════════════════════════
 */
int main(void) {
  srand((unsigned int)time(NULL));

  printf("\n");
  printf("╔══════════════════════════════════════════════════════════════╗\n");
  printf("║              MiniStream — Benchmark Suite                    ║\n");
  printf("╚══════════════════════════════════════════════════════════════╝\n");

  benchmark_arama();
  benchmark_kopya_vs_pointer();
  benchmark_csv();

  printf("\n[BENCHMARK] Tum testler tamamlandi.\n\n");
  return 0;
}
