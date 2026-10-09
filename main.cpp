#include <WiFi.h>
#include <WebServer.h>
#include <TFT_eSPI.h>

const char* ssid = "esp32_proje";
const char* password = "parola1234";
WebServer server(80);
TFT_eSPI tft = TFT_eSPI();

#define RENK_KIRMIZI 0xF800  
#define RENK_YESIL   0x07E0  
#define RENK_MAVI    0x001F  
#define RENK_SARI    0xFFE0  
#define RENK_MOR     0x780F  
#define RENK_TURUNCU 0xFD20  
#define RENK_PEMBE   0xFC18  
#define RENK_BEYAZ   0xFFFF  
#define RENK_SIYAH   0x0000


const int PALET_GENISLIK = 45;
int bolmeYuksekligi;            
int aktifRenk = RENK_KIRMIZI;  
int fircaCap = 3;               

const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Fotoğraf Albümü</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background-color: #000000; padding: 20px; }
        h1 { color: #333; }
        
        .ana-konteyner {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 30px;
            margin-top: 20px;
        }

        .ekran-kutusu { 
            padding: 15px; 
            background: white; 
            border-radius: 10px; 
            box-shadow: 0 4px 8px rgba(0,0,0,0.1); 
            width: 340px;
            height: fit-content;
        }
        
        .galeri-alani {
            background: #e9e9f0;
            padding: 20px;
            border-radius: 10px;
            min-width: 360px;
            max-width: 800px;
            flex: 1;
            box-shadow: inset 0 2px 5px rgba(0,0,0,0.05);
        }

        .galeri-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(160px, 1fr));
            gap: 15px;
            margin-top: 15px;
        }

        .galeri-kutu {
            background: white;
            padding: 8px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            text-align: center;
        }
        
        .galeri-kutu h4 { margin: 0 0 5px 0; color: #666; font-size: 14px; }
        
        img { border: 2px solid #ccc; width: 100%; height: auto; display: block; background-color: #eee; }
        .ekran-kutusu img { margin-bottom: 15px; }
        
        .btn-grup { display: flex; gap: 10px; justify-content: center; }
        .btn { color: white; padding: 12px 15px; font-size: 14px; border: none; border-radius: 5px; cursor: pointer; flex: 1; font-weight: bold; }
        .btn-yenile { background-color: #008CBA; }
        .btn-yenile:hover { background-color: #007B9A; }
        .btn-indir { background-color: #4CAF50; }
        .btn-indir:hover { background-color: #45a049; }
        .btn-galeri-indir { background-color: #ff9800; padding: 5px 10px; font-size: 12px; margin-top: 5px; width: 100%; }
        .btn-galeri-indir:hover { background-color: #e68a00; }
        
        .bos-mesaj { color: #999; font-style: italic; padding: 40px 0; }
    </style>
</head>
<body>
    <h1>ESP32 Albüm</h1>
    <p>Çizdiğiniz resimleri indirin.</p>
    
    <div class="ana-konteyner">
        
        <!-- SOL TARAF: CANLI EKRAN -->
        <div class="ekran-kutusu">
            <h3 style="margin-top:0; color:#333;">Canlı TFT Ekran</h3>
            <img id="espEkran" src="/live_screen.bmp" alt="Yükleniyor...">
            
            <div class="btn-grup">
                <button class="btn btn-yenile" id="btnYenile" onclick="galeriyiYenile()">Ekranı Çek ↻</button>
                <button class="btn btn-indir" onclick="indirVeAlbumeEkle()">İndir</button>
            </div>
        </div>

        <!-- SAĞ TARAF: ALBÜM -->
        <div class="galeri-alani">
            <h3 style="margin-top:0; color:#333; text-align:left;">Fotoğraf Albümüm</h3>
            <div id="bosUyarisi" class="bos-mesaj">Albüm henüz boş. Resim indirdikçe burası dolacak...</div>
            <div class="galeri-grid" id="galeriGrid"></div>
        </div>

    </div>

    <script>
        var resimSayaci = 0;

        function galeriyiYenile() {
            var btn = document.getElementById('btnYenile');
            btn.disabled = true;
            btn.innerText = "Yükleniyor...";

            var zamanDamgasi = new Date().getTime();
            var img = document.getElementById('espEkran');
            
            img.src = '/live_screen.bmp?t=' + zamanDamgasi;

            img.onload = function() {
                btn.disabled = false;
                btn.innerText = "Ekranı Çek ↻";
            };
        }

        function indirVeAlbumeEkle() {
            var canliImg = document.getElementById('espEkran');
            var galeriGrid = document.getElementById('galeriGrid');
            var bosUyarisi = document.getElementById('bosUyarisi');

            if (resimSayaci === 0) {
                bosUyarisi.style.display = 'none';
            }

            resimSayaci++; 
            
            var yeniKutu = document.createElement('div');
            yeniKutu.className = 'galeri-kutu';
            
            yeniKutu.innerHTML = `
                <h4>${resimSayaci}. Resim</h4>
                <img src="${canliImg.src}" alt="${resimSayaci}. Resim">
                <button class="btn btn-galeri-indir" onclick="sadeceBuResmiIndir('${canliImg.src}', 'galeri_resim_${resimSayaci}.bmp')">İndir</button>
            `;
            
            galeriGrid.appendChild(yeniKutu);

            sadeceBuResmiIndir(canliImg.src, 'esp32_cizim_' + resimSayaci + '.bmp');
        }

        function sadeceBuResmiIndir(resimUrl, dosyaAdi) {
            var gizliLink = document.createElement('a');
            gizliLink.href = resimUrl; 
            gizliLink.download = dosyaAdi; 
            
            document.body.appendChild(gizliLink);
            gizliLink.click(); 
            document.body.removeChild(gizliLink);
        }
    </script>
</body>
</html>
)rawliteral";

void arayuzuCiz();


void handleScreenBMP() {

  uint16_t w = tft.width() - PALET_GENISLIK;
  uint16_t h = tft.height();

  uint32_t filesize = 54 + (uint32_t)w * h * 3;
  uint8_t bmpFileHeader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
  uint8_t bmpInfoHeader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};

  bmpFileHeader[ 2] = (uint8_t)(filesize      );
  bmpFileHeader[ 3] = (uint8_t)(filesize >>  8);
  bmpFileHeader[ 4] = (uint8_t)(filesize >> 16);
  bmpFileHeader[ 5] = (uint8_t)(filesize >> 24);

  bmpInfoHeader[ 4] = (uint8_t)(w      );
  bmpInfoHeader[ 5] = (uint8_t)(w >>  8);
  bmpInfoHeader[ 6] = (uint8_t)(w >> 16);
  bmpInfoHeader[ 7] = (uint8_t)(w >> 24);

  int32_t h_neg = -((int32_t)h);
  bmpInfoHeader[ 8] = (uint8_t)(h_neg      );
  bmpInfoHeader[ 9] = (uint8_t)(h_neg >>  8);
  bmpInfoHeader[10] = (uint8_t)(h_neg >> 16);
  bmpInfoHeader[11] = (uint8_t)(h_neg >> 24);

  server.setContentLength(filesize);
  server.send(200, "image/bmp", "");

  WiFiClient client = server.client();
  client.write(bmpFileHeader, 14);
  client.write(bmpInfoHeader, 40);

  uint8_t lineBuffer[w * 3];

  for (uint16_t y = 0; y < h; y++) {
    int bufIdx = 0;
    
    for (uint16_t x = PALET_GENISLIK; x < tft.width(); x++) {
      uint16_t pixel = tft.readPixel(x, y);
      
      uint8_t r = (pixel & 0xF800) >> 8;
      uint8_t g = (pixel & 0x07E0) >> 3;
      uint8_t b = (pixel & 0x001F) << 3;

      lineBuffer[bufIdx++] = b;
      lineBuffer[bufIdx++] = g;
      lineBuffer[bufIdx++] = r;
    }
    client.write(lineBuffer, w * 3);
  }
}

void handleRoot() {
  server.send(200, "text/html", html);
}


void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1); 
  tft.fillScreen(RENK_BEYAZ); 

  
  uint16_t calData[5] = { 275, 3620, 264, 3532, 1 }; 
  tft.setTouch(calData);

  
  bolmeYuksekligi = tft.height() / 9;

  
  arayuzuCiz();

  WiFi.softAP(ssid, password);
  Serial.println("Wi-Fi Agi Baslatildi!");
  Serial.print("ESP32 IP Adresi: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/live_screen.bmp", handleScreenBMP);

  server.begin();
}

void loop() {
  server.handleClient();

  uint16_t x = 0, y = 0;
  
  if (tft.getTouch(&x, &y)) {

   
    if (x < PALET_GENISLIK) {
      int secilenIndex = y / bolmeYuksekligi;

      switch (secilenIndex) {
        case 0: 
          tft.fillRect(PALET_GENISLIK, 0, tft.width() - PALET_GENISLIK, tft.height(), RENK_BEYAZ);
          break;
        case 1: 
          aktifRenk = RENK_BEYAZ;
          fircaCap = 10;
          break;
        case 2: 
          aktifRenk = RENK_PEMBE;
          fircaCap = 3;
          break;
        case 3: 
          aktifRenk = RENK_TURUNCU;
          fircaCap = 3;
          break;
        case 4: 
          aktifRenk = RENK_MOR;
          fircaCap = 3;
          break;
        case 5: 
          aktifRenk = RENK_SARI;
          fircaCap = 3;
          break;
        case 6: 
          aktifRenk = RENK_MAVI;
          fircaCap = 3;
          break;
        case 7: 
          aktifRenk = RENK_YESIL;
          fircaCap = 3;
          break;
        case 8: 
          aktifRenk = RENK_KIRMIZI;
          fircaCap = 3;
          break;
      }
      delay(150); 
    } 
   
    else {
      if (aktifRenk == RENK_BEYAZ) {
        
        tft.fillCircle(x, y, fircaCap, RENK_BEYAZ);
      } else {
        
        tft.fillCircle(x, y, fircaCap, aktifRenk);
      }
    }
  }
}


void arayuzuCiz() {
  
  tft.fillRect(0, 0 * bolmeYuksekligi, PALET_GENISLIK, bolmeYuksekligi, TFT_DARKGREY);
  tft.setTextColor(RENK_BEYAZ);
  tft.setTextSize(1);
  tft.drawString("CLR", 10, 0 * bolmeYuksekligi + 8);

  
  tft.fillRect(0, 1 * bolmeYuksekligi, PALET_GENISLIK, bolmeYuksekligi, TFT_LIGHTGREY);
  tft.setTextColor(RENK_SIYAH);
  tft.drawString("SILGI", 5, 1 * bolmeYuksekligi + 8);

  
  tft.fillRect(0, 2 * bolmeYuksekligi, PALET_GENISLIK, bolmeYuksekligi, RENK_PEMBE);
  tft.fillRect(0, 3 * bolmeYuksekligi, PALET_GENISLIK, bolmeYuksekligi, RENK_TURUNCU);
  tft.fillRect(0, 4 * bolmeYuksekligi, PALET_GENISLIK, bolmeYuksekligi, RENK_MOR);
  tft.fillRect(0, 5 * bolmeYuksekligi, PALET_GENISLIK, bolmeYuksekligi, RENK_SARI);
  tft.fillRect(0, 6 * bolmeYuksekligi, PALET_GENISLIK, bolmeYuksekligi, RENK_MAVI);
  tft.fillRect(0, 7 * bolmeYuksekligi, PALET_GENISLIK, bolmeYuksekligi, RENK_YESIL);
  tft.fillRect(0, 8 * bolmeYuksekligi, PALET_GENISLIK, bolmeYuksekligi, RENK_KIRMIZI);


  tft.drawFastVLine(PALET_GENISLIK - 1, 0, tft.height(), TFT_SILVER);
}