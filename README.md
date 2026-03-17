# MiniStream

MiniStream, C dilinde yazılmış bellek verimli bir müzik kütüphanesi simülasyonudur. Şarkılar gibi büyük veri setlerini yönetirken geleneksel kopya modeli yerine pointer (işaretçi) tabanlı bir model kullanmanın avantajlarını gösterir. Ayrıca, şarkı arama işlemleri için Linked List (Bağlı Liste) ve Hash Map (Özet Tablosu) yapılarının performansını karşılaştırır.

## Veri Modelleri

Proje, bellek yönetimi için iki yaklaşımı değerlendirir:
*   **Kopya Modeli:** Veriler farklı listelere eklenirken tamamen kopyalanır. Bu, gereksiz ve aşırı bellek tüketimine yol açar (O(N*M) bellek yükü simüle edilir).
*   **Pointer Modeli:** Şarkı verisi bellekte sadece bir kez oluşturulur ve çalma listelerine sadece bu verinin adresi (pointer) eklenir. Şarkının kullanım durumunu güvenle takip etmek ve hiçbir listede kalmadığında belleği serbest bırakmak için bir referans sayacı (`ref_sayisi`) mekanizması kullanır.

Ayrıca iki arama yapısını karşılaştırır:
*   **Linked List:** O(n) doğrusal zamanlı arama.
*   **Hash Map:** O(1) ortalama zamanlı, son derece hızlı arama (çakışmalar için chaining yöntemi kullanılmıştır).

## Proje Yapısı

*   `src/ministream.c` / `ministream.h`: Şarkı oluşturma, liste yönetimi ve güvenli bellek silme mantığını içeren temel kodlar.
*   `src/linked_list.c` / `linked_list.h`: Şarkı arama için Linked List yapısı.
*   `src/hash_map.c` / `hash_map.h`: Şarkı arama için Hash Map yapısı.
*   `src/bellek_izci.c` / `bellek_izci.h`: Bellek sızıntılarını tespit etmek için `malloc` ve `free` çağrılarını sayan özel bellek takipçisi.
*   `data/sarkilar.csv`: 10.000 gerçek Spotify şarkısını içeren veri seti.
*   `test/`: Birim (unit) testleri, bellek sızıntısı testleri ve performans benchmark kodları.

## Temel Fonksiyonlar

*   `sarki_olustur`: Yeni bir şarkı için bellek ayırır ve referans sayacını (`ref_sayisi`) 0 olarak başlatır.
*   `sarki_sil`: Parçayı bellekten siler (Ancak sadece `ref_sayisi` 0 ise).
*   `liste_sarki_ekle`: Bir çalma listesine şarkının adresini (pointer) ekler ve şarkının referans sayacını 1 artırır. Liste dolarsa kapasiteyi dinamik olarak ikiye katlar.
*   `liste_sarki_cikar`: Şarkıyı listeden çıkarır ve referans sayacını 1 azaltır.
*   `liste_temizle`: Listeyi temizler ve içindeki tüm şarkıların referans sayısını güvenli bir şekilde düşürür (0 olanlar otomatik silinir).
*   `csv_yukle`: `data/sarkilar.csv` dosyasını okuyup verileri belleğe yükler.
*   `sarki_ara_liste`: Linked list kullanarak ID'ye göre şarkı arar.
*   `sarki_ara_map`: Hash map kullanarak ID'ye göre şarkı arar.

## Kurulum ve Çalıştırma

Proje, kolay derleme için bir Makefile içerir.

### Gereksinimler
*   GCC veya Clang derleyici
*   Make
*   Valgrind veya AddressSanitizer (Bellek kontrolleri için)

### Projeyi Derlemek
```bash
make
```

### Testleri Çalıştırmak
Veri yapılarının doğru çalıştığını kanıtlayan temel testleri yürütür.
```bash
make test
```

### Benchmark (Performans Karşılaştırması) Çalıştırmak
Linked List vs Hash Map ve Kopya Modeli vs Pointer Modeli hız/bellek testlerini çalıştırır.
```bash
make benchmark
```

### Bellek Sızıntı Kontrolü (ASAN)
Projeyi AddressSanitizer ile derler ve bellek sızıntısı olup olmadığını detaylıca kontrol eder.
```bash
make valgrind
```

### Temizlik
Derlenmiş dosyaları silmek için kullanılır.
```bash
make clean
```
