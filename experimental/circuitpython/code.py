# Esrille New Keyboard
# NISSE Powered by RP2040
#
# Copyright (c) 2022 Esrille Inc.
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
    Keycode.MINUS: {Keycode.PRINT_SCREEN},
    Keycode.QUOTE: {Keycode.KEYPAD_NUMLOCK},
    Keycode.BACKSLASH: {Keycode.SCROLL_LOCK},
}

# 10 key mode keymap
num_map = {
    Keycode.SPACE: {Keycode.ZERO},
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
row_pins = (board.GP15, board.GP14, board.GP13, board.GP12,
            board.GP11, board.GP10, board.GP9, board.GP8)
col_pins = (board.GP2, board.GP3, board.GP4, board.GP5,
            board.GP6, board.GP7, board.GP20, board.GP21,
            board.GP22, board.GP23, board.GP24, board.GP25)


class NISSEController:

    def __init__(self):
        self._keyboard = None

        self.rows = []
        for i in range(len(row_pins)):
            self.rows.append(digitalio.DigitalInOut(row_pins[i]))
            self.rows[i].switch_to_output(True, digitalio.DriveMode.OPEN_DRAIN)

        self.cols = []
        for i in range(len(col_pins)):
            self.cols.append(digitalio.DigitalInOut(col_pins[i]))
            self.cols[i].switch_to_input(digitalio.Pull.UP)

        self.led_num = digitalio.DigitalInOut(board.GP18)
        self.led_num.switch_to_output(True, digitalio.DriveMode.PUSH_PULL)
        self.led_caps = digitalio.DigitalInOut(board.GP17)
        self.led_caps.switch_to_output(True, digitalio.DriveMode.PUSH_PULL)
        self.led_scroll = digitalio.DigitalInOut(board.GP16)
        self.led_scroll.switch_to_output(True, digitalio.DriveMode.PUSH_PULL)

        self._report = b"\x00"  # LED status

    def _scan_keys(self):
        on = set()
        for r in range(len(row_pins)):
            self.rows[r].switch_to_output(False, digitalio.DriveMode.OPEN_DRAIN)
            for c in range(len(col_pins)):
                if self.cols[c].value == 0:
                    key = keymap[r][c]
                    if key is not None:
                        on.add(key)
            self.rows[r].switch_to_output(True, digitalio.DriveMode.OPEN_DRAIN)
        return on

    def _update_leds(self):
        report = self._keyboard._keyboard_device.get_last_received_report()
        if report is not None:
            self._report = report
            self.led_num.value = bool(self._report[0] & Keyboard.LED_NUM_LOCK)
            self.led_caps.value = bool(self._report[0] & Keyboard.LED_CAPS_LOCK)
            self.led_scroll.value = bool(self._report[0] & Keyboard.LED_SCROLL_LOCK)

    def _was_pressed(self):
        return self._keyboard.report_modifier[0] or self._keyboard.report_keys[0]

    def _clear_report(self):
        self._keyboard.report_modifier[0] = 0
        for i in range(6):
            self._keyboard.report_keys[i] = 0

    def run(self):
        self._keyboard = Keyboard(usb_hid.devices)
        self.led_num.value = False
        self.led_caps.value = False
        self.led_scroll.value = False
        previous_keys = set()
        current_keys = set()
        while True:
            self._update_leds()
            current_keys = self._scan_keys()
            on = previous_keys & current_keys
            previous_keys = current_keys
            if {LEFT_FN, RIGHT_FN} & on:
                on.discard(LEFT_FN)
                on.discard(RIGHT_FN)
                current_keys = set()
                for k in on:
                    if Keycode.modifier_bit(k):
                        current_keys.add(k)
                    else:
                        current_keys.update(fn_map.get(k, {k, Keycode.LEFT_CONTROL}))
                on = current_keys
            elif self._report[0] & Keyboard.LED_NUM_LOCK:
                current_keys = set()
                for k in on:
                    current_keys.update(num_map.get(k, {k}))
                on = current_keys
            if on:
                try:
                    self._clear_report()
                    self._keyboard.press(*on)
                except ValueError:
                    pass
            elif self._was_pressed():
                self._keyboard.release_all()
            time.sleep(0.008)


nisse = NISSEController()
nisse.run()
