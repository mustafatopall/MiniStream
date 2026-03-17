/*
 * ============================================================================
 *  MiniStream — Bellek İzleyici
 *  bellek_izci.c — İzlenen malloc/free implementasyonu
 * ============================================================================
 *
 *  Her malloc ve free çağrısını sayar ve toplam bellek kullanımını izler.
 *  Projede ham malloc/free yerine bu fonksiyonlar kullanılmalıdır.
 */

#include "bellek_izci.h"
#include <stdio.h>
#include <stdlib.h>

/* ─────────────────────────────────────────────────────────────────────────────
 *  Dahili Sayaçlar
 * ─────────────────────────────────────────────────────────────────────────────
 */
static int g_malloc_sayisi = 0;
static int g_free_sayisi = 0;
static size_t g_toplam_ayrildi = 0;   /* Toplam ayrılan byte        */
static size_t g_toplam_serbest = 0;   /* Toplam serbest bırakılan   */
static size_t g_aktif_bellek = 0;     /* Şu an kullanılan byte      */
static size_t g_en_yuksek_bellek = 0; /* Zirve bellek kullanımı     */

/* ─────────────────────────────────────────────────────────────────────────────
 *  izlenen_malloc — malloc sarmalayıcı
 *  Başarılı olursa pointer döndürür, başarısız olursa NULL + hata mesajı.
 * ─────────────────────────────────────────────────────────────────────────────
 */
void *izlenen_malloc(size_t boyut) {
  void *ptr = malloc(boyut);
  if (ptr == NULL) {
    fprintf(stderr, "[BELLEK HATA] malloc(%zu) basarisiz!\n", boyut);
    return NULL;
  }

  g_malloc_sayisi++;
  g_toplam_ayrildi += boyut;
  g_aktif_bellek += boyut;

  if (g_aktif_bellek > g_en_yuksek_bellek) {
    g_en_yuksek_bellek = g_aktif_bellek;
  }

  return ptr;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  izlenen_realloc — realloc sarmalayıcı
 *  Eski ve yeni boyut farkını izleyicide günceller.
 * ─────────────────────────────────────────────────────────────────────────────
 */
void *izlenen_realloc(void *ptr, size_t eski_boyut, size_t yeni_boyut) {
  void *yeni_ptr = realloc(ptr, yeni_boyut);
  if (yeni_ptr == NULL) {
    fprintf(stderr, "[BELLEK HATA] realloc(%zu -> %zu) basarisiz!\n",
            eski_boyut, yeni_boyut);
    return NULL;
  }

  /* Boyut farkını izle */
  if (yeni_boyut > eski_boyut) {
    size_t fark = yeni_boyut - eski_boyut;
    g_toplam_ayrildi += fark;
    g_aktif_bellek += fark;
  } else {
    size_t fark = eski_boyut - yeni_boyut;
    g_toplam_serbest += fark;
    g_aktif_bellek -= fark;
  }

  if (g_aktif_bellek > g_en_yuksek_bellek) {
    g_en_yuksek_bellek = g_aktif_bellek;
  }

  return yeni_ptr;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  izlenen_free — free sarmalayıcı
 *  ptr NULL ise sessizce atlar.
 * ─────────────────────────────────────────────────────────────────────────────
 */
void izlenen_free(void *ptr, size_t boyut) {
  if (ptr == NULL)
    return;

  free(ptr);

  g_free_sayisi++;
  g_toplam_serbest += boyut;
  g_aktif_bellek -= boyut;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  izci_sifirla — Tüm sayaçları sıfırla (yeni deney öncesi)
 * ─────────────────────────────────────────────────────────────────────────────
 */
void izci_sifirla(void) {
  g_malloc_sayisi = 0;
  g_free_sayisi = 0;
  g_toplam_ayrildi = 0;
  g_toplam_serbest = 0;
  g_aktif_bellek = 0;
  g_en_yuksek_bellek = 0;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  aktif_bellek — Şu anda kullanılan bellek (byte)
 * ─────────────────────────────────────────────────────────────────────────────
 */
size_t aktif_bellek(void) { return g_aktif_bellek; }

/* ─────────────────────────────────────────────────────────────────────────────
 *  bellek_raporu_yazdir — Kapsamlı bellek raporu
 * ─────────────────────────────────────────────────────────────────────────────
 */
void bellek_raporu_yazdir(void) {
  printf("\n");
  printf("╔══════════════════════════════════════════════╗\n");
  printf("║           BELLEK IZLEYICI RAPORU             ║\n");
  printf("╠══════════════════════════════════════════════╣\n");
  printf("║  malloc cagrisi       : %10d            ║\n", g_malloc_sayisi);
  printf("║  free cagrisi         : %10d            ║\n", g_free_sayisi);
  printf("║  Toplam ayrilan       : %10zu byte      ║\n", g_toplam_ayrildi);
  printf("║  Toplam serbest       : %10zu byte      ║\n", g_toplam_serbest);
  printf("║  Aktif bellek         : %10zu byte      ║\n", g_aktif_bellek);
  printf("║  Zirve bellek         : %10zu byte      ║\n", g_en_yuksek_bellek);
  printf("║  Aktif bellek (MB)    : %13.2f MB     ║\n",
         (double)g_aktif_bellek / (1024.0 * 1024.0));
  printf("║  Sizinti              : %10s            ║\n",
         (g_aktif_bellek == 0) ? "YOK ✓" : "VAR ✗");
  printf("╚══════════════════════════════════════════════╝\n");
  printf("\n");
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  Getter Fonksiyonları
 * ─────────────────────────────────────────────────────────────────────────────
 */
int izci_malloc_sayisi(void) { return g_malloc_sayisi; }
int izci_free_sayisi(void) { return g_free_sayisi; }
size_t izci_toplam_ayrildi(void) { return g_toplam_ayrildi; }
size_t izci_toplam_serbest(void) { return g_toplam_serbest; }
