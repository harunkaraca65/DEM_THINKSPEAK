\# ESP32 Wi-Fi \& ThingSpeak Menü Sistemi - Algoritma Akışı

\# ESP32 Wi-Fi \& ThingSpeak Menu System - Algorithm Flow



---



\## 🇹🇷 Türkçe



Bu Arduino uygulaması, ESP32 için kalıcı hafıza (Preferences) destekli bir Wi-Fi ve ThingSpeak bağlantı yöneticisidir.

Program başladığında, seri haberleşmeyi ve kalıcı depolama birimini başlatır.

Kullanıcıya seri monitör üzerinden "Otomatik Bağlan" (O) veya "Yeni Kurulum" (N) seçeneklerini sunan bir menü döngüsü başlatılır.

Eğer kullanıcı "Otomatik Bağlan"ı seçerse, sistem hafızada kayıtlı SSID, şifre ve API anahtarı olup olmadığını kontrol eder.

Kayıtlı veri bulunursa, bu bilgiler kullanılarak Wi-Fi ağına bağlanılmaya çalışılır.

Eğer kullanıcı "Yeni Kurulum"u seçerse, sistem interaktif bir sihirbaz başlatır.

Kullanıcıdan SSID ve şifre istenir ve bağlantı başarılı olana kadar giriş yapılması beklenir.

Wi-Fi bağlantısı sağlandıktan sonra, kullanıcıdan ThingSpeak API anahtarı istenir.

Girilen API anahtarı, sunucuya yapılan bir test isteği ile doğrulanır.

Tüm bilgiler geçerliyse, bu veriler bir sonraki açılışta kullanılmak üzere kalıcı hafızaya kaydedilir.

Kurulum tamamlandığında sistem ana döngüye girer.

Ana döngü, öncelikle Wi-Fi bağlantısının aktif olup olmadığını kontrol eder ve kopukluk varsa yeniden bağlanır.

Her 15 saniyede bir, rastgele bir sensör verisi üretilir.

Bu veri, saklanan API anahtarı kullanılarak ThingSpeak sunucusuna HTTP GET isteği ile gönderilir.



---



\## 🇬🇧 English



This Arduino application is a Wi-Fi and ThingSpeak connection manager with persistent storage (Preferences) support for ESP32.

When the program starts, it initializes serial communication and the non-volatile storage unit.

A menu loop is initiated, offering the user "Auto Connect" (O) or "New Setup" (N) options via the serial monitor.

If the user selects "Auto Connect," the system checks for saved SSID, password, and API keys in memory.

If saved data is found, it attempts to connect to the Wi-Fi network using these credentials.

If the user selects "New Setup," the system launches an interactive wizard.

The user is prompted for the SSID and password, and the system waits for input until a successful connection is established.

Once Wi-Fi connection is secured, the user is asked for the ThingSpeak API key.

The entered API key is validated via a test request to the server.

If all information is valid, the data is saved to persistent memory for use in the next boot.

Upon completion of the setup, the system enters the main loop.

The main loop first checks if the Wi-Fi connection is active and reconnects if it has been lost.

Every 15 seconds, a random sensor value is generated.

This data is sent to the ThingSpeak server via an HTTP GET request using the stored API key.

