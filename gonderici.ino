#include <esp_now.h>
#include <WiFi.h>
extern "C" {
  #include "esp_wifi.h"
}
#include "mbedtls/aes.h"
#include "mbedtls/md.h"

uint8_t receiverMac[] = {0x7C, 0x87, 0xCE, 0xF8, 0xD3, 0x38}; // Alıcı MAC adresi

const uint8_t AES_KEY[16] = {
  0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x70, 0x81,
  0x9A, 0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0xF0, 0x01
};

void encryptAES_CTR(uint8_t* input, size_t len, uint8_t* output, const uint8_t* key, const uint8_t* iv) {
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

void sendEncryptedMessage(const char* plainText) {
  size_t plainLen = strlen(plainText);
  if (plainLen == 0 || plainLen > 250) {
    Serial.println("❌ Mesaj boş veya çok uzun!");
    return;
  }

  uint8_t iv[16];
  esp_fill_random(iv, 16); // 16 byte rastgele IV

  uint8_t encrypted[250];
  encryptAES_CTR((uint8_t*)plainText, plainLen, encrypted, AES_KEY, iv);

  uint8_t hmac[32];
  generateHMAC(encrypted, plainLen, AES_KEY, 16, hmac);

  uint8_t packet[300];
  memcpy(packet, iv, 16);
  memcpy(packet + 16, encrypted, plainLen);
  memcpy(packet + 16 + plainLen, hmac, 4);

  // Hata ayıklama çıktıları
  Serial.print("Orijinal mesaj: ");
  Serial.println(plainText);
  Serial.print("IV: ");
  for (int i = 0; i < 16; i++) Serial.printf("%02X ", iv[i]);
  Serial.println();
  Serial.print("Şifrelenmiş veri: ");
  for (int i = 0; i < plainLen; i++) Serial.printf("%02X ", encrypted[i]);
  Serial.println();
  Serial.print("HMAC (ilk 4): ");
  for (int i = 0; i < 4; i++) Serial.printf("%02X ", hmac[i]);
  Serial.println();

  esp_now_send(receiverMac, packet, 16 + plainLen + 4);
  Serial.println("🚀 Mesaj gönderildi (şifreli).");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW başlatılamadı!");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  if (!esp_now_is_peer_exist(receiverMac)) esp_now_add_peer(&peerInfo);

  Serial.println("📤 Gönderici hazır. Seri porta yaz mesajı yollayayım:");
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    sendEncryptedMessage(msg.c_str());
  }
}
