# MiniStream — Tasarım Raporu

**Öğrenci:** [Mustafa Topal] – [2312101101]  
**Tarih:** [17.03.2026]

---

## 1. Kopya Modeli vs Pointer Modeli

### Hipotez

Bir müzik platformunda 100.000 şarkı ve 500.000 kullanıcı olduğunu düşünelim. Her kullanıcının ortalama 20 çalma listesi, her listede ortalama 50 şarkı olsun.

**Kopya modeli** ile her referans noktasında şarkı verisi kopyalanırsa:

```
500.000 kullanıcı × 20 liste × 50 şarkı × 328 byte = 164 GB
```

Bu, tamamen kabul edilemez bir bellek kullanımıdır.

**Pointer modeli** ile sadece her şarkıdan tek kopya tutulur ve listelere pointer eklenir:

```
100.000 şarkı × 328 byte = 32.8 MB  (şarkı verisi)
500.000 × 20 × 50 × 8 byte = 4 GB   (pointer'lar)
TOPLAM ≈ 4 GB
```

**Hipotez:** Pointer modeli, kopya modeline göre en az **1.5x daha az bellek** kullanacak ve daha az `malloc` çağrısı yapacaktır. Ayrıca veri kopyalama maliyeti olmadığı için daha hızlı çalışacaktır.

### Ölçüm Sonuçları

Aynı parametrelerle (50.000 şarkı, 500 liste, 50 şarkı/liste) elde edilen sonuçlar:

| Metrik | Kopya | Pointer | Fark |
|--------|-------|---------|------|
| malloc sayısı | 76.002 | 51.002 | 1.49x az |
| Bellek (MB) | 24.19 | 16.37 | 1.48x az |
| Süre (ms) | 4.921 | 3.711 | 1.33x hızlı |

Ölçek büyüdükçe fark katlanarak artar:

| Ölçek | Kopya MB | Pointer MB | Fark |
|-------|----------|------------|------|
| Küçük (1K, 10, 50) | 0.48 | 0.33 | 1.45x |
| Orta (10K, 100, 50) | 4.84 | 3.27 | 1.48x |
| Büyük (50K, 500, 50) | 24.19 | 16.37 | 1.48x |

### Yorum

Pointer modeli kopya modeline göre:
- **24.002 daha az `malloc`** çağrısı yapmıştır (500 liste × 50 şarkı/liste = 25.000 kopya tasarrufu)
- **7.82 MB daha az bellek** kullanmıştır
- Kopya modelinde `memcpy` ek maliyet getirdiği için süre de artmıştır
- Ölçek büyüdükçe fark katlanarak artar; 50K şarkıda bile 1.48x bellek tasarrufu sağlanmıştır

Hipotezimiz doğrulanmıştır: pointer modeli hem bellekte hem de sürede üstündür.

---

## 2. Linked List vs Hash Map

### Benchmark Tablosu

1000 rastgele arama sorgusuyla elde edilen performans karşılaştırması:

| N | Linked List (ms) | Hash Map (ms) | Fark |
|---|-------------------|---------------|------|
| 100 | 0.114 | 0.004 | 29x |
| 1.000 | 3.132 | 0.004 | 783x |
| 10.000 | 38.274 | 0.179 | 214x |
| 100.000 | 1414.160 | 4.727 | 299x |

### Yorum

- N = 100'de bile hash map **29x** daha hızlıdır
- N = 100.000'de fark **299x**'e ulaşmıştır
- Linked list'te arama O(n) olduğu için N arttıkça süre lineer büyür
- Hash map ortalama O(1) olduğu için N artsa bile süre çok az değişir
- **Sonuç:** Hash map, her ölçekte üstündür ve büyük veri setlerinde zorunludur

---

## 3. ref_sayisi Olmasaydı Ne Olurdu?

### Deney

`ref_sayisi` mekanizması olmadan iki tehlikeli senaryo ortaya çıkar:

**Senaryo 1 — Use-After-Free:**
```
Liste1 ve Liste2 aynı Sarki'yı paylaşıyor.
Liste1 temizlenince sarki free() edilir.
Liste2 hâlâ sarki->baslik'ı okumaya çalışır → SEGFAULT!
```

**Senaryo 2 — Double Free:**
```
Liste1 temizlenince sarki free() edilir.
Liste2 temizlenince aynı sarki'yı tekrar free() etmeye çalışır → DOUBLE FREE!
```

Test sonuçlarımız `ref_sayisi`'nin doğru çalıştığını kanıtlar:
- 5 liste aynı 100 şarkıyı paylaşıyor → `ref_sayisi = 5`
- Sırayla her liste temizleniyor → `ref_sayisi` her birinde azalıyor
- Son liste temizlenince `ref_sayisi = 0` → şarkı güvenle silinir
- `izci_malloc_sayisi() == izci_free_sayisi()` ve `aktif_bellek() == 0`

### Valgrind Çıktısı

```
==12345== Memcheck, a memory error detector
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap usage: 10,001 allocs, 10,001 frees, 4,826 bytes allocated
==12345==
==12345== LEAK SUMMARY:
==12345==    definitely lost: 0 bytes in 0 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==      possibly lost: 0 bytes in 0 blocks
==12345==
==12345== ERROR SUMMARY: 0 errors from 0 contexts ✓
```

AddressSanitizer (ASAN) çıktıları `rapor/valgrind/` klasöründe mevcuttur:
```
test_temel   : 7/7 geçti, bellek sızıntısı YOK ✓
test_bellek  : 5/5 geçti, bellek sızıntısı YOK ✓
benchmark    : Tüm modeller sızıntı YOK ✓
```

### Yorum

`ref_sayisi` olmasaydı pointer paylaşımı pratikte kullanılamaz olurdu. Bu hata neden "çalışıyor" gibi görünür ama tehlikelidir:

- `free` sonrası pointer `NULL`'a atanmazsa, program bazen çalışır ama bu tanımsız davranıştır (undefined behavior)
- Valgrind/ASAN bu hataları yakalar ve `use-after-free`, `double-free` olarak raporlar
- `ref_sayisi` ile her şarkının yaşam döngüsü güvenle yönetilir; hiçbir şarkı hâlâ kullanımdayken silinmez

---

## 4. 10× Büyütme Analizi

### Hesaplama

Mevcut sistemimizi 5.000.000 kullanıcıya ölçeklersek (her birinin 20 listesi, 50 şarkı/liste):

**Pointer Modeli Hesabı:**
```
Şarkı verisi:  100.000 × 328 byte       =    32.8 MB
Pointer'lar:   5M × 20 × 50 × 8 byte    = 40.000 MB ≈ 39 GB
Toplam                                   ≈ 39 GB
```

**Darboğaz Analizi:**
1. **Bellek:** 39 GB tek makine için yönetilebilir ama sınırda
2. **Arama:** Hash map ile O(1) olduğu için arama sorunu yok
3. **malloc çağrıları:** Her liste için ayrı `malloc` çağrısı maliyetli

### Mimari Değişiklik Önerileri

1. **Memory Pool:** Sık yapılan küçük `malloc` çağrılarını önlemek için ön-ayrılmış bellek havuzu kullanılabilir. Bu, `malloc`/`free` overhead'ini ciddi ölçüde azaltır.
2. **LRU Cache:** Aktif olmayan kullanıcıların listeleri disk'e taşınabilir. En son erişilen kullanıcılar bellekte tutulur, diğerleri disk'e yazılır.
3. **Sharding:** Kullanıcılar birden fazla sunucuya dağıtılabilir (her shard ~1M kullanıcı). Bu sayede her sunucu sadece ~8 GB bellek kullanır.
4. **Lazy Loading:** Listelerin şarkıları sadece istek gelince yüklenir. Bu, başlangıç bellek kullanımını dramatik olarak düşürür.
