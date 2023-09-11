/*
 *  Project     AnalogSelector Library
 *  @author     David Madison
 *  @link       github.com/dmadison/AnalogSelector-Arduino
 *  @license    MIT - Copyright (c) 2023 David Madison
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *  Example:      SimpleSelector
 *  Description:  Create a basic analog selector using the library,
 *                reading from a potentiometer connected to one of the
 *                board's analogs pin and printing the result to serial.
 */

#include <AnalogSelector.h>

const int Pin = A0;
const int NumPositions = 5;
AnalogSelector selector(Pin, NumPositions);

int previous = -1;


void setup() {
	Serial.begin(115200);
	while (!Serial);

	selector.begin();
	delay(500);
}

void loop() {
	int current = selector.getPosition();

	if (current != previous) {
		Serial.print("Selector changed to ");
		Serial.print(current + 1);
		Serial.print(" / ");
		Serial.print(NumPositions);
		Serial.println();

		previous = current;
	}
}
