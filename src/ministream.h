/*
 * ============================================================================
 *  MiniStream — Bellek Verimli Müzik Kütüphanesi
 *  ministream.h — Veri modeli tanımları ve fonksiyon imzaları
 * ============================================================================
 */

#ifndef MINISTREAM_H
#define MINISTREAM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─────────────────────────────────────────────────────────────────────────────
 *  Sabitler
 * ─────────────────────────────────────────────────────────────────────────────
 */
#define BASLIK_MAX 100
#define SANATCI_MAX 100
#define ALBUM_MAX 100
#define ISIM_MAX 50
#define TABLO_BOYUTU 1024 /* Hash map kova sayısı */

/* ─────────────────────────────────────────────────────────────────────────────
 *  Veri Yapıları
 * ─────────────────────────────────────────────────────────────────────────────
 */

/* Sarki — Linked list düğümü olarak da kullanılır (sonraki alanı ile) */
typedef struct Sarki {
  int id;
  char baslik[BASLIK_MAX];
  char sanatci[SANATCI_MAX];
  char album[ALBUM_MAX];
  int sure_sn;           /* duration_ms / 1000  */
  int yil;               /* Çıkış yılı          */
  int ref_sayisi;        /* Kaç listede var?    */
  struct Sarki *sonraki; /* Linked list bağı    */
} Sarki;

/* CalmaListesi — Şarkı pointer dizisi ile yönetilir */
typedef struct {
  int id;
  char isim[ISIM_MAX];
  Sarki **sarkilar; /* Heap'te pointer dizisi (Sarki** !) */
  int sarki_sayisi;
  int kapasite; /* Başlangıç: 10       */
} CalmaListesi;

/* Kullanici — CalmaListesi pointer dizisi ile yönetilir */
typedef struct {
  int id;
  char isim[ISIM_MAX];
  CalmaListesi **listeler;
  int liste_sayisi;
} Kullanici;

/* HashNode — Chaining yöntemi için düğüm */
typedef struct HashNode {
  Sarki *sarki;
  struct HashNode *sonraki;
} HashNode;

/* HashMap — Sabit boyutlu kova dizisi */
typedef struct {
  HashNode *kovalar[TABLO_BOYUTU];
} HashMap;

/* ─────────────────────────────────────────────────────────────────────────────
 *  Sarki İşlemleri
 * ─────────────────────────────────────────────────────────────────────────────
 */
Sarki *sarki_olustur(int id, const char *baslik, const char *sanatci,
                     const char *album, int sure_sn, int yil);
int sarki_sil(Sarki *sarki);

/* ─────────────────────────────────────────────────────────────────────────────
 *  CalmaListesi İşlemleri
 * ─────────────────────────────────────────────────────────────────────────────
 */
CalmaListesi *liste_olustur(int id, const char *isim);
int liste_sarki_ekle(CalmaListesi *liste, Sarki *sarki);
void liste_sarki_cikar(CalmaListesi *liste, int idx);
void liste_temizle(CalmaListesi *liste);

/* ─────────────────────────────────────────────────────────────────────────────
 *  Arama fonksiyonları — ayrı header dosyalarında
 *  #include "linked_list.h"  →  sarki_ara_liste()
 *  #include "hash_map.h"     →  hashmap_olustur/ekle/ara/temizle()
 * ─────────────────────────────────────────────────────────────────────────────
 */

/* ─────────────────────────────────────────────────────────────────────────────
 *  CSV Yükleme
 * ─────────────────────────────────────────────────────────────────────────────
 */
Sarki *csv_yukle(const char *dosya_yolu, int limit, int *toplam);

/* ─────────────────────────────────────────────────────────────────────────────
 *  Veri Üretimi (Benchmark İçin)
 * ─────────────────────────────────────────────────────────────────────────────
 */
Sarki *veri_uret_liste(int n);
HashMap *veri_uret_map(int n);
void liste_temizle_hepsi(Sarki *bas);

#endif /* MINISTREAM_H */
