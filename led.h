/* Quick helper function for single byte transfers */
void i2c_write_byte(uint8_t val);

void lcd_toggle_enable(uint8_t val);

// The display is sent a byte as two separate nibble transfers
void lcd_send_byte(uint8_t val, int mode);

void lcd_clear(void);

// go to location on LCD
void lcd_set_cursor(int line, int position);

void lcd_char(char val);

void lcd_string(const char *s);

void lcd_backlight(int val);

void lcd_init();