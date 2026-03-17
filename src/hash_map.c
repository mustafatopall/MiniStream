/*
 * ============================================================================
 *  MiniStream — Hash Map
 *  hash_map.c — Hash map implementasyonu (Chaining)
 * ============================================================================
 */

#include "hash_map.h"
#include "bellek_izci.h"

/* ─────────────────────────────────────────────────────────────────────────────
 *  hash_fonksiyonu — Basit modüler hash
 * ─────────────────────────────────────────────────────────────────────────────
 */
static unsigned int hash_fonksiyonu(int id) {
  return (unsigned int)(id >= 0 ? id : -id) % TABLO_BOYUTU;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  hashmap_olustur — Yeni boş hash map oluşturur.
 * ─────────────────────────────────────────────────────────────────────────────
 */
HashMap *hashmap_olustur(void) {
  HashMap *map = (HashMap *)izlenen_malloc(sizeof(HashMap));
  if (map == NULL)
    return NULL;

  for (int i = 0; i < TABLO_BOYUTU; i++) {
    map->kovalar[i] = NULL;
  }
  return map;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  hashmap_ekle — Şarkıyı hash map'e ekler (chaining).
 * ─────────────────────────────────────────────────────────────────────────────
 */
void hashmap_ekle(HashMap *map, Sarki *sarki) {
  if (map == NULL || sarki == NULL)
    return;

  unsigned int idx = hash_fonksiyonu(sarki->id);

  HashNode *yeni = (HashNode *)izlenen_malloc(sizeof(HashNode));
  if (yeni == NULL)
    return;

  yeni->sarki = sarki;
  yeni->sonraki = map->kovalar[idx];
  map->kovalar[idx] = yeni;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  sarki_ara_map — Hash map'te ID'ye göre arama (O(1) ortalama).
 * ─────────────────────────────────────────────────────────────────────────────
 */
Sarki *sarki_ara_map(HashMap *map, int id) {
  if (map == NULL)
    return NULL;

  unsigned int idx = hash_fonksiyonu(id);
  HashNode *dugum = map->kovalar[idx];

  while (dugum != NULL) {
    if (dugum->sarki->id == id)
      return dugum->sarki;
    dugum = dugum->sonraki;
  }
  return NULL;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  hashmap_temizle — Hash map'i ve tüm node'ları temizler.
 *  NOT: Şarkılar silinmez! Onlar ayrı yönetilir.
 * ─────────────────────────────────────────────────────────────────────────────
 */
void hashmap_temizle(HashMap *map) {
  if (map == NULL)
    return;

  for (int i = 0; i < TABLO_BOYUTU; i++) {
    HashNode *dugum = map->kovalar[i];
    while (dugum != NULL) {
      HashNode *sonraki = dugum->sonraki;
      izlenen_free(dugum, sizeof(HashNode));
      dugum = sonraki;
    }
  }
  izlenen_free(map, sizeof(HashMap));
}
