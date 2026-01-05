/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "led.h"
#include "evaluate.h"

#define MAX_LINES      2
#define MAX_CHARS      16

void set_row(int pin) {
    for(int i = 0; i < 4; i++) {
        gpio_put(6+i, false);
    }
    gpio_put(pin,true);
}

const char keys[16] = {
    '1','2','3','+',
    '4','5','6','-',
    '7','8','9','*',
    '.','0','=','/'
};

int main() {
    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    lcd_init();

    // keypad
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            const int pin = 6 + (i * 4 + j); // offset + idx
            gpio_init(pin);
            if(pin < 10) {
                gpio_set_dir(pin,GPIO_OUT);
            } else {
                gpio_set_dir(pin,GPIO_IN);
            }
        }
    }

    int newPress = 0;
    char str[16] = "";
    int strIndex = 0;
    int solved = 0;

    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_string("Calculator");
    lcd_set_cursor(1,0);
    lcd_string("Press any key...");
    lcd_backlight(1);

    float evalResult = 0.0f;
    int err = 0;

    float backlightTimeout = 0.0;
    int backlightStatus = 1;

    while (1) {
        for(int i = 0; i < 4; i++) {
            set_row(6+i);
            for(int j = 0; j < 4; j++) {
                const int pin = 10 + j; // offset + idx
                if(gpio_get(pin) == 1) {
                    if(backlightStatus == 0) {
                        backlightStatus = 1;
                        lcd_backlight(1);
                    }
                    backlightTimeout = 0;

                    char key = keys[i * 4 + j];
                    if(key == '=' && strIndex > 0) {
                        if(solved) {
                            strIndex = 0;
                            memset(str,0,16);
                            lcd_clear();
                            solved = false;
                        } else {
                            lcd_set_cursor(1,0);
                            char buffer[16];
                            err = 0;
                            evalResult = eval(str,strIndex, &err);
                            if(!err) sprintf(buffer,"=%.2f",evalResult);
                            else sprintf(buffer,"ERROR");
                            lcd_string(buffer);
                            solved = true;
                        }
                        while(gpio_get(pin) == 1);
                        break;
                    }
                    if(strIndex >= 16 || solved || key == '=') break;
                    str[strIndex] = key;
                    strIndex++;
                    newPress = true;
                    while(gpio_get(pin) == 1);
                }
            }
            sleep_ms(1);
        }

        if(newPress) {
            newPress = false;
            lcd_clear();
            lcd_set_cursor(0,0);
            lcd_string(str);
        }

        sleep_ms(20);
        if(backlightTimeout >= 6000 && backlightStatus == 1) {
            lcd_backlight(0);
            backlightStatus = 0;
        } else if(backlightStatus == 1) {
            backlightTimeout += 20;
        }
    }
}
