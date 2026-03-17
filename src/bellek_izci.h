/*
 * ============================================================================
 *  MiniStream — Bellek İzleyici
 *  bellek_izci.h — İzleyici fonksiyon imzaları
 * ============================================================================
 *
 *  Projede ham malloc/free yerine bu sarmalayıcılar kullanılmalıdır.
 *  Bu sayede tüm bellek işlemleri izlenir ve raporlanır.
 */

#ifndef BELLEK_IZCI_H
#define BELLEK_IZCI_H

#include <stddef.h>

/* ─────────────────────────────────────────────────────────────────────────────
 *  Sarmalayıcı (Wrapper) Fonksiyonlar
 * ─────────────────────────────────────────────────────────────────────────────
 */
void *izlenen_malloc(size_t boyut);
void *izlenen_realloc(void *ptr, size_t eski_boyut, size_t yeni_boyut);
void izlenen_free(void *ptr, size_t boyut);

/* ─────────────────────────────────────────────────────────────────────────────
 *  Sıfırlama & Raporlama
 * ─────────────────────────────────────────────────────────────────────────────
 */
void izci_sifirla(void);
size_t aktif_bellek(void);
void bellek_raporu_yazdir(void);

/* ─────────────────────────────────────────────────────────────────────────────
 *  Getter Fonksiyonları — Test & doğrulama amaçlı
 * ─────────────────────────────────────────────────────────────────────────────
 */
int izci_malloc_sayisi(void);
int izci_free_sayisi(void);
size_t izci_toplam_ayrildi(void);
size_t izci_toplam_serbest(void);

#endif /* BELLEK_IZCI_H */
