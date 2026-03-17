/*
 * ============================================================================
 *  MiniStream — Hash Map
 *  hash_map.h — Hash map arayüzü
 * ============================================================================
 */

#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "ministream.h"

/* ─────────────────────────────────────────────────────────────────────────────
 *  Hash Map İşlemleri — Chaining, O(1) ortalama
 * ─────────────────────────────────────────────────────────────────────────────
 */
HashMap *hashmap_olustur(void);
void hashmap_ekle(HashMap *map, Sarki *sarki);
Sarki *sarki_ara_map(HashMap *map, int id);
void hashmap_temizle(HashMap *map);

#endif /* HASH_MAP_H */
