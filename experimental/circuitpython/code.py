# Esrille New Keyboard
# NISSE Powered by Raspberry Pi Pico
#
# Copyright (c) 2022, 2023 Esrille Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import time
import board
import digitalio
import usb_hid
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keycode import Keycode
from adafruit_hid.consumer_control import ConsumerControl
from adafruit_hid.consumer_control_code import ConsumerControlCode


LED_NUM = 0
LED_CAPS = 1
LED_SCROLL = 2

# Extra key codes
LEFT_FN = 0xF0
RIGHT_FN = 0xF1
CALC = 0xFB

# default keymap (QWERTY US)
keymap = [
    [Keycode.F1, Keycode.F2, Keycode.F3, Keycode.F4, Keycode.F5, Keycode.F6,
     Keycode.F7, Keycode.F8, Keycode.F9, Keycode.F10, Keycode.F11, Keycode.F12],
    [Keycode.CAPS_LOCK, Keycode.LEFT_BRACKET, None, None, None, None,
     None, None, None, None, Keycode.EQUALS, Keycode.QUOTE],
    [Keycode.LEFT_CONTROL, Keycode.RIGHT_BRACKET, None, None, None, Keycode.ESCAPE,
     Keycode.APPLICATION, None, None, None, Keycode.MINUS, Keycode.RIGHT_CONTROL],
    [Keycode.LEFT_GUI, Keycode.GRAVE_ACCENT, None, None, None, Keycode.TAB,
     Keycode.ENTER, None, None, None, Keycode.BACKSLASH, Keycode.RIGHT_GUI],
    [LEFT_FN, Keycode.ONE, Keycode.TWO, Keycode.THREE, Keycode.FOUR, Keycode.FIVE,
     Keycode.SIX, Keycode.SEVEN, Keycode.EIGHT, Keycode.NINE, Keycode.ZERO, RIGHT_FN],
    [Keycode.BACKSPACE, Keycode.Q, Keycode.W, Keycode.E, Keycode.R, Keycode.T,
     Keycode.Y, Keycode.U, Keycode.I, Keycode.O, Keycode.P, Keycode.SPACE],
    [Keycode.LEFT_SHIFT, Keycode.A, Keycode.S, Keycode.D, Keycode.F, Keycode.G,
     Keycode.H, Keycode.J, Keycode.K, Keycode.L, Keycode.SEMICOLON, Keycode.RIGHT_SHIFT],
    [Keycode.LEFT_ALT, Keycode.Z, Keycode.X, Keycode.C, Keycode.V, Keycode.B,
     Keycode.N, Keycode.M, Keycode.COMMA, Keycode.PERIOD, Keycode.FORWARD_SLASH, Keycode.RIGHT_ALT],
]

cc_map = {
    Keycode.F7: ConsumerControlCode.SCAN_PREVIOUS_TRACK,
    Keycode.F8: ConsumerControlCode.PLAY_PAUSE,
    Keycode.F9: ConsumerControlCode.SCAN_NEXT_TRACK,
    Keycode.F10: ConsumerControlCode.MUTE,
    Keycode.F11: ConsumerControlCode.VOLUME_DECREMENT,
    Keycode.F12: ConsumerControlCode.VOLUME_INCREMENT,
}

# function keymap
fn_map = {
    Keycode.GRAVE_ACCENT: {Keycode.INSERT},
    Keycode.CAPS_LOCK: {Keycode.DELETE},
    Keycode.E: {Keycode.PAGE_UP},
    Keycode.D: {Keycode.PAGE_DOWN},
    Keycode.H: {Keycode.HOME},
    Keycode.SEMICOLON: {Keycode.END},
    Keycode.I: {Keycode.UP_ARROW},
    Keycode.J: {Keycode.LEFT_ARROW},
    Keycode.K: {Keycode.DOWN_ARROW},
    Keycode.L: {Keycode.RIGHT_ARROW},
    Keycode.U: {Keycode.LEFT_CONTROL, Keycode.LEFT_ARROW},
    Keycode.O: {Keycode.LEFT_CONTROL, Keycode.RIGHT_ARROW},
    Keycode.Y: {Keycode.LEFT_CONTROL, Keycode.HOME},
    Keycode.P: {Keycode.LEFT_CONTROL, Keycode.END},
    Keycode.MINUS: {Keycode.PRINT_SCREEN},
    Keycode.QUOTE: {Keycode.KEYPAD_NUMLOCK},
    Keycode.BACKSLASH: {Keycode.SCROLL_LOCK},
}

# 10 key mode keymap
num_map = {
    Keycode.SPACE: {Keycode.KEYPAD_ZERO},
    Keycode.M: {Keycode.KEYPAD_ONE},
    Keycode.COMMA: {Keycode.KEYPAD_TWO},
    Keycode.PERIOD: {Keycode.KEYPAD_THREE},
    Keycode.J: {Keycode.KEYPAD_FOUR},
    Keycode.K: {Keycode.KEYPAD_FIVE},
    Keycode.L: {Keycode.KEYPAD_SIX},
    Keycode.U: {Keycode.KEYPAD_SEVEN},
    Keycode.I: {Keycode.KEYPAD_EIGHT},
    Keycode.O: {Keycode.KEYPAD_NINE},
    Keycode.SIX: {CALC},
    Keycode.SEVEN: {Keycode.TAB},
    Keycode.EIGHT: {Keycode.KEYPAD_FORWARD_SLASH},
    Keycode.NINE: {Keycode.KEYPAD_ASTERISK},
    Keycode.ZERO: {Keycode.BACKSPACE},
    Keycode.P: {Keycode.KEYPAD_MINUS},
    Keycode.SEMICOLON: {Keycode.KEYPAD_PLUS},
    Keycode.FORWARD_SLASH: {Keycode.KEYPAD_ENTER},
    Keycode.QUOTE: {Keycode.KEYPAD_NUMLOCK},
}

# NISSE pin configuration
row_pins = (board.GP0, board.GP1, board.GP2, board.GP3,
            board.GP4, board.GP5, board.GP6, board.GP27)
col_pins = (board.GP18, board.GP19, board.GP20, board.GP21,
            board.GP22, board.GP26, board.GP7, board.GP8,
            board.GP9, board.GP10, board.GP11, board.GP12)
led_pins = (board.GP15, board.GP14, board.GP13)



class NISSEController:

    def __init__(self):
        self._rows = []
        for i in range(len(row_pins)):
            self._rows.append(digitalio.DigitalInOut(row_pins[i]))
            self._rows[i].switch_to_output(True, digitalio.DriveMode.OPEN_DRAIN)

        self._cols = []
        for i in range(len(col_pins)):
            self._cols.append(digitalio.DigitalInOut(col_pins[i]))
            self._cols[i].switch_to_input(digitalio.Pull.UP)

        self._led_num = digitalio.DigitalInOut(led_pins[LED_NUM])
        self._led_num.switch_to_output(True, digitalio.DriveMode.PUSH_PULL)
        self._led_caps = digitalio.DigitalInOut(led_pins[LED_CAPS])
        self._led_caps.switch_to_output(True, digitalio.DriveMode.PUSH_PULL)
        self._led_scroll = digitalio.DigitalInOut(led_pins[LED_SCROLL])
        self._led_scroll.switch_to_output(True, digitalio.DriveMode.PUSH_PULL)

        self._keyboard = Keyboard(usb_hid.devices)
        self._led_status = 0
        self._cc = ConsumerControl(usb_hid.devices)

    def _scan_keys(self):
        pressed = set()
        for r in range(len(row_pins)):
            self._rows[r].value = False
            for c in range(len(col_pins)):
                if self._cols[c].value == 0:
                    key = keymap[r][c]
                    if key is not None:
                        pressed.add(key)
            self._rows[r].value = True
        return pressed

    def _update_leds(self):
        report = self._keyboard._keyboard_device.get_last_received_report()
        if report is not None:
            self._led_status = report[0]
            self._led_num.value = bool(self._led_status & Keyboard.LED_NUM_LOCK)
            self._led_caps.value = bool(self._led_status & Keyboard.LED_CAPS_LOCK)
            self._led_scroll.value = bool(self._led_status & Keyboard.LED_SCROLL_LOCK)

    def _send_report(self, keycodes):
        for i in range(8):
            self._keyboard.report[i] = 0
        i = 2
        for keycode in keycodes:
            modifier = Keycode.modifier_bit(keycode)
            if modifier:
                self._keyboard.report[0] |= modifier
            elif i < 8:
                self._keyboard.report[i] = keycode
                i += 1
        self._keyboard._keyboard_device.send_report(self._keyboard.report)

    def run(self):
        self._led_num.value = False
        self._led_caps.value = False
        self._led_scroll.value = False
        previous_pressed = set()  # previously pressed keys
        current_pressed = set()   # currently pressed keys
        previous = set()          # previously report
        previous_cc = 0
        while True:
            self._update_leds()
            current_cc = 0
            current_pressed = self._scan_keys()
            current = previous_pressed & current_pressed
            previous_pressed = current_pressed
            if {LEFT_FN, RIGHT_FN} & current:
                current -= {LEFT_FN, RIGHT_FN}
                tmp = set()
                for k in current:
                    if Keycode.modifier_bit(k):
                        tmp.add(k)
                    elif k in cc_map:
                        current_cc = cc_map[k]
                    else:
                        tmp.update(fn_map.get(k, {k, Keycode.LEFT_CONTROL}))
                current = tmp
            elif self._led_status & Keyboard.LED_NUM_LOCK:
                tmp = set()
                for k in current:
                    tmp.update(num_map.get(k, {k}))
                current = tmp
            if current != previous:
                self._send_report(current)
                previous = current
            if current_cc != previous_cc:
                self._cc.press(current_cc)
                previous_cc = current_cc
            time.sleep(0.008)


nisse = NISSEController()
nisse.run()
