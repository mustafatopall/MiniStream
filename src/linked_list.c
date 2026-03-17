/*
 * ============================================================================
 *  MiniStream — Linked List Arama
 *  linked_list.c — Linked list arama implementasyonu
 * ============================================================================
 */

#include "linked_list.h"

/* ─────────────────────────────────────────────────────────────────────────────
 *  sarki_ara_liste — ID'ye göre linked list'te lineer arama.
 *  Bulursa Sarki* döndürür, bulamazsa NULL.
 * ─────────────────────────────────────────────────────────────────────────────
 */
Sarki *sarki_ara_liste(Sarki *bas, int id) {
  Sarki *simdiki = bas;
  while (simdiki != NULL) {
    if (simdiki->id == id)
      return simdiki;
    simdiki = simdiki->sonraki;
  }
  return NULL;
}
