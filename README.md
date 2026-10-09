# ESP32 Paint

ESP32 ve TFT dokunmatik ekran kullanılarak geliştirilen çizim programı ve web albümü.
⚠️ BMP Satır Dolgusu (Padding) ve Hizalama Notu 105. satırdan bakabirsiniz.

# Nasıl Çalışır?

Proje, ESP32 mikrodenetleyicisi ile web tarayıcısının birlikte çalışmasına dayanır. ESP32, bir web sunucusu başlatarak kullanıcının çizimlerini web arayüzü üzerinden görüntülemesine ve indirmesine olanak tanır. Kullanıcı, ESP32'nin ekranında çizim yapabilir ve web arayüzündeki butonlar aracılığıyla çizimini canlı olarak görüntüleyebilir veya indirebilir.

## Çizim Kısmı

ESP32 ve TFT ekranın sol tarafında renk seçme paleti bulunur. Kullanıcı bir renge dokunduğunda, dokunmatik ekranın X ve Y koordinatları hesaplanarak hangi butona basıldığı belirlenir. Böylece kullanıcı istediği rengi seçebilir.

**Clear** butonuna basıldığında çizim alanındaki tüm pikseller beyaza boyanır ve ekran başlangıçtaki boş hâline döner.

## Canlı Önizleme ve Web Galerisi

ESP32, ilk olarak kendi Wi-Fi ağını oluşturur. Ağ, `esp32_proje` adıyla başlatılır ve ESP32 üzerinde bir HTTP web sunucusu çalıştırılır.

Kullanıcı, tarayıcısına ESP32'nin IP adresini girdiğinde, ESP32'nin flash belleğinde saklanan HTML, CSS ve JavaScript kodları tarayıcıya gönderilir.

Resmi aktarmak için web sitesindeki **Ekranı Çek** butonuna basıldığında tarayıcı, ESP32'den kullanıcının çizimini BMP formatında ister. ESP32, çizim alanındaki pikselleri sırayla okur ve yaklaşık 1 KB'lık bir tampon bellek kullanarak görüntü verilerini ağ üzerinden tarayıcıya aktarır.

ESP32'nin RAM kapasitesi sınırlı olduğundan görüntü verisi tek seferde bellekte tutulmaz; bunun yerine veriler parça parça aktarılır.

**İndir** butonuna basıldığında ise tarayıcı, görüntüyü yeniden ESP32'den istemek yerine mevcut resim URL'sini kullanarak indirme işlemini başlatmayı dener. JavaScript'teki `appendChild()` yöntemiyle yeni bir resim kartı web galerisine eklenir. Bu işlem, galeri öğesinin tarayıcı tarafında oluşturulmasını sağlar ve tek başına ESP32'de ek bir resim saklama işlemi gerektirmez.

# Donanım Gereksinimleri

* ESP32 geliştirme kartı
* ILI9341 veya benzeri SPI dokunmatik TFT ekran
* Bağlantı kabloları
* 
### 2. TFT_eSPI Kütüphane Ayarı
`TFT_eSPI` kütüphanesinin ekranınızla doğru haberleşebilmesi için pin ayarlarını yapmanız gerekir:

1. `Documents/Arduino/libraries/TFT_eSPI/User_Setup.h` dosyasını bir metin düzenleyici ile açın.
2. Ekran sürücünüzü seçin (Örn: `#define ILI9341_DRIVER`).
3. ESP32 ve TFT ekran pin bağlantılarını kendi devrenize göre tanımlayın:

```cpp
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
#define TOUCH_CS  5
```
# Kodun Detaylı İncelemesi

## Renkler Neden Sayılarla Yazılmış?

Kullandığım TFT ekran, renkleri 16 bitlik RGB565 formatında işler. Bu nedenle renkler, onaltılık (hexadecimal) tabanda ifade edilen sayısal değerlerle tanımlanır.

```cpp
#define RENK_KIRMIZI 0xF800
#define RENK_YESIL   0x07E0
#define RENK_MAVI    0x001F
#define RENK_SARI    0xFFE0
#define RENK_MOR     0x780F
#define RENK_TURUNCU 0xFD20
#define RENK_PEMBE   0xFC18
#define RENK_BEYAZ   0xFFFF
#define RENK_SIYAH   0x0000
```

## RGB565 Formatı Nedir ve Nasıl Çalışır?

Bir görüntüdeki her pikselin rengi farklı biçimlerde saklanabilir. Örneğin, 24 bitlik RGB formatında bir piksel 3 bayt bellek kullanır. Ancak ESP32 gibi mikrodenetleyicilerde bellek kullanımını ve veri aktarımını azaltmak amacıyla RGB565 formatı kullanılabilir. Bu formatta her piksel 16 bit, yani 2 bayt olarak saklanır.

RGB565 formatında renk kanalları şu şekilde kodlanır:

* **Kırmızı:** 5 bit
* **Yeşil:** 6 bit
* **Mavi:** 5 bit

Toplamda 16 bit kullanılır. Böylece pikseller daha az bellek kullanılarak temsil edilebilir ve ekranın renkli görüntü oluşturması sağlanır.

## BMP Başlık (Header) Verilerinin Hazırlanması

Tarayıcının kendisine gönderilen veriyi bir resim dosyası olarak yorumlayabilmesi için BMP formatına uygun bir başlık hazırlanır. Standart BMP başlığı, 14 baytlık File Header ve 40 baytlık Info Header olmak üzere toplam 54 bayttır.

Tuvalin genişliği `w`, yüksekliği ise `h` değişkeniyle hesaplanır.

### Tuval Boyutlarının Hesaplanması (`w` ve `h`)

```cpp
uint16_t w = tft.width() - PALET_GENISLIK;
uint16_t h = tft.height();
```

Burada `tft.width()` ekranın genişliğini, `tft.height()` ise yüksekliğini verir. `PALET_GENISLIK` değeri çıkarılarak yalnızca çizim alanının genişliği hesaplanır. Böylece sol taraftaki renk paleti görüntü aktarımının dışında tutulur.

### BMP Dosya ve Bilgi Başlıkları

BMP formatının standart yapısına uygun iki başlık dizisi oluşturulur. Bu diziler dosya biçimi, görüntü boyutları ve renk derinliği gibi bilgileri içerir.

```cpp
uint8_t bmpFileHeader[14] = {'B', 'M', ...};
uint8_t bmpInfoHeader[40] = {40, 0, 0, 0, ...};
```

`bmpFileHeader`, dosya başlığını; `bmpInfoHeader` ise görüntüye ait bilgileri tutar. Başlıkların içindeki değerler, BMP formatının gerektirdiği biçime göre düzenlenir.

### Bit Kaydırma (Bit Shifting)

`filesize` ve `w` gibi değerler birden fazla baytla temsil edilebilir. BMP başlığında bu değerlerin baytları, little-endian sıralamasına uygun şekilde yazılır.

Aşağıdaki kodda `>> 8`, `>> 16` ve `>> 24` işlemleri, sayının bitlerini sağa kaydırarak farklı baytlarının elde edilmesini sağlar. `(uint8_t)` dönüşümü ise ilgili değerin en düşük 8 bitini alır.

```cpp
bmpFileHeader[2] = (uint8_t)(filesize);
bmpFileHeader[3] = (uint8_t)(filesize >> 8);
bmpFileHeader[4] = (uint8_t)(filesize >> 16);
bmpFileHeader[5] = (uint8_t)(filesize >> 24);
```

Bu sayede dosya boyutu, BMP başlığında dört ayrı bayt hâlinde saklanır.

### ⚠️ BMP Satır Dolgusu (Padding) ve Hizalama Notu

BMP standartları gereği, her resim satırının genişliği (bayt cinsinden) **4'ün katı** olmak zorundadır. 

* Eğer $(w \times 3)$ değeri 4'ün katı değilse, satır sonuna `(4 - ((w * 3) % 4)) % 4` kadar sıfır baytı (padding) eklenmelidir.
* Bu projede varsayılan ekran yapısında tuval genişliği $w = 275$ pikseldir ($275 \times 3 = 825$ bayt). $825$ bayt 4'ün katı olmadığı için tam uyumluluk adına satır sonuna **3 bayt dolgu (padding)** eklenerek satır genişliği 828 bayta tamamlanır.

---

### 4. Düşük RAM Kullanımlı Satır Satır BMP Akışı (`handleScreenBMP`)

Tam ekranlık bir bellek tamponu (framebuffer) oluşturmak ESP32'nin RAM'ini tüketebileceği için **satır satır okuma ve akış (streaming)** yöntemi uygulanır.

* **Tampon Bellek Boyutu (Buffer Size):**  
  Tuval genişliği $w = 275$ piksel için her piksel 3 bayt (RGB) kaplar.  
  $$\text{Tampon Boyutu} = 275 \times 3 = 825 \text{ Bayt}$$  
  *(Padding ile birlikte her satır için ağa toplam 828 bayt gönderilir).*
