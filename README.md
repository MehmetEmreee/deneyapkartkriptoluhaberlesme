# Deneyap Kart Güvenli Mesaj İletimi Projesi

## Proje Açıklaması
Bu proje, Deneyap Kart 1A ve 1A v2 mikrodenetleyicileri kullanılarak geliştirilmiş bir güvenli mesaj iletimi sistemidir. ESP-NOW protokolü üzerinden AES-CTR şifreleme ve HMAC doğrulama ile gönderici ve alıcı kartlar arasında kablosuz veri aktarımı gerçekleştirilir. Kullanıcı, gönderici kart üzerinden seri monitör ile mesaj girer, bu mesaj şifrelenerek alıcıya iletilir ve alıcı tarafından çözülerek görüntülenebilir.

## Özellikler
- **Güvenli İletim**: AES-CTR şifreleme ve HMAC ile veri güvenliği.
- **Kablosuz İletişim**: ESP-NOW protokolü ile düşük güç tüketimi.
- **Kolay Kullanım**: Deneyap Kart IDE ile programlama.
- **Hata Ayıklama**: Detaylı seri port çıktıları ile sorun giderme.

## Gereksinimler
- **Donanım**: 2 adet Deneyap Kart (1A veya 1A v2), USB kabloları.
- **Yazılım**: Deneyap Kart IDE, mbedtls kütüphanesi.

## Kurulum
1. Deneyap Kart IDE’yi indirin ve kurun: [Deneyap Kart IDE](https://www.deneyapkart.org/tr/ide).
2. IDE’de mbedtls kütüphanesini yükleyin: **Araçlar > Kütüphaneleri Yönet**.
3. Alıcı kartın MAC adresini bulun (geçici kod ile seri monitörden).
4. Gönderici kodunda `receiverMac` adresini güncelleyin.
5. Her iki kartın kodunu sırasıyla yükleyin.

## Kullanım
- Gönderici seri monitörüne mesaj yazın (örneğin, "MERHABA").
- Alıcı seri monitöründe çözülen mesajı görün.
