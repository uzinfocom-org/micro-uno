#include <avr/pgmspace.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte num_rows= 5;
const byte num_cols= 4;

uint8_t keymap[num_rows][num_cols] = {
  {'0','1','2','3'},
  {'4','5','6','7'},
  {'8','9','A','B'},
  {'C','D','E','F'},
  {'R','M','K','L'},
};

byte row_pins[num_rows] = {9, 10, 11, 12, 13};
byte col_pins[num_cols] = {8, 7, 6, 5};


Keypad keypad = Keypad(makeKeymap(keymap), row_pins, col_pins, num_rows, num_cols);

#define MEMORY_SIZE 1024

uint8_t  memory[MEMORY_SIZE];

uint8_t  register_A = 0;
uint8_t  register_B = 0;
uint16_t  program_counter = 0;
uint16_t stack_pointer = 0x3ff;
bool zero_flag = 0;


void reset_memory() {
  for (uint16_t i = 0; i < MEMORY_SIZE; i++)
    memory[i] = 0;
}

uint8_t read_byte() {
  uint8_t value = memory[program_counter];
  program_counter++;
  return value;
}

uint16_t read_word() {
  uint8_t MSB = read_byte();
  uint8_t LSB = read_byte();
  uint16_t value = MSB;
  value <<= 8;
  value |= LSB;
  return value;
}

uint16_t encode_word() {
  uint16_t addr = 0;
  for (int i = 12; i >= 0; i -= 4) {  
    char input = getch();
    if ( input == 'R' ){
      return 0xffff;
    }else if ( input == 'K' ){
      return 0xfffa;
    }else if ( input == 'L' ){
      return 0xfffd;
    }else if ( input == 'M' ){
      return 0xfffc;
    }
    uint8_t hex = ascii_to_hex(input);
    if (i) addr |= hex << i;
    else addr |= hex;
    lcd.print(hex, HEX);
  }
  delay(200);
  return addr;
}

uint8_t encode_byte() {
  uint8_t value = 0;
  for (int i = 4; i >= 0; i -= 4) {  
    char input = getch();
    uint8_t hex = ascii_to_hex(input);
    if (i) value |= hex << i;
    else value |= hex;
    lcd.print(hex, HEX);
  } return value;
}

void print_byte(uint8_t  byte) {
  if (byte<0x10) print_message_lcd(F("0"));
  lcd.print(byte, HEX);
  lcd.print(' ');
}

void print_word(uint16_t  word) {
  uint8_t  MSB = byte(word >> 8);
  uint8_t  LSB = byte(word);
  if (MSB<0x10) { print_message_lcd(F("0")); } lcd.print(MSB,HEX);
  if (LSB<0x10) { print_message_lcd(F("0")); } lcd.print(LSB,HEX);
}

void memory_dump(uint16_t addr) {
  lcd.clear();
  print_word(addr); lcd.print(':');
  for (uint16_t i = addr; i < addr + 4; i++) print_byte(memory[i]);
  lcd.setCursor(0, 1);
}

void print_hex(uint8_t data) {  
  if (data < 0x10) print_message_serial(F("0")); 
  Serial.print(data, HEX);
}

void print_message_lcd(const __FlashStringHelper *message) {
  for (byte i = 0; i < strlen_P((const char*)message); i++) {
    char character = pgm_read_byte_near((const char*)message + i);
    lcd.print(character);
  }
}

void print_message_serial(const __FlashStringHelper *message) {
  for (byte i = 0; i < strlen_P((const char*)message); i++) {
    char character = pgm_read_byte_near((const char*)message + i);
    Serial.print(character);
  }
}

void reset_cpu() {
  register_A = 0;
  register_B = 0;
  program_counter = 0;
  stack_pointer = 0x3ff;
  zero_flag = 0;
}

void execute() {
  while (true) {
    uint8_t opcode = read_byte();    
    
    switch (opcode) {
      case 0x00: program_counter = 0; return;
      case 0x01: zero_flag = ((register_A = read_byte()) == 0); break;
      case 0x02: zero_flag = ((register_A = memory[(read_word() + register_B)]) == 0); break;
      case 0x03: zero_flag = ((register_B = register_A) == 0); break;
      case 0x04: zero_flag = ((register_A += read_byte()) == 0); break;
      case 0x05: zero_flag = ((register_A -= read_byte()) == 0); break;
      case 0x06: memory[read_word() + register_B] = register_A; break;
      case 0x08: program_counter = read_word(); break;
      case 0x09: zero_flag = (++register_B == 0); break;
      case 0x0a: zero_flag = (--register_B == 0); break;
      case 0x0b: zero_flag = ((register_A - read_byte()) == 0); break;
      case 0x0c: if (zero_flag) program_counter = read_word(); else read_word(); break;
      case 0x07: zero_flag = (register_A = keypad.getKey()) == 0; break;
      case 0x0e: while ((register_A = keypad.getKey()) == NO_KEY); break;
      case 0x0f: lcd.print(char(register_A)); break;
      case 0x28: Serial.print(char(register_A)); break;
      case 0x10: zero_flag = ((register_A & read_byte()) == 0); break;
      case 0x11: zero_flag = ((register_A &= read_byte()) == 0); break;
      case 0x12: zero_flag = ((register_A |= read_byte()) == 0); break;
      case 0x13: zero_flag = ((register_A ^= read_byte()) == 0); break;
      case 0x14: zero_flag = ((register_A = ~read_byte()) == 0); break;
      case 0x15: zero_flag = ((register_A <<= read_byte()) == 0); break;
      case 0x16: zero_flag = ((register_A >>= read_byte()) == 0); break;
      case 0x17: lcd.clear(); break;
      case 0x18: lcd.scrollDisplayLeft(); break;
      case 0x19: lcd.scrollDisplayRight(); break;
      case 0x1a: lcd.blink(); break;
      case 0x1b: lcd.noBlink(); break;
      case 0x1e: lcd.setCursor(register_A, register_B); break;
      case 0x1f: delay(read_byte()); break;
      case 0x20: zero_flag = (register_A = random(read_byte())); break;
      case 0x25: lcd.print(memory[read_word()]); break;
      case 0x26: zero_flag = (++memory[read_word()] == 0); break;
      case 0x27: zero_flag = (--memory[read_word()] == 0); break;
      case 0x21:
        memory[stack_pointer--] = register_A;
        memory[stack_pointer--] = register_B;
        break;
      case 0x22:
        register_B = memory[++stack_pointer];
        register_A = memory[++stack_pointer];
        break;
      case 0x23:
        memory[stack_pointer--] = (uint8_t)(program_counter & 0x00ff) + 2;
        memory[stack_pointer--] = (uint8_t)(program_counter >> 4);
        program_counter = read_word();
        break;
      case 0x24:
        program_counter = 0;
        program_counter <<= memory[++stack_pointer];
        program_counter |= memory[++stack_pointer];
        break;
      case 0x1c:
        lcd.createChar(register_A, memory + register_B);
        lcd.begin();
        break;
      case 0x1d: lcd.write(byte(read_byte())); break;
      case 0x0d:
        print_message_serial(F("A register: 0x"));
        Serial.println(register_A, HEX);
        print_message_serial(F("B register: 0x"));
        Serial.println(register_B, HEX);
        print_message_serial(F("Hisoblagich: 0x"));
        Serial.println(program_counter, HEX);
        print_message_serial(F("Stek ko'rsatkichi: 0x"));
        Serial.println(stack_pointer, HEX);
        print_message_serial(F("ZF: 0x"));
        Serial.println(zero_flag, HEX);
        break;
      default:
        lcd.clear();
        print_message_lcd(F("NOMA'LUM OPKOD:"));
        lcd.setCursor(0, 1);
        print_byte(opcode);
        print_message_lcd(F("? "));
        return;
    }
  }
}


char getch() {
  char key;
  while ((key = keypad.getKey()) == NO_KEY) {
      
  }
  return key;
}

uint8_t ascii_to_hex(char ascii) {
  return ascii <= '9' ? (ascii - '0') : (ascii - 'A' + 10);
}

void init_computer() {
  lcd.clear();
  print_message_lcd(F("    MICRO UNO   "));
  lcd.setCursor(0, 1);
  reset_cpu();
  reset_memory();
}

void command_run() {
  lcd.clear();
  print_message_lcd(F("ISHGA TUSHIRISH"));
  lcd.setCursor(3, 2);
  delay(300);
  lcd.clear();
  execute();
}

void command_view() {
  lcd.clear();
  print_message_lcd(F("KO'RISH: "));
  memory_dump(encode_word());
}

void command_load() {
  lcd.clear();
  print_message_lcd(F("Yuklash..."));
  delay(300);
  lcd.clear();
  print_message_lcd(F(" Ma'lumotlar"));
  lcd.setCursor(0, 1);
  print_message_lcd(F("kutilmoqda..."));
  while (Serial.available() == 0);
  lcd.clear();
  print_message_lcd(F("Yuklash..."));
  Serial.readBytes(memory, MEMORY_SIZE);
  
  for (int i = 0; i < MEMORY_SIZE; i++) {
    memory[i] = ascii_to_hex(memory[i]);
    if ((i % 2) == 0) memory[i] <<= 4;
    else {
      memory[i - 1] |= memory[i];
      memory[i] = 0;
    }
  }
  
  for (int i = 0; i < MEMORY_SIZE; i++) {
    if ((i % 2) == 0) {
      if (i) {
        memory[i - ((int)(i / 2))] = memory[i];
        memory[i] = 0;
      }
    }
  }

  print_message_lcd(F("    ok"));
  lcd.setCursor(0, 1);
}

void command_save() {
  lcd.clear();
  print_message_lcd(F("SAQLASH "));
  delay(300);
  lcd.clear();
  print_message_lcd(F("Saqlash..."));
  
  for (int i = 0; i < MEMORY_SIZE; i++) {
    if ( memory[i] < 0x10) print_message_serial(F("0"));
    Serial.print(memory[i], HEX);
  }
  
  print_message_lcd(F("    ok"));
  lcd.setCursor(0, 1);
}

void command_clear() {
  lcd.clear();
  print_message_lcd(F("TOZALASH"));
  delay(300);
  lcd.clear();
}

void command_new() {
  lcd.clear();
  print_message_lcd(F("YANGI  "));
  lcd.setCursor(3, 2);
  delay(300);
  init_computer();
}

void setup() {
  Serial.begin(9600);
  
  lcd.begin();
  lcd.noAutoscroll();
  lcd.blink();
  
  init_computer();
}

void loop() {  
  while (true) {
    uint16_t addr = encode_word();
    lcd.print(':');

    switch (addr) {
      case 0xfffa: command_clear(); break;
      case 0xfffb: command_new(); break;
      case 0xfffc: command_view(); break;
      case 0xfffd: command_load(); break;
      case 0xfffe: command_save(); break;
      case 0xffff: command_run(); break;
      
      default:
        for (uint16_t i = addr; i < addr + 4; i++) {
          memory[i] = encode_byte();
          lcd.print(' ');
        }
        
        delay(300);
        memory_dump(addr);
        break;
    }    
  }
}
