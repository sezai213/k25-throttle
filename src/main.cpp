#include <mcp_can.h>
#include <SPI.h>
#define K25_DEBUG_MODE 0
#define CAN_CS_PIN 10 // MCP2515 CS pini

#define RAW_INPUT_BASE_VALUE 722.0
#define RAW_INPUT_TOP_VALUE 668.0

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
  // Serial.println(deger);
  if (K25_DEBUG_MODE)
  {
    Serial.print("Analog değer: ");
    Serial.println(deger);
  }

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
    if (K25_DEBUG_MODE)
    {
      Serial.print("CAN mesajı başarıyla gönderildi! Gönderilen değer: ");
      Serial.println(percent);
    }
  }
  else
  {
    if (K25_DEBUG_MODE)
    {
      Serial.print("CAN mesajı gönderilemedi!: ");
      Serial.println(sendStatus);
    }
  }
  // 5ms (5000 microseconds) bekle - loop hızını kontrol et
  delay(20); // 50 Hz
}
