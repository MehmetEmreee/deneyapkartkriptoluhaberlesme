#include <esp_now.h>
#include <WiFi.h>
extern "C" {
  #include "esp_wifi.h"
}
#include "mbedtls/aes.h"
#include "mbedtls/md.h"

const uint8_t AES_KEY[16] = {
  0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x70, 0x81,
  0x9A, 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xF0, 0x01
};

void decryptAES_CTR(uint8_t* input, size_t len, uint8_t* output, const uint8_t* key, const uint8_t* iv) {
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, key, 128);
  uint8_t stream_block[16] = {0};
  size_t nc_off = 0;
  uint8_t iv_copy[16];
  memcpy(iv_copy, iv, 16); // IV'nin kopyasını kullan
  mbedtls_aes_crypt_ctr(&aes, len, &nc_off, iv_copy, stream_block, input, output);
  mbedtls_aes_free(&aes);
}

void generateHMAC(const uint8_t* data, size_t len, const uint8_t* key, size_t keylen, uint8_t* output) {
  const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, md_info, 1);
  mbedtls_md_hmac_starts(&ctx, key, keylen);
  mbedtls_md_hmac_update(&ctx, data, len);
  mbedtls_md_hmac_finish(&ctx, output);
  mbedtls_md_free(&ctx);
}

void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
  if (len < 20 || len > 270) {
    Serial.println("❌ Geçersiz veri uzunluğu.");
    return;
  }

  uint8_t iv[16];
  memcpy(iv, data, 16);
  int dataLen = len - 16 - 4;

  if (dataLen <= 0 || dataLen > 250) {
    Serial.println("❌ Geçersiz veri uzunluğu.");
    return;
  }

  uint8_t encrypted[250];
  memcpy(encrypted, data + 16, dataLen);

  uint8_t receivedHMAC[4];
  memcpy(receivedHMAC, data + 16 + dataLen, 4);

  uint8_t calcHMAC[32];
  generateHMAC(encrypted, dataLen, AES_KEY, 16, calcHMAC);

  // Hata ayıklama çıktıları
  Serial.print("Gelen IV: ");
  for (int i = 0; i < 16; i++) Serial.printf("%02X ", iv[i]);
  Serial.println();
  Serial.print("Gelen şifrelenmiş veri: ");
  for (int i = 0; i < dataLen; i++) Serial.printf("%02X ", encrypted[i]);
  Serial.println();
  Serial.print("Gelen HMAC: ");
  for (int i = 0; i < 4; i++) Serial.printf("%02X ", receivedHMAC[i]);
  Serial.println();
  Serial.print("Hesaplanan HMAC (ilk 4): ");
  for (int i = 0; i < 4; i++) Serial.printf("%02X ", calcHMAC[i]);
  Serial.println();

  if (memcmp(receivedHMAC, calcHMAC, 4) != 0) {
    Serial.println("❌ HMAC uyuşmuyor. Mesaj çözülemedi.");
    return;
  }

  uint8_t decrypted[250];
  decryptAES_CTR(encrypted, dataLen, decrypted, AES_KEY, iv);
  decrypted[dataLen] = '\0';

  // Yazdırılabilir karakter kontrolü
  bool valid = true;
  for (int i = 0; i < dataLen; i++) {
    if (decrypted[i] < 32 || decrypted[i] > 126) {
      valid = false;
      break;
    }
  }

  if (valid) {
    Serial.print("📥 Alınan şifreli mesajın çözümü: ");
    Serial.println((char*)decrypted);
  } else {
    Serial.println("❌ Çözülen veri geçersiz (yazdırılamaz karakterler).");
    Serial.print("Çözülen ham veri: ");
    for (int i = 0; i < dataLen; i++) Serial.printf("%02X ", decrypted[i]);
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW başlatılamadı!");
    return;
  }

  esp_now_register_recv_cb(onReceive);
  Serial.println("📡 Alıcı kart hazır.");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // Gelen mesajları pasif bekliyoruz
}
