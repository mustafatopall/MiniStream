/*
 * ============================================================================
 *  MiniStream — Bellek Verimli Müzik Kütüphanesi
 *  ministream.c — Tüm temel operasyonlar, arama yapıları, CSV yükleme
 * ============================================================================
 */

#include "ministream.h"
#include "bellek_izci.h"
#include "hash_map.h"
#include <time.h>

/* ═══════════════════════════════════════════════════════════════════════════
 *  BÖLÜM 1 — Şarkı İşlemleri
 * ═══════════════════════════════════════════════════════════════════════════
 */

/*
 * sarki_olustur — Heap'te yeni bir Sarki nesnesi oluşturur.
 * ref_sayisi = 0, sonraki = NULL ile başlatılır.
 */
Sarki *sarki_olustur(int id, const char *baslik, const char *sanatci,
                     const char *album, int sure_sn, int yil) {
  Sarki *yeni = (Sarki *)izlenen_malloc(sizeof(Sarki));
  if (yeni == NULL)
    return NULL;

  yeni->id = id;
  strncpy(yeni->baslik, baslik, BASLIK_MAX - 1);
  yeni->baslik[BASLIK_MAX - 1] = '\0';
  strncpy(yeni->sanatci, sanatci, SANATCI_MAX - 1);
  yeni->sanatci[SANATCI_MAX - 1] = '\0';
  strncpy(yeni->album, album, ALBUM_MAX - 1);
  yeni->album[ALBUM_MAX - 1] = '\0';

  yeni->sure_sn = sure_sn;
  yeni->yil = yil;
  yeni->ref_sayisi = 0;
  yeni->sonraki = NULL;

  return yeni;
}

/*
 * sarki_sil — Şarkıyı bellekten siler.
 * Dönüş:  0 = başarılı silme
 *        -1 = reddedildi (ref_sayisi > 0, hâlâ kullanımda)
 */
int sarki_sil(Sarki *sarki) {
  if (sarki == NULL)
    return -1;

  if (sarki->ref_sayisi > 0) {
    return -1; /* Hâlâ bir veya daha fazla listede referans var */
  }

  izlenen_free(sarki, sizeof(Sarki));
  return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  BÖLÜM 2 — Çalma Listesi İşlemleri
 * ═══════════════════════════════════════════════════════════════════════════
 */

/*
 * liste_olustur — Yeni çalma listesi oluşturur.
 * Başlangıç kapasitesi 10 şarkı pointer'ı.
 */
CalmaListesi *liste_olustur(int id, const char *isim) {
  CalmaListesi *liste = (CalmaListesi *)izlenen_malloc(sizeof(CalmaListesi));
  if (liste == NULL)
    return NULL;

  liste->id = id;
  strncpy(liste->isim, isim, ISIM_MAX - 1);
  liste->isim[ISIM_MAX - 1] = '\0';

  liste->kapasite = 10;
  liste->sarki_sayisi = 0;
  liste->sarkilar =
      (Sarki **)izlenen_malloc(sizeof(Sarki *) * (size_t)liste->kapasite);

  if (liste->sarkilar == NULL) {
    izlenen_free(liste, sizeof(CalmaListesi));
    return NULL;
  }

  return liste;
}

/*
 * liste_sarki_ekle — Listeye şarkı pointer'ı ekler (kopya değil!).
 * Kapasite doluysa 2x büyütür. ref_sayisi artırılır.
 * Dönüş:  0 = başarılı, -1 = hata
 */
int liste_sarki_ekle(CalmaListesi *liste, Sarki *sarki) {
  if (liste == NULL || sarki == NULL)
    return -1;

  /* Kapasite kontrolü — doluysa 2x büyüt */
  if (liste->sarki_sayisi >= liste->kapasite) {
    int yeni_kap = liste->kapasite * 2;
    size_t eski_boyut = sizeof(Sarki *) * (size_t)liste->kapasite;
    size_t yeni_boyut = sizeof(Sarki *) * (size_t)yeni_kap;

    Sarki **yeni_dizi =
        (Sarki **)izlenen_realloc(liste->sarkilar, eski_boyut, yeni_boyut);
    if (yeni_dizi == NULL)
      return -1;

    liste->sarkilar = yeni_dizi;
    liste->kapasite = yeni_kap;
  }

  /* Pointer ekle — veri kopyalanmaz! */
  liste->sarkilar[liste->sarki_sayisi] = sarki;
  liste->sarki_sayisi++;
  sarki->ref_sayisi++;

  return 0;
}

/*
 * liste_sarki_cikar — İndeksteki şarkıyı listeden çıkarır.
 * ÖNCE ref_sayisi azaltılır, SONRA son elemanla swap yapılır (O(1)).
 */
void liste_sarki_cikar(CalmaListesi *liste, int idx) {
  if (liste == NULL)
    return;
  if (idx < 0 || idx >= liste->sarki_sayisi)
    return;

  /* ÖNCE ref_sayisi azalt */
  liste->sarkilar[idx]->ref_sayisi--;

  /* Son elemanla swap (O(1) çıkarma) */
  liste->sarki_sayisi--;
  liste->sarkilar[idx] = liste->sarkilar[liste->sarki_sayisi];
}

/*
 * liste_temizle — Listeyi tamamen temizler.
 * Her şarkının ref_sayisi azaltılır; ref_sayisi==0 ise sarki_sil() çağrılır.
 * Pointer dizisi ve liste yapısı serbest bırakılır.
 */
void liste_temizle(CalmaListesi *liste) {
  if (liste == NULL)
    return;

  for (int i = 0; i < liste->sarki_sayisi; i++) {
    if (liste->sarkilar[i] != NULL) {
      liste->sarkilar[i]->ref_sayisi--;
      if (liste->sarkilar[i]->ref_sayisi == 0) {
        sarki_sil(liste->sarkilar[i]);
      }
    }
  }

  izlenen_free(liste->sarkilar, sizeof(Sarki *) * (size_t)liste->kapasite);
  izlenen_free(liste, sizeof(CalmaListesi));
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  BÖLÜM 5 — CSV Yükleme
 * ═══════════════════════════════════════════════════════════════════════════
 */

/*
 * csv_alan_oku — CSV satırından bir alan okur.
 * Tırnak içindeki virgülleri doğru şekilde ele alır.
 * ptr:     satır içindeki mevcut konum (güncellenir)
 * hedef:   okunan değerin yazılacağı tampon
 * max_boy: tampon boyutu
 */
static void csv_alan_oku(const char **ptr, char *hedef, int max_boy) {
  const char *p = *ptr;
  int i = 0;

  /* Başlangıç boşlukları atla */
  while (*p == ' ' || *p == '\t')
    p++;

  if (*p == '"') {
    /* Tırnaklı alan */
    p++; /* Açılış tırnağını atla */
    while (*p != '\0') {
      if (*p == '"') {
        if (*(p + 1) == '"') {
          /* Kaçırılmış tırnak ("") */
          if (i < max_boy - 1)
            hedef[i++] = '"';
          p += 2;
        } else {
          /* Kapanış tırnağı */
          p++;
          break;
        }
      } else {
        if (i < max_boy - 1)
          hedef[i++] = *p;
        p++;
      }
    }
    /* Virgülü atla */
    if (*p == ',')
      p++;
  } else {
    /* Normal alan */
    while (*p != ',' && *p != '\n' && *p != '\r' && *p != '\0') {
      if (i < max_boy - 1)
        hedef[i++] = *p;
      p++;
    }
    if (*p == ',')
      p++;
  }

  hedef[i] = '\0';
  *ptr = p;
}

/*
 * sanatci_ayikla — "['Sanatci Adi']" formatındaki stringden sanatçı adını
 * çıkarır. İlk tırnak içindeki değeri alır.
 */
static void sanatci_ayikla(const char *kaynak, char *hedef, int max_boy) {
  const char *p = kaynak;

  /* Tek tırnak veya normal tırnak ara */
  while (*p != '\0' && *p != '\'' && *p != '"')
    p++;

  if (*p == '\'' || *p == '"') {
    char tirnak = *p;
    p++;
    int i = 0;
    while (*p != '\0' && *p != tirnak && i < max_boy - 1) {
      hedef[i++] = *p;
      p++;
    }
    hedef[i] = '\0';
  } else {
    /* Tırnak bulunamadı, olduğu gibi kopyala */
    strncpy(hedef, kaynak, max_boy - 1);
    hedef[max_boy - 1] = '\0';
  }
}

/*
 * csv_yukle — CSV dosyasından şarkıları okur ve linked list olarak döndürür.
 *
 * CSV Sütun Sırası (tracks_features.csv):
 *   0:id, 1:name, 2:album, 3:album_id, 4:artists, 5:artist_ids,
 *   6:track_number, 7:disc_number, 8:explicit, 9:danceability,
 *   10:energy, 11:key, 12:loudness, 13:mode, 14:speechiness,
 *   15:acousticness, 16:instrumentalness, 17:liveness, 18:valence,
 *   19:tempo, 20:duration_ms, 21:time_signature, 22:year, 23:release_date
 *
 * dosya_yolu: CSV dosya yolu
 * limit:      Okunacak maksimum satır (0 = hepsi)
 * toplam:     Okunan satır sayısı (çıktı parametresi)
 *
 * Dönüş: Linked list başı (Sarki*)
 */
Sarki *csv_yukle(const char *dosya_yolu, int limit, int *toplam) {
  FILE *dosya = fopen(dosya_yolu, "r");
  if (dosya == NULL) {
    fprintf(stderr, "[CSV HATA] Dosya acilamadi: %s\n", dosya_yolu);
    return NULL;
  }

  char satir[4096];
  Sarki *bas = NULL;
  Sarki *son = NULL;
  int sayac = 0;
  int satir_no = 0;

  while (fgets(satir, sizeof(satir), dosya) != NULL) {
    satir_no++;

    /* İlk satır (başlık) atla */
    if (satir_no == 1)
      continue;

    /* Limit kontrolü */
    if (limit > 0 && sayac >= limit)
      break;

    const char *ptr = satir;
    char alanlar[24][256];

    /* 24 alanı oku */
    for (int i = 0; i < 24; i++) {
      csv_alan_oku(&ptr, alanlar[i], 256);
    }

    /* İlgili alanları çıkar */
    char baslik[BASLIK_MAX];
    char album[ALBUM_MAX];
    char sanatci_raw[256];
    char sanatci[SANATCI_MAX];

    strncpy(baslik, alanlar[1], BASLIK_MAX - 1); /* name      */
    baslik[BASLIK_MAX - 1] = '\0';
    strncpy(album, alanlar[2], ALBUM_MAX - 1); /* album     */
    album[ALBUM_MAX - 1] = '\0';
    strncpy(sanatci_raw, alanlar[4], 255); /* artists   */
    sanatci_raw[255] = '\0';
    sanatci_ayikla(sanatci_raw, sanatci, SANATCI_MAX);

    int duration_ms = atoi(alanlar[20]); /* duration_ms */
    int yil = atoi(alanlar[22]);         /* year        */

    /* Şarkı oluştur */
    Sarki *yeni = sarki_olustur(sayac + 1, baslik, sanatci, album,
                                duration_ms / 1000, yil);
    if (yeni == NULL)
      continue;

    /* Linked list'e ekle */
    if (bas == NULL) {
      bas = yeni;
      son = yeni;
    } else {
      son->sonraki = yeni;
      son = yeni;
    }
    sayac++;
  }

  fclose(dosya);

  if (toplam != NULL)
    *toplam = sayac;

  printf("[CSV] %d sarki yuklendi: %s\n", sayac, dosya_yolu);
  return bas;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  BÖLÜM 6 — Veri Üretimi (Benchmark Hazırlık)
 * ═══════════════════════════════════════════════════════════════════════════
 */

/*
 * veri_uret_liste — n adet rastgele şarkı üretir, linked list döndürür.
 */
Sarki *veri_uret_liste(int n) {
  Sarki *bas = NULL;
  Sarki *son = NULL;

  for (int i = 0; i < n; i++) {
    char baslik[BASLIK_MAX];
    char sanatci[SANATCI_MAX];
    char album[ALBUM_MAX];

    snprintf(baslik, BASLIK_MAX, "Sarki_%d", i + 1);
    snprintf(sanatci, SANATCI_MAX, "Sanatci_%d", (i % 100) + 1);
    snprintf(album, ALBUM_MAX, "Album_%d", (i % 50) + 1);

    Sarki *yeni = sarki_olustur(i + 1, baslik, sanatci, album, 180 + (i % 300),
                                2000 + (i % 25));
    if (yeni == NULL)
      continue;

    if (bas == NULL) {
      bas = yeni;
      son = yeni;
    } else {
      son->sonraki = yeni;
      son = yeni;
    }
  }
  return bas;
}

/*
 * veri_uret_map — n adet rastgele şarkı üretir, hash map döndürür.
 * Ayrıca şarkılar linked list olarak iç içe zincirlenmiş olarak
 * map'in ilk kovasından erişilebilir (temizlik için).
 */
HashMap *veri_uret_map(int n) {
  HashMap *map = hashmap_olustur();
  if (map == NULL)
    return NULL;

  for (int i = 0; i < n; i++) {
    char baslik[BASLIK_MAX];
    char sanatci[SANATCI_MAX];
    char album[ALBUM_MAX];

    snprintf(baslik, BASLIK_MAX, "Sarki_%d", i + 1);
    snprintf(sanatci, SANATCI_MAX, "Sanatci_%d", (i % 100) + 1);
    snprintf(album, ALBUM_MAX, "Album_%d", (i % 50) + 1);

    Sarki *yeni = sarki_olustur(i + 1, baslik, sanatci, album, 180 + (i % 300),
                                2000 + (i % 25));
    if (yeni == NULL)
      continue;

    hashmap_ekle(map, yeni);
  }
  return map;
}

/*
 * liste_temizle_hepsi — Linked list'teki tüm şarkıları siler.
 */
void liste_temizle_hepsi(Sarki *bas) {
  Sarki *simdiki = bas;
  while (simdiki != NULL) {
    Sarki *sonraki = simdiki->sonraki;
    /* ref_sayisi kontrolü yapmadan direkt sil (linked list temizliği) */
    izlenen_free(simdiki, sizeof(Sarki));
    simdiki = sonraki;
  }
}
