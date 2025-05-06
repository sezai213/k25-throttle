#include <mcp_can.h>
#include <SPI.h>

#define CAN_CS_PIN 10 // MCP2515 CS pini
#define LED_PIN 13    // Dahili LED pini (D13)

#define RAW_INPUT_BASE_VALUE 588.0
#define RAW_INPUT_TOP_VALUE 550.0

MCP_CAN CAN(CAN_CS_PIN); // MCP2515 nesnesi oluşturuluyor

float get_throttle_value(int read_value)
{
  if (read_value > RAW_INPUT_BASE_VALUE)
  {
    read_value = RAW_INPUT_BASE_VALUE;
  }
  if (read_value < RAW_INPUT_TOP_VALUE)
  {
    read_value = RAW_INPUT_TOP_VALUE;
  }
  return (RAW_INPUT_BASE_VALUE - read_value) / (RAW_INPUT_BASE_VALUE - RAW_INPUT_TOP_VALUE) * 100.0;
}

void setup()
{
  Serial.begin(112500);     // Seri iletişim başlatılıyor
  pinMode(LED_PIN, OUTPUT); // Dahili LED pini çıkış olarak ayarlanıyor

  // CAN Bus başlatılıyor
  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
  {
    Serial.println("MCP2515 başarıyla başlatıldı!");
  }
  else
  {
    Serial.println("MCP2515 başlatılamadı!");
    while (1)
      ; // Başlatma başarısız olursa programı durdur
  }

  CAN.setMode(MCP_NORMAL); // Normal moda geçiş yapılıyor
}

void loop()
{
  // CAN mesajı alınıyor

  int deger = analogRead(A0); // A0 pininden analog değeri oku (0 - 1023 arası)
  Serial.println(get_throttle_value(deger));

  float percent = get_throttle_value(deger);

  // Mercedes genellikle throttle'ı 16-bit olarak kullanır (0–65535 arası)
  int throttle_raw = (int)(percent / 100.0 * 65535.0);

  // 2 byte olarak ayır (big endian - MSB, LSB)
  byte data[8] = {
      (byte)(throttle_raw >> 8),   // Byte 0: MSB
      (byte)(throttle_raw & 0xFF), // Byte 1: LSB
      0, 0, 0, 0, 0, 0             // Diğer byte'lar boş
  };

  // CAN mesajı gönderiliyor
  byte sendStatus = CAN.sendMsgBuf(0x0B4, 0, 8, data);
  if (sendStatus == CAN_OK)
  {
    Serial.print("CAN mesajı başarıyla gönderildi! Gönderilen değer: ");
    Serial.println(percent);
  }
  else
  {
    Serial.println("CAN mesajı gönderilemedi!");
    Serial.println(sendStatus);
  }
  // 5ms (5000 microseconds) bekle - loop hızını kontrol et
  delay(2);
}

// byte randomValue = random(1, 8);
// byte data[1] = {randomValue};  // Gönderilecek veri

// // CAN mesajı gönderiliyor
// byte sendStatus = CAN.sendMsgBuf(0x100, 0, 1, data);  // CAN ID: 0x100, Veri uzunluğu: 1
// if (sendStatus == CAN_OK) {
//   Serial.print("CAN mesajı başarıyla gönderildi! Gönderilen değer: ");
//   Serial.println(randomValue);
// } else {
//   Serial.println("CAN mesajı gönderilemedi!");
//   Serial.println(sendStatus);

// }
//}